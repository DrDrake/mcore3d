#include "Pch.h"
#include "ModelPod.h"
#include "Texture.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/Utility.h"
#include "../../3Party/glew/glew.h"
#include "../../3Party/PowerVR/PVRTModelPOD.h"
#include <vector>

#ifdef MCD_VC
#	pragma comment(lib, "PowerVR")
#endif	// MCD_VC

namespace MCD {

class ModelPod::Impl
{
public:
	Impl();
	~Impl();

	void initBuffers();

	void draw();

	volatile bool mPodLoaded;
	CPVRTModelPOD mPod;

	GLuint* mVertexBuffers;
	GLuint* mNormalBuffers;
	GLuint* mCoordBuffers;
	GLuint* mIndexBuffers;

	typedef std::vector<TexturePtr> Textures;
	Textures mTextures;
};

ModelPod::Impl::Impl()
{
	mPodLoaded = false;
	mVertexBuffers = nullptr;
	mNormalBuffers = nullptr;
	mCoordBuffers = nullptr;
	mIndexBuffers = nullptr;
}

ModelPod::Impl::~Impl()
{
	size_t meshCount = mPod.nNumMesh;
	glDeleteBuffers(meshCount, mVertexBuffers);
	glDeleteBuffers(meshCount, mNormalBuffers);
	glDeleteBuffers(meshCount, mCoordBuffers);
	glDeleteBuffers(meshCount, mIndexBuffers);

	delete[] mVertexBuffers;
	delete[] mNormalBuffers;
	delete[] mCoordBuffers;
	delete[] mIndexBuffers;

	mPod.Destroy();
}

void ModelPod::Impl::initBuffers()
{
	MCD_ASSERT(!mPodLoaded);
	mPodLoaded = true;

	size_t meshCount = mPod.nNumMesh;
	if(meshCount == 0)
		return;

	mVertexBuffers = new GLuint[meshCount];
	glGenBuffers(meshCount, mVertexBuffers);
	mNormalBuffers = new GLuint[meshCount];
	glGenBuffers(meshCount, mNormalBuffers);
	mCoordBuffers = new GLuint[meshCount];
	glGenBuffers(meshCount, mCoordBuffers);
	mIndexBuffers = new GLuint[meshCount];
	glGenBuffers(meshCount, mIndexBuffers);

	for(size_t i=0; i<meshCount; ++i)
	{
		SPODMesh* mesh = mPod.pMesh + i;
		MCD_ASSUME(mesh);

		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffers[i]);
		glBufferData(GL_ARRAY_BUFFER, mesh->nNumVertex * 3 * sizeof(GLfloat),
					 mesh->sVertex.pData, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffers[i]);
		glBufferData(GL_ARRAY_BUFFER, mesh->nNumVertex * 3 * sizeof(GLfloat),
					 mesh->sNormals.pData, GL_STATIC_DRAW);

		if(mesh->nNumUVW > 0) {
			glBindBuffer(GL_ARRAY_BUFFER, mCoordBuffers[i]);
			// TODO: Currently only support 1 channel
			glBufferData(GL_ARRAY_BUFFER, mesh->nNumVertex * 2 * sizeof(GLfloat),
						 mesh->psUVW[0].pData, GL_STATIC_DRAW);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffers[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->nNumFaces * 3 * sizeof(GLushort),
					 mesh->sFaces.pData, GL_STATIC_DRAW);

	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ModelPod::Impl::draw()
{
	if(!mPodLoaded)
		return;

	// Iterate through all the mesh nodes in the scene
	for(size_t i = 0; i < mPod.nNumMeshNode; ++i)
	{
		// Get the mesh node.
		SPODNode* node = mPod.pNode + i;
		MCD_ASSUME(node);

		size_t meshIdx = node->nIdx;
		SPODMesh* mesh = mPod.pMesh + meshIdx;
		MCD_ASSUME(mesh);

//		if(node->nIdxMaterial < 0)
//			continue;

		if(mPod.pMaterial && node->nIdxMaterial >= 0)
		{
			SPODMaterial* material = mPod.pMaterial + node->nIdxMaterial;
			MCD_ASSUME(material);

			{	// Standard material
				float color[4];
				color[3] = 1.0f;

				for(size_t j=3; j--;)
					color[j] = material->pfMatAmbient[j];
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);

				for(size_t j=3; j--;)
					color[j] = material->pfMatDiffuse[j];
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

				for(size_t j=3; j--;)
					color[j] = material->pfMatSpecular[j];
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->fMatShininess);
			}

			if(material->nIdxTexDiffuse >= 0 && material->nIdxTexDiffuse < int(mPod.nNumTexture)) {
				Texture* texture = mTextures[material->nIdxTexDiffuse].get();
				if(texture) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

					texture->bind();
				}
			}
		}

		// Gets the node model matrix
		PVRTMATRIX world;
		mPod.GetWorldMatrix(world, *node);
		glPushMatrix();
		glMultMatrixf(world.f);

		// Set Data Pointers
		if(mesh->nNumUVW > 0) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glClientActiveTexture(GL_TEXTURE0);
			glBindBuffer(GL_ARRAY_BUFFER, mCoordBuffers[meshIdx]);
			glTexCoordPointer(2, GL_FLOAT, 0, 0);
		}

		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffers[meshIdx]);
		glNormalPointer(GL_FLOAT, 0, 0/*mesh->sNormals.pData*/);

		// Calling glVertexPointer() as late as possible will have a big performance difference!
		// Reference: http://developer.nvidia.com/object/using_VBOs.html
		glEnableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffers[meshIdx]);
		glVertexPointer(3, GL_FLOAT, 0, 0/*mesh->sVertex.pData*/);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffers[meshIdx]);
		// TODO: Use glDrawRangeElements
		glDrawElements(GL_TRIANGLES, mesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0/*mesh->sFaces.pData*/);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		glPopMatrix();
	}
}

ModelPod::ModelPod(const Path& fileId)
	: Resource(fileId)
{
	mImpl = new Impl;
}

ModelPod::~ModelPod()
{
	delete mImpl;
}

void ModelPod::draw()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->draw();
}

}	// namespace MCD

#include "../Core/System/ResourceManager.h"
#include "../Core/System/Log.h"

namespace MCD {

class PodLoader::Impl
{
public:
	~Impl();

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId);

	void commit(Resource& resource);

	CPVRTModelPOD mPod;
	Path mPath;
	ResourceManager* mResourceManager;
	volatile IResourceLoader::LoadingState mLoadingState;
};

PodLoader::Impl::~Impl()
{
	mPod.Destroy();
}

IResourceLoader::LoadingState PodLoader::Impl::load(std::istream* is, const Path* fileId)
{
	mLoadingState = is ? NotLoaded : Aborted;

	if(mLoadingState & Stopped)
		return mLoadingState;

	EPVRTError result = mPod.ReadFromStream(*is);

	if(result == PVR_SUCCESS) {
		if(fileId)
			mPath = *fileId;
		mLoadingState = Loaded;
	}
	else {
		mPod.Destroy();	// Be more safe
		mLoadingState = Aborted;
	}

	return mLoadingState;
}

void PodLoader::Impl::commit(Resource& resource)
{
	ModelPod& model = dynamic_cast<ModelPod&>(resource);
	CPVRTModelPOD& pod = model.mImpl->mPod;

	// TODO: Error checking
	pod.CopyFromMemory(mPod);

	// Load textures
	model.mImpl->mTextures.resize(pod.nNumTexture);

	if(pod.pTexture) for(size_t i=0; i<pod.nNumTexture; ++i)
	{
		std::wstring texturePath;

		if(!strToWStr(pod.pTexture[i].pszName, texturePath)) {
			Log::format(Log::Warn, L"Invalid texture path when loading %s", mPath.getString().c_str());
			continue;
		}

		// We assume the texture path is relative to the pod file.
		Path adjustedPath = mPath.hasRootDirectory() ? texturePath : mPath.getBranchPath() / texturePath;

		if(mResourceManager)
			model.mImpl->mTextures[i] = dynamic_cast<Texture*>(mResourceManager->load(adjustedPath, false).get());
		else
			model.mImpl->mTextures[i] = new Texture(adjustedPath);
	}

	model.mImpl->initBuffers();
}

PodLoader::PodLoader(ResourceManager* resourceManager)
{
	mImpl = new Impl;
	mImpl->mResourceManager = resourceManager;
}

PodLoader::~PodLoader()
{
	delete mImpl;
}

IResourceLoader::LoadingState PodLoader::load(std::istream* is, const Path* fileId)
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->load(is, fileId);
}

void PodLoader::commit(Resource& resource)
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->commit(resource);
}

IResourceLoader::LoadingState PodLoader::getLoadingState() const
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->mLoadingState;
}

}	// namespace MCD
