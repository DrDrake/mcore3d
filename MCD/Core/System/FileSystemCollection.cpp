#include "Pch.h"
#include "FileSystemCollection.h"
#include "StrUtility.h"
#include <algorithm>
#include <list>
#include <stdexcept>

namespace MCD {

static void throwError(const std::string& prefix, const std::wstring& pathStr, const std::string& sufix)
{
	throw std::runtime_error(
		prefix + "\"" + wStrToStr(pathStr) + "\"" + sufix
	);
}

class FileSystemCollection::Impl
{
public:
	~Impl()
	{
		for(FileSystems::iterator i=mFileSystems.begin(); i!=mFileSystems.end(); ++i)
			delete (*i);
	}

	IFileSystem* findFileSystemForPath(const Path& path) const
	{
		for(FileSystems::const_iterator i=mFileSystems.begin(); i!=mFileSystems.end(); ++i) {
			IFileSystem* fileSystem = (*i);
			MCD_ASSUME(fileSystem != nullptr);
			if(fileSystem->isExists(path))
				return fileSystem;
		}
		return nullptr;
	}

	typedef std::list<IFileSystem*> FileSystems;
	FileSystems mFileSystems;
};

FileSystemCollection::FileSystemCollection()
{
	mImpl = new Impl;
}

FileSystemCollection::~FileSystemCollection()
{
	delete mImpl;
}

void FileSystemCollection::addFileSystem(IFileSystem& fileSystem)
{
	MCD_ASSUME(mImpl);
	Impl::FileSystems& fileSystems = mImpl->mFileSystems;
	Impl::FileSystems::iterator i = std::find(fileSystems.begin(), fileSystems.end(), &fileSystem);
	if(i != fileSystems.end())
		fileSystems.erase(i);
	fileSystems.push_back(&fileSystem);
}

IFileSystem* FileSystemCollection::findFileSystemForPath(const Path& path) const
{
	MCD_ASSUME(mImpl);
	return mImpl->findFileSystemForPath(path);
}

Path FileSystemCollection::getRoot() const {
	MCD_ASSUME(mImpl);
	if(mImpl->mFileSystems.empty())
		return Path();
	else
		return mImpl->mFileSystems.front()->getRoot();
}

bool FileSystemCollection::setRoot(const Path&) {
	return false;
}

bool FileSystemCollection::isExists(const Path& path) const
{
	MCD_ASSUME(mImpl);
	return mImpl->findFileSystemForPath(path) != nullptr;
}

bool FileSystemCollection::isDirectory(const Path& path) const
{
	MCD_ASSUME(mImpl);
	IFileSystem* fileSystem = mImpl->findFileSystemForPath(path);
	if(!fileSystem)
		throwError("Directory ", path.getString(), " does not exist");

	return fileSystem->isDirectory(path);
}

uint64_t FileSystemCollection::getSize(const Path& path) const
{
	MCD_ASSUME(mImpl);
	IFileSystem* fileSystem = mImpl->findFileSystemForPath(path);
	if(!fileSystem)
		throwError("File ", path.getString(), " does not exist");

	return fileSystem->getSize(path);
}

std::time_t FileSystemCollection::getLastWriteTime(const Path& path) const
{
	MCD_ASSUME(mImpl);
	IFileSystem* fileSystem = mImpl->findFileSystemForPath(path);
	if(!fileSystem)
		return 0;

	return fileSystem->getLastWriteTime(path);
}

bool FileSystemCollection::makeDir(const Path& path) const
{
	// TODO: Implement
	return false;
}

bool FileSystemCollection::remove(const Path& path) const
{
	MCD_ASSUME(mImpl);
	IFileSystem* fileSystem = mImpl->findFileSystemForPath(path);
	if(!fileSystem)
		return false;

	// TODO: Search for more file system until one of the remove() return true
	return fileSystem->remove(path);
}

std::auto_ptr<std::istream> FileSystemCollection::openRead(const Path& path) const
{
	MCD_ASSUME(mImpl);
	IFileSystem* fileSystem = mImpl->findFileSystemForPath(path);
	if(!fileSystem)
		return std::auto_ptr<std::istream>();

	return fileSystem->openRead(path);
}

std::auto_ptr<std::ostream> FileSystemCollection::openWrite(const Path& path) const
{
	// TODO: Implement
	MCD_ASSERT(false && "Not implement yet");
	return std::auto_ptr<std::ostream>();
}

}	// namespace MCD
