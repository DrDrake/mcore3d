#include "Pch.h"
#include "FbxModelLoader.h"
#include "FbxFile.h"
#include "ModelImporter.h"

#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Core/System/MemoryProfiler.h"
#include "../../MCD/Core/System/StrUtility.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Mesh.h"

namespace MCD
{

/*! FbxModelLoader */
class FbxModelLoader::Impl
{
public:
	Impl(IResourceManager* resourceManager);

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const wchar_t* args);

	void commit(Resource& resource);

	IResourceLoader::LoadingState getLoadingState() const;

private:
	IResourceManager* mResourceManager;

	std::auto_ptr<FbxFile> mFbxFile;

	volatile IResourceLoader::LoadingState mLoadingState;
	mutable Mutex mMutex;
};	// Impl

FbxModelLoader::Impl::Impl(IResourceManager* resourceManager)
	: mResourceManager(resourceManager)
{
}

IResourceLoader::LoadingState FbxModelLoader::Impl::load(std::istream* is, const Path* fileId, const wchar_t* args)
{
	ScopeLock lock(mMutex);

	mLoadingState = is ? NotLoaded : Aborted;

	if(mLoadingState & Stopped)
		return mLoadingState;

	mFbxFile.reset(new FbxFile(fileId->getString().c_str()));

	bool ok;

	{	// Unlock the mutex and ready for the actual load
		ScopeUnlock unlock(mMutex);
		ok = mFbxFile->open();
	}

	return (mLoadingState = ok ? Loaded : Aborted);
}

void FbxModelLoader::Impl::commit(Resource& resource)
{
	// There is no need to do a mutex lock because Max3dsLoader didn't support progressive loading.
	// Therefore, commit will not be invoked if the load() function itsn't finished.

	MCD_ASSUME(mFbxFile.get() != nullptr);

	Model& model = dynamic_cast<Model&>(resource);
	ModelImporter(mResourceManager).import(*mFbxFile, model, true);
}

IResourceLoader::LoadingState FbxModelLoader::Impl::getLoadingState() const
{
	ScopeLock lock(mMutex);

	return mLoadingState;
}

FbxModelLoader::FbxModelLoader(IResourceManager* resourceManager)
{
	mImpl = new Impl(resourceManager);
}

FbxModelLoader::~FbxModelLoader()
{
	delete mImpl;
}

IResourceLoader::LoadingState FbxModelLoader::load(std::istream* is, const Path* fileId, const wchar_t* args)
{
	MemoryProfiler::Scope scope("FbxModelLoader::load");
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->load(is, fileId, args);
}

void FbxModelLoader::commit(Resource& resource)
{
	MemoryProfiler::Scope scope("FbxModelLoader::commit");
	MCD_ASSUME(mImpl != nullptr);
	mImpl->commit(resource);
}

IResourceLoader::LoadingState FbxModelLoader::getLoadingState() const
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->getLoadingState();
}

FbxModelLoaderFactory::FbxModelLoaderFactory(IResourceManager& resourceManager)
    : mResourceManager(resourceManager)
{
}

ResourcePtr FbxModelLoaderFactory::createResource(const Path& fileId, const wchar_t* args)
{
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"fbx") != 0)
		return nullptr;

	return new Model(fileId);
}

IResourceLoader* FbxModelLoaderFactory::createLoader()
{
	return new FbxModelLoader(&mResourceManager);
}

}	// namespace MCD
