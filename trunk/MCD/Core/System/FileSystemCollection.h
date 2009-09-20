#ifndef __MCD_CORE_SYSTEM_FILESYSTEMCOLLECTION__
#define __MCD_CORE_SYSTEM_FILESYSTEMCOLLECTION__

#include "FileSystem.h"

namespace MCD {

/*!	A collection of file systems that present to the user as a single one.
	You can put numbers of other IFileSystem into FileSystemCollection for instance,
	ZipFileSystem and RawFileSystem; then when you try to access a file / directory
	it will try to search in the supplied ZipFileSystem first, if it's not found,
	it will try to search the next IFileSystem (ie. the supplied RawFileSystem) and
	so on.
 */
class MCD_CORE_API FileSystemCollection : public IFileSystem
{
public:
	FileSystemCollection();

	sal_override ~FileSystemCollection();

	/*!	Add a file system, and give it's ownership to FileSystemCollection.
		\note The file search order depends on the order you add the file systems.
		\note Adding the same file system twice will make that file system place at
			the end of the ordering list.
	 */
	void addFileSystem(IFileSystem& fileSystem);

	bool removeFileSystem(const Path& fileSystemRootPath);

	//! Find the FIRST file system which contains the supplied path, returns null if not found.
	IFileSystem* findFileSystemForPath(const Path& path) const;

	//! It only returns the first file system's root path.
	sal_override Path getRoot() const;

	//! Inapplicable, simple return false.
	sal_override bool setRoot(const Path& rootPath);

	sal_override bool isExists(const Path& path) const;

	sal_override bool isDirectory(const Path& path) const;

	sal_override uint64_t getSize(const Path& path) const;

	sal_override std::time_t getLastWriteTime(const Path& path) const;

	sal_override bool makeDir(const Path& path) const;

	//! How should it behave? remove the first or all? For simplicity, I choose to just remove the first.
	sal_override bool remove(const Path& path) const;

	sal_override std::auto_ptr<std::istream> openRead(const Path& path) const;

	sal_override std::auto_ptr<std::ostream> openWrite(const Path& path) const;

private:
	class Impl;
	Impl* mImpl;
};	// FileSystemCollection

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_FILESYSTEMCOLLECTION__
