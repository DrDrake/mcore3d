#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/ResourceLoaderFactory.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/Resource.h"
#include "../../MCD/Core/System/ResourceLoader.h"

using namespace MCD;

DefaultResourceManager::DefaultResourceManager(const Path& rootPath)
	: ResourceManager(*(new RawFileSystem(rootPath)))
{
	setupFactories();
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
	addFactory(new JpegLoaderFactory);
	addFactory(new Max3dsLoaderFactory(*this));
	addFactory(new PixelShaderLoaderFactory);
	addFactory(new PngLoaderFactory);
	addFactory(new TgaLoaderFactory);
	addFactory(new VertexShaderLoaderFactory);
}
