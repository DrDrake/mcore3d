#include "Pch.h"
#include "FileSystemCollection.h"
#include "Log.h"
#include <algorithm>
#include <list>
#include <set>

namespace MCD {

namespace {

struct SearchContext
{
	std::set<Path> uniquePaths;
	std::set<Path>::const_iterator iterator;
};	// SearchFileContext

}	// namespace

class FileSystemCollection::Impl
{
public:
	~Impl()
	{
		for(FileSystems::iterator i=mFileSystems.begin(); i!=mFileSystems.end(); ++i)
			delete (*i);
	}

	void addFileSystem(IFileSystem& fileSystem)
	{
		Impl::FileSystems::iterator i = std::find(mFileSystems.begin(), mFileSystems.end(), &fileSystem);
		if(i != mFileSystems.end())
			mFileSystems.erase(i);
		mFileSystems.push_back(&fileSystem);
	}

	bool removeFileSystem(const Path& fileSystemRootPath)
	{
		// TODO: Should we consider the full path when preforming the search?
		for(FileSystems::iterator i=mFileSystems.begin(); i!=mFileSystems.end(); ) {
			IFileSystem* fileSystem = (*i);
			MCD_ASSUME(fileSystem != nullptr);
			if(fileSystem->getRoot() == fileSystemRootPath)
				i = mFileSystems.erase(i);
			else
				++i;
		}

		return false;
	}

	IFileSystem* getNextFileSystem(IFileSystem* previousFs)
	{
		FileSystems::const_iterator ret = mFileSystems.end();
		for(FileSystems::const_iterator i=mFileSystems.begin(); i!=mFileSystems.end(); ) {
			if((*i) == previousFs)
				ret = ++i;
			else
				++i;
		}
		return ret == mFileSystems.end() ? nullptr : *ret;
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

	void* openFirstChildFolder(const Path& folder) const
	{
		SearchContext* context = new SearchContext;

		for(FileSystems::const_iterator i=mFileSystems.begin(); i!=mFileSystems.end(); ++i)
		{
			IFileSystem& fs = *(*i);

			void* c = fs.openFirstChildFolder(folder);
			Path p;
			while(true) {
				p = fs.getNextSiblingFolder(c);
				if(p.getString().empty())
					break;
				context->uniquePaths.insert(p);
			}
			fs.closeFirstChildFolder(c);
		}

		context->iterator = context->uniquePaths.begin();
		return context;
	}

	void* openFirstFileInFolder(const Path& folder) const
	{
		SearchContext* context = new SearchContext;

		for(FileSystems::const_iterator i=mFileSystems.begin(); i!=mFileSystems.end(); ++i)
		{
			IFileSystem& fs = *(*i);

			void* c = fs.openFirstFileInFolder(folder);
			Path p;
			while(true) {
				p = fs.getNextFileInFolder(c);
				if(p.getString().empty())
					break;
				context->uniquePaths.insert(p);
			}
			fs.closeFirstFileInFolder(c);
		}

		context->iterator = context->uniquePaths.begin();
		return context;
	}

	//! This function is shared by both getNextSiblingFolder() and getNextFileInFolder()
	Path getNextSearchPath(void* context) const
	{
		SearchContext* c = reinterpret_cast<SearchContext*>(context);
		Path ret;

		if(c->iterator != c->uniquePaths.end()) {
			ret = *c->iterator;
			++(c->iterator);
		}

		return ret;
	}

	typedef std::list<IFileSystem*> FileSystems;
	FileSystems mFileSystems;
};	// Impl

FileSystemCollection::FileSystemCollection()
	: mImpl(*new Impl)
{
}

FileSystemCollection::~FileSystemCollection()
{
	delete &mImpl;
}

void FileSystemCollection::addFileSystem(IFileSystem& fileSystem)
{
	mImpl.addFileSystem(fileSystem);
}

bool FileSystemCollection::removeFileSystem(const Path& fileSystemRootPath)
{
	return mImpl.removeFileSystem(fileSystemRootPath);
}

IFileSystem* FileSystemCollection::getNextFileSystem(IFileSystem* previousFs)
{
	return mImpl.getNextFileSystem(previousFs);
}

IFileSystem* FileSystemCollection::findFileSystemForPath(const Path& path) const
{
	return mImpl.findFileSystemForPath(path);
}

Path FileSystemCollection::getRoot() const {
	if(mImpl.mFileSystems.empty())
		return Path();
	else
		return mImpl.mFileSystems.front()->getRoot();
}

bool FileSystemCollection::setRoot(const Path&) {
	return false;
}

bool FileSystemCollection::isExists(const Path& path) const
{
	return mImpl.findFileSystemForPath(path) != nullptr;
}

bool FileSystemCollection::isDirectory(const Path& path) const
{
	IFileSystem* fileSystem = mImpl.findFileSystemForPath(path);
	if(!fileSystem) {
		Log::format(Log::Warn, "The path %s does not exist. At %s line %i", path.c_str(), __FILE__, __LINE__);
		return false;
	}

	return fileSystem->isDirectory(path);
}

uint64_t FileSystemCollection::getSize(const Path& path) const
{
	IFileSystem* fileSystem = mImpl.findFileSystemForPath(path);
	if(!fileSystem) {
		Log::format(Log::Warn, "The path %s does not exist. At %s line %i", path.c_str(), __FILE__, __LINE__);
		return false;
	}

	return fileSystem->getSize(path);
}

std::time_t FileSystemCollection::getLastWriteTime(const Path& path) const
{
	IFileSystem* fileSystem = mImpl.findFileSystemForPath(path);
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
	IFileSystem* fileSystem = mImpl.findFileSystemForPath(path);
	if(!fileSystem)
		return false;

	// TODO: Search for more file system until one of the remove() return true
	return fileSystem->remove(path);
}

std::auto_ptr<std::istream> FileSystemCollection::openRead(const Path& path) const
{
	IFileSystem* fileSystem = mImpl.findFileSystemForPath(path);
	if(!fileSystem)
		return std::auto_ptr<std::istream>();

	return fileSystem->openRead(path);
}

std::auto_ptr<std::ostream> FileSystemCollection::openWrite(const Path& path) const
{
	if(IFileSystem* fileSystem = mImpl.findFileSystemForPath(path))
		return fileSystem->openWrite(path);

	std::auto_ptr<std::ostream> ret;
	for(Impl::FileSystems::iterator i=mImpl.mFileSystems.begin(); i!=mImpl.mFileSystems.end(); ++i) {
		ret = (*i)->openWrite(path);
		if(ret.get())
			return ret;
	}

	return std::auto_ptr<std::ostream>();
}

void* FileSystemCollection::openFirstChildFolder(const Path& folder) const
{
	return mImpl.openFirstChildFolder(folder);
}

Path FileSystemCollection::getNextSiblingFolder(void* context) const
{
	return mImpl.getNextSearchPath(context);
}

void FileSystemCollection::closeFirstChildFolder(void* context) const
{
	SearchContext* c = reinterpret_cast<SearchContext*>(context);
	delete c;
}

void* FileSystemCollection::openFirstFileInFolder(const Path& folder) const
{
	return mImpl.openFirstFileInFolder(folder);
}

Path FileSystemCollection::getNextFileInFolder(void* context) const
{
	return mImpl.getNextSearchPath(context);
}

void FileSystemCollection::closeFirstFileInFolder(void* context) const
{
	closeFirstChildFolder(context);
}

}	// namespace MCD
