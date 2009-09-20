#include "stdafx.h"
#include "FileSystemCollection.h"
#include "Utility.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Core/System/FileSystemCollection.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#undef nullptr
#include <gcroot.h>

using namespace MCD;
using namespace System;

namespace Binding {


FileSystemCollection::FileSystemCollection()
{
	mImpl = new MCD::FileSystemCollection();
	mFileSystems = gcnew System::Collections::Generic::List<System::String^>();
}

FileSystemCollection::~FileSystemCollection()
{
	this->!FileSystemCollection();
}

FileSystemCollection::!FileSystemCollection()
{
	delete mImpl;
	mImpl = nullptr;
}

String^ FileSystemCollection::getRoot()
{
	return gcnew String(mImpl->getRoot().getString().c_str());
}

bool FileSystemCollection::setRoot(String^ rootPath)
{
	return mImpl->setRoot(Utility::toWString(rootPath));
}

bool FileSystemCollection::isExists(String^ path)
{
	return mImpl->isExists(Utility::toWString(path));
}

bool FileSystemCollection::isDirectory(String^ path)
{
	return mImpl->isDirectory(Utility::toWString(path));
}

uint64_t FileSystemCollection::getSize(String^ path)
{
	return mImpl->getSize(Utility::toWString(path));
}

MCD::FileSystemCollection* FileSystemCollection::getRawPtr()
{
	return mImpl;
}

void FileSystemCollection::addFileSystem(String^ pathToFileSystem)
{
	MCD::IFileSystem* fs = nullptr;

	// Check the corresponding file system according to the input: pathToFileSystem
	if(IO::Directory::Exists(pathToFileSystem))
		fs = new MCD::RawFileSystem(Utility::toWString(pathToFileSystem));

	if(fs) {
		mImpl->addFileSystem(*fs);
		mFileSystems->Add(pathToFileSystem);
	}
}

bool FileSystemCollection::removeFileSystem(String^ pathToFileSystem)
{
	mFileSystems->Remove(pathToFileSystem);
	return mImpl->removeFileSystem(Utility::toWString(pathToFileSystem));
}

FileSystemCollection::FileSystems^ FileSystemCollection::fileSystems::get()
{
	return mFileSystems;
}

}	// namespace Binding
