#ifndef __MCD_CORE_SYSTEM_RAWFILESYSTEM__
#define __MCD_CORE_SYSTEM_RAWFILESYSTEM__

#include "FileSystem.h"

namespace MCD {

/*!	A raw file system that open files from OS supported drive.
	Operations are suppose to work with relative path but it's alright
	to use absolute path also.
 */
class MCD_CORE_API RawFileSystem : public IFileSystem
{
public:
	/*!	Construct the raw file system with the supplied rootPath
		\sa SetRoot
	 */
	explicit RawFileSystem(const Path& rootPath);

	sal_override Path getRoot() const;

	/*!	Set the root.
		\param rootPath If it's a relative or an empty path,
			Path::getCurrentPath() will be used in front of it.
	 */
	sal_override bool setRoot(const Path& rootPath);

	sal_override bool isExists(const Path& path) const;

	sal_override bool isDirectory(const Path& path) const;

	sal_override uint64_t getSize(const Path& path) const;

	sal_override std::time_t getLastWriteTime(const Path& path) const;

	sal_override bool makeDir(const Path& path) const;

	sal_override bool remove(const Path& path) const;

	sal_override std::auto_ptr<std::istream> openRead(const Path& path) const;

	sal_override std::auto_ptr<std::ostream> openWrite(const Path& path) const;

private:
	//! Convert to absolute path, if necessary
	Path toAbsolutePath(const Path& path) const;

	Path mRootPath;
};	// RawFileSystem

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RAWFILESYSTEM__
