#include "stdafx.h"
#include "ResourceManager.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Render/ResourceLoaderFactory.h"
#include "../../MCD/Component/Render/EntityPrototypeLoader.h"

using namespace MCD;

namespace Binding {

class DefaultResourceManager : public MCD::ResourceManager
{
public:
	explicit DefaultResourceManager(MCD::IFileSystem& fileSystem)
		: ResourceManager(fileSystem)
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
		addFactory(new Max3dsLoaderFactory(*this));
		addFactory(new EntityPrototypeLoaderFactory(*this));

		// CubemapLoader must be added last (have a higher priority than other image loader factory)
		addFactory(new CubemapLoaderFactory);
	}

	/*!
		\return
			>0 - A resource is sucessfully loaded (partial or full).
			=0 - There are no more event to process at this moment.
			<0 - The resource failed to load.
	 */
	int processLoadingEvents()
	{
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

protected:
	void setupFactories();

	class Impl;
	Impl* mImpl;
};	// DefaultResourceManager

ResourceManager::ResourceManager()
{
//	mImpl = new DefaultResourceManager;
}

ResourceManager::~ResourceManager()
{
	this->!ResourceManager();
}

ResourceManager::!ResourceManager()
{
	delete mImpl;
}

}	// namespace Binding
