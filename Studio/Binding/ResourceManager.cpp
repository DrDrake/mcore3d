#include "stdafx.h"
#include "ResourceManager.h"
#include "FileSystemCollection.h"
#include "../../MCD/Binding/Launcher.h"
#include "../../MCD/Core/System/Resource.h"	// for the definition of ~Resource()
#undef nullptr
#include <gcroot.h>

using namespace MCD;

namespace Binding {

class ResourceManagerImpl
{
public:
	ResourceManagerImpl(FileSystemCollection^ fsc)
		: fsCollection(fsc), mgr(*fsc->getRawPtr(), false)
	{
	}

	void pollForUpdatedFiles()
	{
		Binding::RawFileMonitors& monitors = fsCollection->monitors();
		for(size_t i=0; i<monitors.size(); ++i)
		{	// Reload any changed files in the RawFileSystem
			std::wstring path;
			while(!(path = monitors[i].getChangedFile()).empty())
				mgr.reload(Path(path).normalize(), IResourceManager::NonBlock);
		}
	}

	gcroot<FileSystemCollection^> fsCollection;
	LauncherDefaultResourceManager mgr;
};	// ResourceManagerImpl

ResourceManager::ResourceManager(FileSystemCollection^ fsc)
	: mImpl(new ResourceManagerImpl(fsc))
{
}

ResourceManager::~ResourceManager()
{
	this->!ResourceManager();
}

ResourceManager::!ResourceManager()
{
	destroy();
}

void ResourceManager::destroy()
{
	delete mImpl;
	mImpl = nullptr;
}

void ResourceManager::pollForUpdatedFiles()
{
	if(mImpl)
		mImpl->pollForUpdatedFiles();
}

MCD::IResourceManager* ResourceManager::getRawPtr()
{
	return mImpl ? &mImpl->mgr : nullptr;
}

FileSystemCollection^ ResourceManager::fileSystemCollection::get()
{
	return mImpl->fsCollection;
}

}	// namespace Binding
