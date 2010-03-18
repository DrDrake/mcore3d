#include "Pch.h"
#include "PodLoader.h"
#include "../Component/Render/MeshComponent.h"
#include "../Component/Prefab.h"
#include "../Render/Effect.h"
#include "../Render/Material.h"
#include "../Render/Mesh.h"
#include "../Render/SemanticMap.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/StrUtility.h"
#include "../../3Party/glew/glew.h"
#include "../../3Party/PowerVR/PVRTModelPOD.h"

#include "../Core/Math/Vec2.h"

#ifdef MCD_VC
#	ifdef NDEBUG
#		pragma comment(lib, "PowerVR")
#	else
#		pragma comment(lib, "PowerVRd")
#	endif
#	pragma comment(lib, "OpenGL32")
#	pragma comment(lib, "GLU32")
#	pragma comment(lib, "glew")
#endif	// MCD_VC

namespace MCD {

class PodMaterial : public Material
{
};	// PodMaterial

class PodLoader::Impl
{
public:
	Impl(IResourceManager* resourceManager)
		: mResourceManager(resourceManager)
	{
	}

	~Impl() {
		mPod.Destroy();
	}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const wchar_t* args);

	void commit(Resource& resource);

	IResourceManager* mResourceManager;

	Entity mRootEntity;
	CPVRTModelPOD mPod;

	//!	Store a list of Mesh that is pending to commit the buffer (which must done in main thread).
	std::vector<std::pair<MeshPtr, std::vector<void*> > > mMeshes;

	std::vector<TexturePtr> mTextures;
	ptr_vector<PodMaterial> mMaterials;

	volatile IResourceLoader::LoadingState mLoadingState;
};	// Impl

static int podTypeToGlType(EPVRTDataType type)
{
	switch(type) {
	case EPODDataFloat:			return GL_FLOAT;
	case EPODDataInt:			return GL_INT;
	case EPODDataShort:			return GL_SHORT;
	case EPODDataUnsignedShort:	return GL_UNSIGNED_SHORT;
	default:					MCD_ASSERT(false); return -1;
	}
}

static uint16_t typeToSize(EPVRTDataType type)
{
	switch(type) {
	case EPODDataFloat:			return sizeof(float);
	case EPODDataInt:			return sizeof(int);
	case EPODDataShort:			return sizeof(short);
	case EPODDataUnsignedShort:	return sizeof(unsigned short);
	default:					MCD_ASSERT(false); return 0;
	}
}

static void assignAttribute(Mesh& mesh, std::vector<void*>& bufferPtrs, const SPODMesh& podMesh, Mesh::Attribute& a, const CPODData& data, bool isIndex=false)
{
	a.bufferIndex = uint8_t(mesh.bufferCount);
	a.dataType = podTypeToGlType(data.eType);
	a.elementCount = uint8_t(data.n);
	a.elementSize = typeToSize(data.eType);
	a.stride = uint16_t(data.nStride);

	if(isIndex) {
		a.byteOffset = 0;
		mesh.bufferCount++;
		bufferPtrs.push_back(data.pData);
	}
	else {
		a.byteOffset = podMesh.pInterleaved ? uint16_t(data.pData) : 0;

		if(!podMesh.pInterleaved) {
			mesh.bufferCount++;
			bufferPtrs.push_back(data.pData);
		}
	}

	mesh.attributeCount++;
}

IResourceLoader::LoadingState PodLoader::Impl::load(std::istream* is, const Path* fileId, const wchar_t* args)
{
	mLoadingState = is ? NotLoaded : Aborted;

	if(mLoadingState & Stopped)
		return mLoadingState;

	EPVRTError result = mPod.ReadFromStream(*is);

	if(result != PVR_SUCCESS)
		return mLoadingState = Aborted;

	// Load the textures ASAP
	for(size_t i=0; i<mPod.nNumTexture; ++i)
	{
		std::wstring texturePath;
		if(!strToWStr(mPod.pTexture[i].pszName, texturePath)) {
			mTextures.push_back(new Texture(L""));
			continue;
		}

		// We assume the texture is relative to the pod file, if the texture file didn't has a root path.
		Path adjustedPath = fileId ? fileId->getBranchPath()/texturePath : texturePath;
		if(mResourceManager)
			mTextures.push_back(dynamic_cast<Texture*>(mResourceManager->load(adjustedPath).get()));
		else
			mTextures.push_back(new Texture(adjustedPath));
	}

	// Generate materials
	for(size_t i=0; i<mPod.nNumMaterial; ++i)
	{
		const SPODMaterial& material = mPod.pMaterial[i];

		PodMaterial* m = new PodMaterial;
		m->mRenderPasses.push_back(new Material::Pass);
		m->addProperty(
			new StandardProperty(
//				ColorRGBAf(0.5f, material.fMatOpacity),
//				ColorRGBAf(1, material.fMatOpacity),
//				ColorRGBAf(0.2f, material.fMatOpacity),
				ColorRGBAf(reinterpret_cast<const ColorRGBf&>(*material.pfMatAmbient), material.fMatOpacity),
				ColorRGBAf(reinterpret_cast<const ColorRGBf&>(*material.pfMatDiffuse), material.fMatOpacity),
				ColorRGBAf(reinterpret_cast<const ColorRGBf&>(*material.pfMatSpecular), material.fMatOpacity),
				ColorProperty::ColorOperation::Replace,
				material.fMatShininess * 100	// TODO: There is some problem in the shininess value
			), 0
		);

		if(material.nIdxTexDiffuse >= 0 && material.nIdxTexDiffuse < int(mTextures.size()))
			m->addProperty(new TextureProperty(mTextures[material.nIdxTexDiffuse].get(), 0, GL_LINEAR, GL_LINEAR), 0);

		mMaterials.push_back(m);
	}

	// Generate meshes
	for(size_t i=0; i<mPod.nNumMesh; ++i)
	{
		MeshPtr mesh = new Mesh();
		const SPODMesh& podMesh = mPod.pMesh[i];
		mMeshes.push_back(std::make_pair(mesh, std::vector<void*>()));
		std::vector<void*>& bufferPtrs = mMeshes[i].second;

		if(podMesh.ePrimitiveType != ePODTriangles)	// We only support triangle
			continue;
		if(podMesh.nNumStrips > 0 || podMesh.pnStripLength)	// Triangle strip is not supported (yet?)
			continue;

		const SemanticMap& semanticMap = SemanticMap::getSingleton();

		// Index
		if(podMesh.sFaces.n) {
			mesh->indexAttrIdx = uint8_t(mesh->attributeCount);
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];

			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.sFaces, true);
			a.semantic = semanticMap.index().name;
		}
		else {
			Log::write(Log::Warn, L"Empty mesh data in pod");
			continue;
		}

		// Vertex position
		if(podMesh.sVertex.n) {
			mesh->positionAttrIdx = uint8_t(mesh->attributeCount);
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];

			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.sVertex);
			a.semantic = semanticMap.position().name;
		}
		else {
			Log::write(Log::Warn, L"Empty mesh data in pod");
			continue;
		}

		// Vertex normal
		if(podMesh.sNormals.n) {
			mesh->normalAttrIdx = uint8_t(mesh->attributeCount);
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];

			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.sNormals);
			a.semantic = semanticMap.normal().name;
		}

		// UV channels
		for(size_t j=0; j<podMesh.nNumUVW; ++j) {
			if(j == 0) mesh->uv0AttrIdx = uint8_t(mesh->attributeCount);
			if(j == 1) mesh->uv1AttrIdx = uint8_t(mesh->attributeCount);
			if(j == 2) mesh->uv2AttrIdx = uint8_t(mesh->attributeCount);
			if(j > 2) continue;

			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];

			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.psUVW[j]);
			a.semantic = semanticMap.uv(j, a.elementCount).name;
		}

		if(podMesh.pInterleaved) {
			mesh->bufferCount++;
			bufferPtrs.push_back(podMesh.pInterleaved);
		}

		mesh->vertexCount = podMesh.nNumVertex;
		mesh->indexCount = podMesh.nNumFaces * 3;

		// NOTE: The upload of vertex data is postponed until commit() which is run in main thread.
	}

	// Index of pod node to Entity
	std::vector<EntityPtr> nodeToEntity;
	nodeToEntity.push_back(&mRootEntity);	// SPODNode::nIdxParent gives -1 for no parent

	// Loop for all nodes
	for(size_t i=0; i<mPod.nNumNode; ++i)
	{
		const SPODNode& podNode = mPod.pNode[i];

		if(podNode.nIdx < 0 || podNode.nIdx >= int(mMeshes.size()))
			continue;

		EntityPtr e = new Entity();
		e->name = strToWStr(podNode.pszName);

		if(podNode.nIdxParent + 1 >= int(nodeToEntity.size()))
			continue;

		e->asChildOf(nodeToEntity[podNode.nIdxParent + 1].getNotNull());	// SPODNode::nIdxParent gives -1 for no parent

		{	// Calculate the local matrix
			PVRTMat4& mat = reinterpret_cast<PVRTMat4&>(e->localTransform);
			PVRTMat4 tmp;

			mPod.GetScalingMatrix(mat, podNode);
			mPod.GetRotationMatrix(tmp, podNode);
			PVRTMatrixMultiply(mat, mat, tmp);
			mPod.GetTranslationMatrix(tmp, podNode);
			PVRTMatrixMultiply(mat, mat, tmp);

			mat = mat.transpose();
		}

		// Add mesh component if 1) It's a pod mesh node and 2) The mesh format is supported by our loader.
		if(i < mPod.nNumMeshNode && mMeshes[podNode.nIdx].first->vertexCount > 0) {
			MeshComponent* c = new MeshComponent;
			c->mesh = mMeshes[podNode.nIdx].first;
			e->addComponent(c);

			// Setup for material
			if(podNode.nIdxMaterial >= 0 && podNode.nIdxMaterial < int(mMaterials.size())) {
				c->effect = new Effect(L"");
				c->effect->material.reset(mMaterials[podNode.nIdxMaterial].clone());
			}
		}
	}

	return mLoadingState = Loaded;
}

void PodLoader::Impl::commit(Resource& resource)
{
	// There is no need to do a mutex lock because PodLoader didn't support progressive loading.
	// Therefore, commit will not be invoked if the load() function itsn't finished.
	MCD_ASSERT(mLoadingState == Loaded);

	// Commit all the mesh buffers
	for(size_t i=0; i<mPod.nNumMesh; ++i) {
		// TODO: Optimization, ignore those mesh in mMeshes with reference count == 1
		Mesh& mesh = *mMeshes[i].first;

		MCD_ASSERT(!mPod.pMesh[i].pInterleaved || mesh.bufferCount <= 2);

		for(size_t j=0; j<mesh.bufferCount; ++j) {
			uint* handle = mesh.handles[j].get();
			MCD_ASSUME(handle);
			if(!*handle)
				glGenBuffers(1, handle);

			const GLenum verOrIdxBuf = j == 0 ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
			glBindBuffer(verOrIdxBuf, *handle);
			glBufferData(verOrIdxBuf, mesh.bufferSize(j), mMeshes[i].second[j], Mesh::Static);	// TODO: Way to set the Mesh::StorageHint
		}

		if(false && mesh.uv0AttrIdx > -1) {	// I don't know why there is a need of flipping the UV in y-direction, need to better understand the pod format spec.
			Mesh::MappedBuffers mapped;
			StrideArray<Vec2f> uv = mesh.mapAttribute<Vec2f>(mesh.uv0AttrIdx, mapped, Mesh::MapOption(Mesh::Read | Mesh::Write));
			for(size_t k=0; k<uv.size; ++k) {
				uv[k].y = 1.0f - uv[k].y;
			}
			mesh.unmapBuffers(mapped);
		}
	}

	Prefab& prefab = dynamic_cast<Prefab&>(resource);
	prefab.entity.reset(new Entity());

	// Clone all the node in mRootEntity to the target.
	for(Entity* i=mRootEntity.firstChild(); i; i = i->nextSibling()) {
		Entity* e = i->clone();
		MCD_ASSERT(e);
		e->asChildOf(prefab.entity.get());
	}
}

PodLoader::PodLoader(IResourceManager* resourceManager)
	: mImpl(*new Impl(resourceManager))
{
}

PodLoader::~PodLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState PodLoader::load(std::istream* is, const Path* fileId, const wchar_t* args)
{
	MemoryProfiler::Scope scope("PodLoader::load");
	return mImpl.load(is, fileId, args);
}

void PodLoader::commit(Resource& resource)
{
	return mImpl.commit(resource);
}

IResourceLoader::LoadingState PodLoader::getLoadingState() const
{
	return mImpl.mLoadingState;
}


PodLoaderFactory::PodLoaderFactory(IResourceManager& resourceManager)
    : mResourceManager(resourceManager)
{
}

ResourcePtr PodLoaderFactory::createResource(const Path& fileId, const wchar_t* args)
{
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"pod") == 0)
		return new Prefab(fileId);
	return nullptr;
}

IResourceLoader* PodLoaderFactory::createLoader()
{
	return new PodLoader(&mResourceManager);
}

}	// namespace MCD
