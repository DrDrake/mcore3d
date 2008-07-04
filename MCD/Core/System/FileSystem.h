#ifndef __SGE_CORE_SYSTEM_FILESYSTEM__
#define __SGE_CORE_SYSTEM_FILESYSTEM__

#include "Path.h"
#include <memory>	// For std::auto_ptr
#include <ctime>	// For std::time_t
#include <iosfwd>	// For declaration of istream and ostream

namespace SGE {

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
 */
class SGE_NOVTABLE IFileSystem
{
public:
	virtual ~IFileSystem() {}

	virtual Path getRoot() const = 0;

	virtual bool setRoot(const Path& rootPath) = 0;

	virtual bool isExists(const Path& path) const = 0;

	virtual bool isDirectory(const Path& path) const = 0;

	virtual uint64_t getSize(const Path& path) const = 0;

	virtual std::time_t getLastWriteTime(const Path& path) const = 0;

	//! Intermediate directories will be created as well.
	virtual void makeDir(const Path& path) const = 0;

	//! Remove a single file or a directory recursively.
	virtual void remove(const Path& path) const = 0;

	virtual std::auto_ptr<std::istream> openRead(const Path& path) const = 0;

	virtual std::auto_ptr<std::ostream> openWrite(const Path& path) const = 0;
};	// IFileSystem

}	// namespace SGE

#endif	// __SGE_CORE_SYSTEM_FILESYSTEM__
