#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Loader/ResourceLoaderFactory.h"
#include "../../MCD/Core/System/FileSystemCollection.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/RawFileSystemMonitor.h"
#include "../../MCD/Core/System/Resource.h"
#include "../../MCD/Core/System/ResourceLoader.h"
#include "../../MCD/Core/System/ZipFileSystem.h"
#include "../../MCD/Component/Render/EntityPrototypeLoader.h"

using namespace MCD;

IFileSystem* createDefaultFileSystem()
{
	std::auto_ptr<FileSystemCollection> fileSystem(new FileSystemCollection);

	Path actualRoot;

	try {
		std::auto_ptr<IFileSystem> rawFs(new RawFileSystem("Media"));
		actualRoot = rawFs->getRoot();
		fileSystem->addFileSystem(*rawFs.release());
	} catch(...) {}

	try {
		std::auto_ptr<IFileSystem> zipFs(new ZipFileSystem(actualRoot.getBranchPath() / "Media.zip"));
		fileSystem->addFileSystem(*zipFs.release());
	} catch(...) {}

	return fileSystem.release();
}

class DefaultResourceManager::Impl
{
public:
	Impl(IFileSystem& fileSystem)
		: mMonitor(fileSystem.getRoot().getString().c_str(), true)
		, mFileSystem(fileSystem)
	{
	}

	MCD::RawFileSystemMonitor mMonitor;
	IFileSystem& mFileSystem;
};	// Impl

DefaultResourceManager::DefaultResourceManager(IFileSystem& fileSystem)
	: ResourceManager(fileSystem)
	, mImpl(*new Impl(fileSystem))
{
	setupFactories();
}

DefaultResourceManager::~DefaultResourceManager()
{
	delete &mImpl;
}

MCD::IFileSystem& DefaultResourceManager::fileSystem() const
{
	return mImpl.mFileSystem;
}

int DefaultResourceManager::processLoadingEvents()
{
	{	// Reload any changed files in the RawFileSystem
		std::string path;
		while(!(path = mImpl.mMonitor.getChangedFile()).empty())
			reload(Path(path).normalize(), IResourceManager::NonBlock);
	}

	ResourceManager::Event e = popEvent();
	if(e.loader) {
		const IResourceLoader::LoadingState loadingState = e.loader->getLoadingState();
		const bool hasError = loadingState == IResourceLoader::Aborted;

		if(hasError)
			Log::format(Log::Warn, "Resource: %s %s", e.resource->fileId().getString().c_str(), "failed to load");
		else if(loadingState != IResourceLoader::Loading)
			e.loader->commit(*e.resource);	// Allow one resource to commit for each frame

		// Note that commit() is invoked before doCallbacks()
		doCallbacks(e);

		return hasError ? -1 : 1;
	}

	return 0;
}

void DefaultResourceManager::setupFactories()
{
	addFactory(new AnimationTrackLoaderFactory);
	addFactory(new BitmapLoaderFactory);
	addFactory(new DdsLoaderFactory);
	addFactory(new EffectLoaderFactory(*this));
	addFactory(new JpegLoaderFactory);
	addFactory(new Max3dsLoaderFactory(*this));
	addFactory(new PixelShaderLoaderFactory);
	addFactory(new PngLoaderFactory);
	addFactory(new SkeletonLoaderFactory);
	addFactory(new TgaLoaderFactory);
	addFactory(new VertexShaderLoaderFactory);

	// CubemapLoader must be added last (have a higher priority than other image loader factory)
	addFactory(new CubemapLoaderFactory);
}
