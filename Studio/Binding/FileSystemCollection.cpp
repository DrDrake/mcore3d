#include "stdafx.h"
#include "FileSystemCollection.h"
#include "Utility.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/StrUtility.h"
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
		mMonitors->push_back(new RawFileSystemMonitor(fs->getRoot().getString().c_str(), true));
		mFileSystems->Add(pathToFileSystem);
	}
}

bool FileSystemCollection::removeFileSystem(String^ pathToFileSystem)
{
	std::wstring ws = Utility::toWString(pathToFileSystem);
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
	std::auto_ptr<std::istream> is = mImpl->openRead(Utility::toWString(path));

	if(!is.get())
		return L"";

	std::ostringstream buffer;
	buffer << is->rdbuf();

	return gcnew String(buffer.str().c_str());
}

bool FileSystemCollection::saveString(System::String^ path, System::String^ str)
{
	std::auto_ptr<std::ostream> os = mImpl->openWrite(Utility::toWString(path));
	if(!os.get())
		return false;

	std::wstring ws = Utility::toWString(str);
	std::string utf8;
	if(!wStrToUtf8(ws, utf8))
		return false;

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
