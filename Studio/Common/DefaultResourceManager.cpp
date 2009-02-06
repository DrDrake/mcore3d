#include "stdafx.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/ResourceLoaderFactory.h"
#include "../../MCD/Core/System/FileSystemCollection.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/RawFileSystemMonitor.h"
#include "../../MCD/Core/System/Resource.h"
#include "../../MCD/Core/System/ResourceLoader.h"
#include "../../MCD/Core/System/ZipFileSystem.h"

using namespace MCD;

IFileSystem* createDefaultFileSystem()
{
	std::auto_ptr<FileSystemCollection> fileSystem(new FileSystemCollection);

	try {
		std::auto_ptr<IFileSystem> rawFs(new RawFileSystem(L"Media"));
		fileSystem->addFileSystem(*rawFs.release());
	} catch(...) {}

	try {
		std::auto_ptr<IFileSystem> zipFs(new ZipFileSystem(L"Media.zip"));
		fileSystem->addFileSystem(*zipFs.release());
	} catch(...) {}

	return fileSystem.release();
}

class DefaultResourceManager::Impl
{
public:
	Impl(const wchar_t* monitorDirectory)
		: mMonitor(monitorDirectory, true)
	{
	}

	MCD::RawFileSystemMonitor mMonitor;
};	// Impl

DefaultResourceManager::DefaultResourceManager(IFileSystem& fileSystem)
	: ResourceManager(fileSystem)
{
	mImpl = new Impl(L"Media");
	setupFactories();
}

DefaultResourceManager::~DefaultResourceManager()
{
	delete mImpl;
}

int DefaultResourceManager::processLoadingEvents()
{
	MCD_ASSUME(mImpl);

	{	// Reload any changed files in the RawFileSystem
		std::wstring path = mImpl->mMonitor.getChangedFile();
		while(!path.empty()) {
			reload(Path(path).normalize(), false);
			path = mImpl->mMonitor.getChangedFile();
		}
	}

	ResourceManager::Event e = popEvent();
	if(e.loader) {
		bool hasError = e.loader->getLoadingState() == IResourceLoader::Aborted;

		if(hasError)
			Log::format(Log::Warn, L"Resource: %s %s", e.resource->fileId().getString().c_str(), L"failed to load");
		else	// Allow at most one resource to commit at each time
			e.loader->commit(*e.resource);

		// Note that commit() is invoked before doCallbacks()
		doCallbacks(e);

		return hasError ? -1 : 1;
	}

	return 0;
}

void DefaultResourceManager::setupFactories()
{
	addFactory(new BitmapLoaderFactory);
	addFactory(new DdsLoaderFactory);
	addFactory(new EffectLoaderFactory(*this));
	addFactory(new JpegLoaderFactory);
	addFactory(new Max3dsLoaderFactory(*this));
	addFactory(new PodLoaderFactory(*this));
	addFactory(new PixelShaderLoaderFactory);
	addFactory(new PngLoaderFactory);
	addFactory(new TgaLoaderFactory);
	addFactory(new VertexShaderLoaderFactory);
}
