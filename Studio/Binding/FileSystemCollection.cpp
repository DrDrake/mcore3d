#include "stdafx.h"
#include "FileSystemCollection.h"
#include "Utility.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Core/System/FileSystemCollection.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#undef nullptr
#include <gcroot.h>
#include <sstream>

using namespace MCD;
using namespace System;

namespace Binding {


FileSystemCollection::FileSystemCollection()
{
	mImpl = new MCD::FileSystemCollection();
	mFileSystems = gcnew StringCollection();
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

String^ FileSystemCollection::openAsString(String^ path)
{
	std::auto_ptr<std::istream> is = mImpl->openRead(Utility::toWString(path));

	if(!is.get())
		return L"";

	std::ostringstream buffer;
	buffer << is->rdbuf();

	return gcnew String(buffer.str().c_str());
}

FileSystemCollection::StringCollection^ FileSystemCollection::fileSystems::get()
{
	return mFileSystems;
}

FileSystemCollection::StringCollection^ FileSystemCollection::getDirectories(String^ path)
{
	StringCollection^ sc = gcnew StringCollection();

	{	void* c = mImpl->openFirstChildFolder(Utility::toWString(path));
		Path p;
		while(true)
		{
			Path p(mImpl->getNextSiblingFolder(c));
			if(p.getString().empty())
				break;
			sc->Add(gcnew String(p.getString().c_str()));
		}
		mImpl->closeFirstChildFolder(c);
	}

	return sc;
}

FileSystemCollection::StringCollection^ FileSystemCollection::getFiles(String^ path)
{
	StringCollection^ sc = gcnew StringCollection();

	{	void* c = mImpl->openFirstFileInFolder(Utility::toWString(path));
		Path p;
		while(true)
		{
			Path p(mImpl->getNextFileInFolder(c));
			if(p.getString().empty())
				break;
			sc->Add(gcnew String(p.getString().c_str()));
		}
		mImpl->closeFirstFileInFolder(c);
	}

	return sc;
}

}	// namespace Binding
