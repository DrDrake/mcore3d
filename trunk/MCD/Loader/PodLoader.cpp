#include "Pch.h"
#include "PodLoader.h"
#include "../Component/Render/AnimationComponent.h"
#include "../Component/Render/MeshComponent.h"
#include "../Component/Render/SkinMeshComponent.h"
#include "../Component/Render/SkeletonAnimationComponent.h"
#include "../Component/Prefab.h"
#include "../Render/Effect.h"
#include "../Render/Material.h"
#include "../Render/Mesh.h"
#include "../Render/Model.h"
#include "../Render/SemanticMap.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/StrUtility.h"
#include "../Core/Math/AnimationInstance.h"
#include "../Core/Math/Skeleton.h"
#include "../Core/Math/Quaternion.h"
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
	Impl(IResourceManager* resourceManager, AnimationUpdaterComponent* animationUpdater, SkeletonAnimationUpdaterComponent* skeletonAnimationUpdater)
		: mResourceManager(resourceManager), mAnimationUpdater(animationUpdater), mSkeletonAnimationUpdater(skeletonAnimationUpdater)
	{
	}

	~Impl() {
		mPod.Destroy();
	}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const char* args);

	void commit(Resource& resource);

	IResourceManager* mResourceManager;
	AnimationUpdaterComponent* mAnimationUpdater;
	SkeletonAnimationUpdaterComponent* mSkeletonAnimationUpdater;

	Entity mRootEntity;
	CPVRTModelPOD mPod;

	//!	Store a list of Mesh that is pending to commit the buffer (which must done in main thread).
	std::vector<std::pair<MeshPtr, std::vector<void*> > > mMeshes;

	std::vector<TexturePtr> mTextures;
	ptr_vector<PodMaterial> mMaterials;

	std::vector<std::pair<SkinMeshComponentPtr, ModelPtr> > mSkinMeshToCommit;

	volatile IResourceLoader::LoadingState mLoadingState;
};	// Impl

static int podTypeToGlType(EPVRTDataType type)
{
	switch(type) {
	case EPODDataUnsignedByte:	return GL_UNSIGNED_BYTE;
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
	case EPODDataUnsignedByte:	return sizeof(unsigned char);
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

static void getLocalTransform(const CPVRTModelPOD& pod, const SPODNode& node, Mat44f& ret)
{
	PVRTMat4& mat = reinterpret_cast<PVRTMat4&>(ret);
	PVRTMat4 tmp;

	pod.GetScalingMatrix(mat, node);
	pod.GetRotationMatrix(tmp, node);
	PVRTMatrixMultiply(mat, mat, tmp);
	pod.GetTranslationMatrix(tmp, node);
	PVRTMatrixMultiply(mat, mat, tmp);

	mat = mat.transpose();
}

static StrideArray<Vec3f> getVertexData(const SPODMesh& mesh)
{
	StrideArray<Vec3f> ret(nullptr, mesh.nNumVertex, mesh.sVertex.nStride);
	if(mesh.pInterleaved)
		ret.data = (char*)(mesh.pInterleaved + size_t(mesh.sVertex.pData));
	else
		ret.data = (char*)mesh.sVertex.pData;
	return ret;
}

static StrideArray<uint8_t> getJointIdx(const SPODMesh& mesh)
{
	StrideArray<uint8_t> ret(nullptr, mesh.nNumVertex, mesh.sBoneIdx.nStride);
	if(mesh.pInterleaved)
		ret.data = (char*)(mesh.pInterleaved + size_t(mesh.sBoneIdx.pData));
	else
		ret.data = (char*)mesh.sBoneIdx.pData;
	return ret;
}

static bool isSkinMeshNode(const CPVRTModelPOD& pod, size_t i)
{
	if(i >= pod.nNumMesh)
		return false;

	const SPODNode& podNode = pod.pNode[i];
	const SPODMesh& podMesh = pod.pMesh[podNode.nIdx];
	return podMesh.sBoneIdx.n > 0;
}

IResourceLoader::LoadingState PodLoader::Impl::load(std::istream* is, const Path* fileId, const char* args)
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
		std::string texturePath(mPod.pTexture[i].pszName);

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
			Log::write(Log::Warn, "Empty mesh data in pod");
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
			Log::write(Log::Warn, "Empty mesh data in pod");
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

		if(podMesh.sBoneIdx.n) {
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];
			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.sBoneIdx);
			a.semantic = semanticMap.blendIndex().name;
		}

		if(podMesh.sBoneWeight.n) {
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];
			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.sBoneWeight);
			a.semantic = semanticMap.blendWeight().name;
		}

		if(podMesh.pInterleaved) {
			mesh->bufferCount++;
			bufferPtrs.push_back(podMesh.pInterleaved);
		}

		mesh->vertexCount = podMesh.nNumVertex;
		mesh->indexCount = podMesh.nNumFaces * 3;

		// NOTE: The upload of vertex data is postponed until commit() which is run in main thread.
	}

	// Create animation track
	AnimationTrackPtr track = new AnimationTrack("");
	track->naturalFramerate = 30;

	// Create skeleton animation
	SkeletonPtr skeleton = new Skeleton("");

	static const size_t cExtraRootNode = 1;

	// Determines the sub-track structure
	std::vector<size_t> subTrackStructure(AnimationComponent::subtrackPerEntity * cExtraRootNode, 1);

	for(size_t i=0; i<mPod.nNumNode; ++i)
	{
		// We share the same animation track if the pod file contains both node and skeleton animation,
		// as node animation needs at least 4 sub-track, there will be redudant tracks when use as skeleton.
		size_t frames[AnimationComponent::subtrackPerEntity];
		for(size_t j=0; j<AnimationComponent::subtrackPerEntity; ++j) frames[j] = 1;

		if(mPod.pNode[i].nAnimFlags & ePODHasPositionAni)
			frames[0] = mPod.nNumFrame;
		if(mPod.pNode[i].nAnimFlags & ePODHasRotationAni)
			frames[1] = mPod.nNumFrame;
		if(mPod.pNode[i].nAnimFlags & ePODHasScaleAni)
			frames[2] = mPod.nNumFrame;

		subTrackStructure.insert(subTrackStructure.end(), &frames[0], &frames[4]);
	}

	if(!subTrackStructure.empty())
	{	// Fill the animation track
		AnimationTrack::ScopedWriteLock lock(*track);
		MCD_VERIFY(track->init(StrideArray<const size_t>(&subTrackStructure[0], subTrackStructure.size())));

		// Assign the time of each frame
		for(size_t i=0; i<subTrackStructure.size(); ++i) {
			AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(i);
			for(size_t j=0; j<frames.size; ++j)
				frames[j].time = float(j);
		}

		// Assign for the extra root node
		{	AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(0);
			reinterpret_cast<Vec3f&>(frames[0]) = Vec3f(0);
			frames = track->getKeyFramesForSubtrack(1);
			reinterpret_cast<Quaternionf&>(frames[0]) = Quaternionf::cIdentity;
			frames = track->getKeyFramesForSubtrack(2);
			reinterpret_cast<Vec3f&>(frames[0]) = Vec3f(1);
		}

		size_t j = cExtraRootNode;
		for(size_t i=0; i<mPod.nNumNode; ++i)
		{
			const SPODNode& podNode = mPod.pNode[i];
			const size_t subTrackOffset = j * AnimationComponent::subtrackPerEntity;

			// Translation
			AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(subTrackOffset + 0);
			for(size_t k=0; k<frames.size; ++k)
				reinterpret_cast<Vec3f&>(frames[k]) = reinterpret_cast<Vec3f&>(podNode.pfAnimPosition[k * 3]);

			// Rotation
			frames = track->getKeyFramesForSubtrack(subTrackOffset + 1);
			track->subtracks[subTrackOffset + 1].flag = AnimationTrack::Slerp;
			for(size_t k=0; k<frames.size; ++k)
				reinterpret_cast<Quaternionf&>(frames[k]) = reinterpret_cast<Quaternionf&>(podNode.pfAnimRotation[k * 4]).inverseUnit();

			// Sclae
			frames = track->getKeyFramesForSubtrack(subTrackOffset + 2);
			for(size_t k=0; k<frames.size; ++k)
				reinterpret_cast<Vec3f&>(frames[k]) = reinterpret_cast<Vec3f&>(podNode.pfAnimScale[k * 7]);	// Don't know why the stride of scale is 7

			++j;	// Move to next node with animation
		}
	}

	bool hasSkeleton = false;

	if(!subTrackStructure.empty())
	{	// Setup the skeleton
		skeleton->init(subTrackStructure.size() / AnimationComponent::subtrackPerEntity);
		skeleton->basePose.init(subTrackStructure.size() / AnimationComponent::subtrackPerEntity);

		for(size_t i=0; i<mPod.nNumNode; ++i) {
			const SPODNode& podNode = mPod.pNode[i];
			skeleton->names[i + cExtraRootNode] = podNode.pszName;
			skeleton->parents[i + cExtraRootNode] = podNode.nIdxParent < 0 ? 0 : podNode.nIdxParent + cExtraRootNode;
		}

		for(size_t i=0; i<mPod.nNumMesh; ++i)
		{
			const SPODMesh& podMesh = mPod.pMesh[i];
			if(!podMesh.sBoneIdx.pData || !podMesh.sBoneWeight.pData)
				continue;

			std::vector<bool> boneIdxRemapAssigned(podMesh.nNumVertex, false);

			StrideArray<uint8_t> jointIdx = getJointIdx(podMesh);

			const CPVRTBoneBatches& boneBatches = podMesh.sBoneBatches;
			for(int iBatch = 0; iBatch < boneBatches.nBatchCnt; ++iBatch)
			{
				std::vector<size_t> boneIdxRemap;

				// Go through the bones for the current bone batch
				for(int b=0; b<boneBatches.pnBatchBoneCnt[iBatch]; ++b) {
					// Get the Node of the bone
					int nodeIdx = boneBatches.pnBatches[iBatch * boneBatches.nBatchBoneMax + b];
					boneIdxRemap.push_back(nodeIdx + cExtraRootNode);
					hasSkeleton = true;
				}

				// Transform the original bone index to match our flattened bone patch
				size_t triBegin = podMesh.sBoneBatches.pnBatchOffset[iBatch];	// Begining triangle for this patch
				size_t triEnd = iBatch+1 < boneBatches.nBatchCnt ? boneBatches.pnBatchOffset[iBatch+1] : podMesh.nNumFaces;	// End triangle for this patch

				for(size_t t=triBegin; t<triEnd; ++t) for(size_t iv=0; iv<3; ++iv) {
					const size_t v = ((uint16_t*)(podMesh.sFaces.pData))[t*3 + iv];
					if(boneIdxRemapAssigned[v])
						continue;
					boneIdxRemapAssigned[v] = true;

					for(size_t j=0; j<podMesh.sBoneIdx.n; ++j) {
						uint8_t& val = *(&jointIdx[v] + j);
						const size_t newIdx = boneIdxRemap[val];

						MCD_ASSERT(newIdx < 255);
						val = uint8_t(newIdx);
					}
				}
			}
		}

		// Bake the mesh's transform into the vertex, for the joint matrix inverse to work correctly.
		if(hasSkeleton) for(size_t i=0; i<mPod.nNumMeshNode; ++i)
		{
			const SPODNode& podNode = mPod.pNode[i];
			const SPODMesh& podMesh = mPod.pMesh[podNode.nIdx];

			Mat44f mat;
			getLocalTransform(mPod, podNode, mat);

			StrideArray<Vec3f> vertex = getVertexData(podMesh);
			for(size_t j=0; j<vertex.size; ++j)
				mat.transformPoint(vertex[j]);
		}

		if(!hasSkeleton)
			skeleton = nullptr;
	}

	// Create AnimationComponent if necessary
	AnimationComponentPtr animationComponent;
	if(mAnimationUpdater && mPod.nNumFrame > 0) {
		EntityPtr e = new Entity();
		e->name = "Animation controller";
		animationComponent = new AnimationComponent(*mAnimationUpdater);
		e->addComponent(animationComponent.get());
		e->asChildOf(&mRootEntity);
	}

	// Create SkeletonAnimationComponent if necessary
	SkeletonAnimationComponentPtr skeletonAnimationComponent;
	if(mSkeletonAnimationUpdater && skeleton) {
		skeletonAnimationComponent = new SkeletonAnimationComponent(*mSkeletonAnimationUpdater);
		EntityPtr e = new Entity();
		e->name = "Skeleton animation controller";
		e->addComponent(skeletonAnimationComponent.get());
		e->asChildOf(&mRootEntity);

		skeletonAnimationComponent->skeletonAnimation.skeleton = skeleton;
		skeletonAnimationComponent->skeletonAnimation.anim.addTrack(*track);
		skeletonAnimationComponent->skeletonAnimation.anim.update();
		skeletonAnimationComponent->skeletonAnimation.applyTo(skeleton->basePose);	// Use the first frame's pose as the base pose
		skeletonAnimationComponent->pose.init(skeleton->basePose.jointCount());
		skeleton->initBasePoseInverse();
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

		if(podNode.nIdxParent + 1 >= int(nodeToEntity.size()))
			continue;

		EntityPtr e = new Entity();
		e->name = podNode.pszName;
		e->asChildOf(nodeToEntity[podNode.nIdxParent + 1].getNotNull());	// SPODNode::nIdxParent gives -1 for no parent

		if(!hasSkeleton)
			getLocalTransform(mPod, podNode, e->localTransform);

		nodeToEntity.push_back(e);

		// Add mesh component if 1) It's a pod mesh node and 2) The mesh format is supported by our loader.
		if(i < mPod.nNumMeshNode && mMeshes[podNode.nIdx].first->vertexCount > 0)
		{
			// Setup for material
			EffectPtr effect = new Effect("");
			if(podNode.nIdxMaterial >= 0 && podNode.nIdxMaterial < int(mMaterials.size()))
				effect->material.reset(mMaterials[podNode.nIdxMaterial].clone());

			if(mPod.pMesh[podNode.nIdx].sBoneIdx.n) {
				SkinMeshComponentPtr sm = new SkinMeshComponent();

				ModelPtr model = new Model(podNode.pszName);
				Model::MeshAndMaterial* mm = new Model::MeshAndMaterial;
				mm->mesh = mMeshes[podNode.nIdx].first;
				mm->effect = effect;

				mSkinMeshToCommit.push_back(std::make_pair(sm, model));

				model->mMeshes.pushBack(*mm);
				sm->skeleton = skeleton;
				sm->skeletonAnimation = skeletonAnimationComponent;

				e->addComponent(sm.get());
			}
			else {
				MeshComponent* c = new MeshComponent;
				c->mesh = mMeshes[podNode.nIdx].first;
				c->effect = effect;
				e->addComponent(c);
			}
		}
	}

	if(mAnimationUpdater && animationComponent) {
		// Initialize animation track
		MCD_VERIFY(animationComponent->animationInstance.addTrack(*track));

		// Calculate animation data and link up with the entities
		animationComponent->affectingEntities.push_back(nullptr);
		for(size_t i=cExtraRootNode; i<nodeToEntity.size(); ++i) {
			EntityPtr e = isSkinMeshNode(mPod, i-cExtraRootNode) ? nullptr : nodeToEntity[i];
			animationComponent->affectingEntities.push_back(e);
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

	// Commit skin mesh
	for(size_t i=0; i<mSkinMeshToCommit.size(); ++i) {
		mSkinMeshToCommit[i].first->init(*mResourceManager, *mSkinMeshToCommit[i].second);
	}

	Prefab& prefab = dynamic_cast<Prefab&>(resource);
	prefab.entity.reset(new Entity());

	// Clone all the node in mRootEntity to the target.
	Entity* cloned = mRootEntity.clone();
	for(Entity* i=cloned->firstChild(); i;) {
		Entity* bk = i;
		i = i->nextSibling();
		bk->asChildOf(prefab.entity.get());
	}
}

PodLoader::PodLoader(
	IResourceManager* resourceManager,
	AnimationUpdaterComponent* animationUpdater,
	SkeletonAnimationUpdaterComponent* skeletonAnimationUpdater
)
	: mImpl(*new Impl(resourceManager, animationUpdater, skeletonAnimationUpdater))
{
}

PodLoader::~PodLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState PodLoader::load(std::istream* is, const Path* fileId, const char* args)
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


PodLoaderFactory::PodLoaderFactory(
	IResourceManager& resourceManager,
	AnimationUpdaterComponent* animationUpdater,
	SkeletonAnimationUpdaterComponent* skeletonAnimationUpdater
)
	: mResourceManager(resourceManager)
	, mAnimationUpdater(animationUpdater)
	, mSkeletonAnimationUpdater(skeletonAnimationUpdater)
{
}

ResourcePtr PodLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "pod") == 0)
		return new Prefab(fileId);
	return nullptr;
}

IResourceLoader* PodLoaderFactory::createLoader()
{
	return new PodLoader(&mResourceManager, mAnimationUpdater, mSkeletonAnimationUpdater);
}

}	// namespace MCD
