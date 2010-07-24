#ifndef __MCD_CORE_SYSTEM_MEMORYFILESYSTEM__
#define __MCD_CORE_SYSTEM_MEMORYFILESYSTEM__

#include "FileSystem.h"
#include "Map.h"

namespace MCD {

///	A file system for in memory data to use as regular file.
///	There are numbers of restrictions on this file system,
///	see the documentation of each individual functions in this class.
class MCD_CORE_API MemoryFileSystem : public IFileSystem
{
public:
	explicit MemoryFileSystem(const Path& root);

	sal_override ~MemoryFileSystem();

	sal_override Path getRoot() const {
		return mRoot;
	}

	sal_override bool setRoot(const Path& rootPath) {
		return false;
	}

	sal_override bool isExists(const Path& path) const;

	sal_override bool isDirectory(const Path& path) const {
		return false;
	}

	sal_override uint64_t getSize(const Path& path) const;

	/// NOT supported in this file system.
	sal_override std::time_t getLastWriteTime(const Path& path) const {
		return (std::time_t)0;
	}

	/// NOT supported in this file system.
	sal_override bool makeDir(const Path& path) const {
		return false;
	}

	/// NOT supported in this file system.
	sal_override bool remove(const Path& path) const {
		return false;
	}

	///	Returns a std::istream for reading, null if fail.
	sal_override std::auto_ptr<std::istream> openRead(const Path& path) const;

	///	Returns a std::ostream for writing, null if fail.
	///	\note NOT supported in this file system.
	sal_override std::auto_ptr<std::ostream> openWrite(const Path& path) const {
		return std::auto_ptr<std::ostream>(); 
	}

	/// Adds a block of memory to this file system.
	///	This will NOT take ownership of fileData.
	///	Returns false if this fileId already exists.
	sal_checkreturn bool add(const Path& fileId, sal_in_bcount(fileSize) const void* fileData, size_t fileSize);

private:
	Path mRoot;

	struct MemoryFile : MapBase<Path>::Node<MemoryFile>
	{
		MemoryFile(const Path& key, const void* ptr, size_t sz);
		const void* fileData;
		size_t fileSize;
	};	// MemoryFile

	Map<MemoryFile> mMemoryFiles;
};	// MemoryFileSystem

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_MEMORYFILESYSTEM__
