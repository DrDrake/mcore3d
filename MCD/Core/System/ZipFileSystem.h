#ifndef __MCD_CORE_SYSTEM_ZIPFILESYSTEM__
#define __MCD_CORE_SYSTEM_ZIPFILESYSTEM__

#include "FileSystem.h"
#include "IntrusivePtr.h"

namespace MCD {

/// A zip file system.
/// It only support reading but not writing.
///
/// \note Since we use an incremental approach to unzip the data, user will not
/// 	able to perform seek operation on the stream. If such feature is
/// 	required, we can add an option such that the whole data are
/// 	unzipped to a single memory buffer.
///
/// \note This class is multi-thread safe.
/// \note Internal data structure is shared among opened streams, so feel free
/// 	to destroy the ZipFileSystem or invoke setRoot() while you are still
/// 	using the opened streams.
/// \todo Support loading zip archive from istream other than *.zip file only.
/// \todo Support unicode file name.
class MCD_CORE_API ZipFileSystem : public IFileSystem
{
public:
	/// Construct the zip file system with the supplied path.
	/// \sa SetRoot
	explicit ZipFileSystem(const Path& zipFilePath);

	sal_override ~ZipFileSystem();

	sal_override Path getRoot() const;

	/// Set the root.
	/// \param rootPath If it's a relative or an empty path,
	/// 	Path::getCurrentPath() will be used in front of it.
	sal_override bool setRoot(const Path& zipFilePath);

	sal_override bool isExists(const Path& path) const;

	sal_override bool isDirectory(const Path& path) const;

	sal_override uint64_t getSize(const Path& path) const;

	sal_override std::time_t getLastWriteTime(const Path& path) const;

	/// Not supported.
	sal_override bool makeDir(const Path& path) const;

	/// Not supported.
	sal_override bool remove(const Path& path) const;

	/// Returns a std::istream for reading, null if fail.
	/// The stream will perform a full unzip operation on demand,
	/// so this openRead() function is non-blocking.
	sal_override std::auto_ptr<std::istream> openRead(const Path& path) const;

	/// Not supported.
	sal_override std::auto_ptr<std::ostream> openWrite(const Path& path) const;

private:
	/// Convert to absolute path, if necessary
	Path toAbsolutePath(const Path& path) const;

	struct Context;
	class ZipStreamProxy;
	class Impl;
	typedef IntrusivePtr<Impl> ImplPtr;
	ImplPtr mImpl;
};	// ZipFileSystem

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_ZIPFILESYSTEM__
