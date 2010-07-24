#include "stdafx.h"
#include "FileSystemCollection.h"
#include "Utility.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/StrUtility.h"
#include "../../MCD/Core/System/ZipFileSystem.h"
#include <sstream>

using namespace MCD;
using namespace System;

namespace Binding {

FileSystemCollection::FileSystemCollection()
{
	mImpl = new MCD::FileSystemCollection();
	mMonitors = new RawFileMonitors;
	mFileSystems = gcnew StringCollection();
}

FileSystemCollection::~FileSystemCollection()
{
	this->!FileSystemCollection();
}

FileSystemCollection::!FileSystemCollection()
{
	delete mImpl;
	delete mMonitors;
	mImpl = nullptr;
	mMonitors = nullptr;
}

RawFileMonitors& FileSystemCollection::monitors()
{
	return *mMonitors;
}

String^ FileSystemCollection::getRoot()
{
	return Utility::fromUtf8(mImpl->getRoot().getString());
}

bool FileSystemCollection::setRoot(String^ rootPath)
{
	return mImpl->setRoot(Utility::toUtf8String(rootPath));
}

bool FileSystemCollection::isExists(String^ path)
{
	return mImpl->isExists(Utility::toUtf8String(path));
}

bool FileSystemCollection::isDirectory(String^ path)
{
	return mImpl->isDirectory(Utility::toUtf8String(path));
}

uint64_t FileSystemCollection::getSize(String^ path)
{
	return mImpl->getSize(Utility::toUtf8String(path));
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
		fs = new MCD::RawFileSystem(Utility::toUtf8String(pathToFileSystem));
	else if(IO::File::Exists(pathToFileSystem) && IO::Path::GetExtension(pathToFileSystem)->ToLowerInvariant() == ".zip")
		fs = new MCD::ZipFileSystem(Utility::toUtf8String(pathToFileSystem));

	if(fs) {
		mImpl->addFileSystem(*fs);
		mMonitors->push_back(new RawFileSystemMonitor(fs->getRoot().getString().c_str(), true));
		mFileSystems->Add(pathToFileSystem);
	}
}

bool FileSystemCollection::removeFileSystem(String^ pathToFileSystem)
{
	std::string ws = Utility::toUtf8String(pathToFileSystem);
	IFileSystem* f = mImpl->findFileSystemForPath(ws);
	if(!f) return false;

	for(RawFileMonitors::iterator i=mMonitors->begin(); i!=mMonitors->end(); ++i)
		if(i->monitringPath() == ws) {	// TODO: Is this comparison reliable?
			mMonitors->erase(i);
			break;
		}

	mFileSystems->Remove(pathToFileSystem);
	return mImpl->removeFileSystem(ws);
}

String^ FileSystemCollection::openAsString(String^ path)
{
	std::auto_ptr<std::istream> is = mImpl->openRead(Utility::toUtf8String(path));

	if(!is.get())
		return "";

	std::ostringstream buffer;
	buffer << is->rdbuf();

	return gcnew String(buffer.str().c_str());
}

bool FileSystemCollection::saveString(System::String^ path, System::String^ str)
{
	std::auto_ptr<std::ostream> os = mImpl->openWrite(Utility::toUtf8String(path));
	if(!os.get())
		return false;

	std::string utf8 = Utility::toUtf8String(str);

	if(!utf8.empty())
		os->write(&utf8[0], utf8.size());
	return true;
}

FileSystemCollection::StringCollection^ FileSystemCollection::fileSystems::get()
{
	return mFileSystems;
}

FileSystemCollection::StringCollection^ FileSystemCollection::getDirectories(String^ path)
{
	StringCollection^ sc = gcnew StringCollection();

	{	void* c = mImpl->openFirstChildFolder(Utility::toUtf8String(path));
		Path p;
		while(true)
		{
			Path p(mImpl->getNextSiblingFolder(c));
			if(p.getString().empty())
				break;
			sc->Add(Utility::fromUtf8(p.getString()));
		}
		mImpl->closeFirstChildFolder(c);
	}

	return sc;
}

FileSystemCollection::StringCollection^ FileSystemCollection::getFiles(String^ path)
{
	StringCollection^ sc = gcnew StringCollection();

	{	void* c = mImpl->openFirstFileInFolder(Utility::toUtf8String(path));
		Path p;
		while(true)
		{
			Path p(mImpl->getNextFileInFolder(c));
			if(p.getString().empty())
				break;
			sc->Add(Utility::fromUtf8(p.getString()));
		}
		mImpl->closeFirstFileInFolder(c);
	}

	return sc;
}

}	// namespace Binding
