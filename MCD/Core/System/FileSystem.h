#ifndef __MCD_CORE_SYSTEM_FILESYSTEM__
#define __MCD_CORE_SYSTEM_FILESYSTEM__

#include "Path.h"
#include <memory>	// For std::auto_ptr
#include <ctime>	// For std::time_t
#include <iosfwd>	// For declaration of istream and ostream

namespace MCD {

/*!	An abstract file system.
	This class is supposed to be override to make useful file system such as
	RawFileSystem, ZipFileSystem and WebFileSystem etc.

	The file system 'may' associate a root, for example "c:\" for a raw file
	system and "http://mysite.com/" for a web file system. Files are then accessed
	as relative path from the root.

	For simplification, only binary open mode is supported. To convert the binary
	stream to a text stream, you may use TODO.

	Since a stream is not copyable, an auto pointer is returned from OpenRead and
	OpenWrite. The callee of OpenRead/OpenWrite is then the owner of the stream.

	\sa There is a library that act similar to our IFileSystem: http://icculus.org/physfs/
 */
class MCD_ABSTRACT_CLASS IFileSystem
{
public:
	virtual ~IFileSystem() {}

	//! Returns the absolute root path of this file system.
	virtual Path getRoot() const = 0;

	virtual bool setRoot(const Path& rootPath) = 0;

	virtual bool isExists(const Path& path) const = 0;

	virtual bool isDirectory(const Path& path) const = 0;

	virtual uint64_t getSize(const Path& path) const = 0;

	virtual std::time_t getLastWriteTime(const Path& path) const = 0;

	//! Intermediate directories will be created as well.
	virtual bool makeDir(const Path& path) const = 0;

	//! Remove a single file or a directory recursively.
	virtual bool remove(const Path& path) const = 0;

	//!	Returns a std::istream for reading, null if fail.
	virtual std::auto_ptr<std::istream> openRead(const Path& path) const = 0;

	//!	Returns a std::ostream for writing, null if fail.
	virtual std::auto_ptr<std::ostream> openWrite(const Path& path) const = 0;

	virtual sal_maybenull void* openFirstChildFolder(const Path& folder) const { return nullptr; }

	virtual Path getNextSiblingFolder(sal_maybenull void* context) const { return Path(); }

	virtual void closeFirstChildFolder(sal_maybenull void* context) const {}

	virtual sal_maybenull void* openFirstFileInFolder(const Path& folder) const { return nullptr; }

	virtual Path getNextFileInFolder(sal_maybenull void* context) const { return Path(); }

	virtual void closeFirstFileInFolder(sal_maybenull void* context) const {}
};	// IFileSystem

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_FILESYSTEM__
