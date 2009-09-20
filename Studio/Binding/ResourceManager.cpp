#include "stdafx.h"
#include "ResourceManager.h"
#include "FileSystemCollection.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Binding/Launcher.h"

using namespace MCD;

namespace Binding {

ResourceManager::ResourceManager(FileSystemCollection^ fileSystemCollection)
	: mFileSystemCollection(fileSystemCollection)
{
	mImpl = new LauncherDefaultResourceManager(*fileSystemCollection->getRawPtr(), false);
}

ResourceManager::~ResourceManager()
{
	this->!ResourceManager();
}

ResourceManager::!ResourceManager()
{
	destroy();
}

MCD::IResourceManager* ResourceManager::getRawPtr()
{
	return mImpl;
}

void ResourceManager::destroy()
{
	delete mImpl;
	mImpl = nullptr;
}

FileSystemCollection^ ResourceManager::fileSystemCollection::get()
{
	return mFileSystemCollection;
}

}	// namespace Binding
