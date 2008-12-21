#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/ResourceLoaderFactory.h"
#include "../../MCD/Core/System/FileSystemCollection.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Core/System/RawFileSystem.h"
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

DefaultResourceManager::DefaultResourceManager(IFileSystem& fileSystem)
	: ResourceManager(fileSystem)
{
	setupFactories();
}

int DefaultResourceManager::processLoadingEvents()
{
	ResourceManager::Event e = popEvent();
	if(e.loader) {
		if(e.loader->getLoadingState() == IResourceLoader::Aborted) {
			Log::format(Log::Warn, L"Resource: %s %s", e.resource->fileId().getString().c_str(), L"failed to load");
			return -1;
		}

		// Allow at most one resource to commit at each time
		e.loader->commit(*e.resource);

		return 1;
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
	addFactory(new PixelShaderLoaderFactory);
	addFactory(new PngLoaderFactory);
	addFactory(new TgaLoaderFactory);
	addFactory(new VertexShaderLoaderFactory);
}
