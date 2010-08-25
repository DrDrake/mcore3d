#include "Pch.h"
#include "ZipFileSystem.h"
#include "Log.h"
#include "Mutex.h"
#include "PtrVector.h"
#include "Stream.h"
#include "Thread.h"
#include <map>
#include <stdexcept>
#include "../../../3Party/minizip/unzip.h"

#ifdef MCD_VC
#	pragma comment(lib, "zlib")
#	pragma comment(lib, "minizip")
#endif	// MCD_VC

using namespace std;

/// NOTE: Extra complexity of this implementation comes from the fact that
/// a single minizip handle can only map to a single IO stream, and yet we
/// want minizip handle pooling for better efficience, plus the support
/// of multi-threading.

namespace MCD {

static void throwError(const std::string& prefix, const std::string& pathStr, const std::string& sufix)
{
	throw std::runtime_error(
		prefix + "\"" + pathStr + "\"" + sufix
	);
}

static bool isDirectory(int flag) {
	return (flag & 32) == 0;
}

typedef std::map<std::string, unz_file_pos_s> FileMap;

/// Every stream should have it's own context, while a context
/// can be re-used after a stream is finished.
struct ZipFileSystem::Context
{
	MCD_NOINLINE explicit Context(sal_in const char* zipFilePath)
	{
		mZipHandle = unzOpen(zipFilePath);
		mZipFilePath = zipFilePath;
	}

	~Context()
	{
		// unzClose will call unzCloseCurrentFile()
		if(mZipHandle)
			MCD_VERIFY(unzClose(mZipHandle) == UNZ_OK);
	}

	/// Make the zip item specified by path as the current
	bool locateFile(const Path& path, const FileMap& fileMap)
	{
		if(!mZipHandle)
			return false;

		Path normalizedPath = path;
		normalizedPath.normalize();

		if(fileMap.empty())
			return unzLocateFile(mZipHandle, normalizedPath.getString().c_str(), 1) == UNZ_OK;
		else {
			FileMap::const_iterator i = fileMap.find(normalizedPath.getString());
			if(i == fileMap.end())
				return false;
			return unzGoToFilePos(mZipHandle, const_cast<unz_file_pos*>(&(i->second))) == UNZ_OK;
		}
	}

	sal_checkreturn bool getFileInfo(const Path& path, const FileMap& fileMap, unz_file_info& ret)
	{
		memset(&ret, 0, sizeof(ret));
		if(!locateFile(path, fileMap))
			return false;
		if(unzGetCurrentFileInfo(mZipHandle, &ret, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK)
			return false;
		return true;
	}

	sal_maybenull unzFile mZipHandle;
	std::string mZipFilePath;
};	// Context

class ZipFileSystem::ZipStreamProxy : public StreamProxy
{
public:
	ZipStreamProxy(const ImplPtr& impl);

	sal_checkreturn bool openRead(const char* zipItem);

	~ZipStreamProxy();

protected:
	typedef ZipFileSystem::ImplPtr ImplPtr;

	size_t doUnzip();

	sal_override size_t read(char* data, size_t size);

	sal_notnull Context* mContext;

	/// Hold a reference to ZipFileSystem::Impl so that it won't destroyed too soon
	const ImplPtr mImpl;
};	// ZipStreamProxy

/// Shared among all zip context, such that it only get destroy after all context are deleted first.
class ZipFileSystem::Impl : public IntrusiveSharedObject<AtomicInteger>
{
public:
	Impl() : mQueryContext(nullptr) {}

	~Impl()
	{
		ScopeRecursiveLock lock(mMutex);
		if(mQueryContext)
			releaseContext(*mQueryContext);
	}

	/// Get a context from the pool, or creates a new one if needed.
	/// The context should pass to \em releaseContext() after use.
	Context& getContext()
	{
		MCD_ASSERT(mMutex.isLocked());
		while(!mContextPool.empty()) {
			Context& c = *mContextPool.begin();
			if(c.mZipFilePath == mZipFilePath) {
				mContextPool.erase(mContextPool.begin(), false);
				return c;
			}
			// If the setRoot() is invoked, remove those no longer valid context
			else
				mContextPool.erase(mContextPool.begin(), true);
		}

		return *new Context(mZipFilePath.c_str());
	}

	void releaseContext(Context& c)
	{
		// NOTE: If you hit this assert on iPhone, please check that you are
		// not hitting the limit of 256 simultaneous opened file descriptor
		MCD_ASSERT(c.mZipHandle);
		MCD_ASSERT(mMutex.isLocked());
		unzCloseCurrentFile(c.mZipHandle);
		mContextPool.push_back(&c);
	}

	sal_checkreturn bool setRoot(const Path& zipFilePath)
	{
		Path absolutePath = zipFilePath;
		if(!absolutePath.hasRootDirectory())
			absolutePath = Path::getCurrentPath() / absolutePath;

		ScopeRecursiveLock lock(mMutex);
		if(absolutePath != mZipFilePath)
		{
			std::string bk = mZipFilePath;
			mZipFilePath = absolutePath.getString();
			Context& c = getContext();
			if(!c.mZipHandle) {
				mZipFilePath = bk;
				delete &c;
				return false;
			}

			if(mQueryContext)
				releaseContext(*mQueryContext);
			mQueryContext = &c;
			initFileMap();
		}

		return true;
	}

	void initFileMap()
	{
		MCD_ASSUME(mQueryContext);
		unzFile z = mQueryContext->mZipHandle;
		MCD_ASSUME(z);
		mFileMap.clear();

		// Pre-allocate a table to all zip item entry for fast retrival
		if(UNZ_OK == unzGoToFirstFile(z)) do
		{
			char filePath[128];
			unz_file_info info;
			MCD_VERIFY(unzGetCurrentFileInfo(z, &info, filePath, sizeof(filePath), NULL, 0, NULL, 0) == UNZ_OK);
			unz_file_pos filePos;
			MCD_VERIFY(unzGetFilePos(z, &filePos) == UNZ_OK);

			// Remove trailling '/' if any
			const size_t len = strlen(filePath);
			if(filePath[len - 1] == '/')
				filePath[len - 1] = '\0';

			mFileMap[filePath] = filePos;
		} while(UNZ_OK == unzGoToNextFile(z));
	}

	sal_checkreturn bool getFileInfo(const Path& path, unz_file_info& ret)
	{
		MCD_ASSUME(mQueryContext);
		return mQueryContext->getFileInfo(path, mFileMap, ret);
	}

	std::auto_ptr<istream> openRead(const Path& path)
	{
		auto_ptr<istream> is(nullptr);
		auto_ptr<ZipStreamProxy> newImpl(new ZipStreamProxy(this));

		if(!newImpl->openRead(path.getString().c_str()))
			return is;

		is.reset(new Stream(*newImpl.release()));
		return is;
	}

	struct FileInFolderContext {
		Path path;
		FileMap::const_iterator iter;
	};	// FileInFolderContext

	std::string mZipFilePath;
	/// This context is for use with querying like isExists(), getSize() etc...
	Context* mQueryContext;
	/// Cached file position for locating zip item fast
	FileMap mFileMap;
	ptr_vector<Context> mContextPool;
	mutable RecursiveMutex mMutex;
};	// Impl

ZipFileSystem::ZipStreamProxy::ZipStreamProxy(const ImplPtr& impl)
	: mImpl(impl), mContext(&impl->getContext())
{}

ZipFileSystem::ZipStreamProxy::~ZipStreamProxy()
{
	::free(rawBufPtr());
	ScopeRecursiveLock lock(mImpl->mMutex);
	// The stream operations are complete, we can release the zip handle context
	// back to the pool for further use.
	mImpl->releaseContext(*mContext);
}

bool ZipFileSystem::ZipStreamProxy::openRead(const char* zipItem)
{
	MCD_ASSUME(mContext);
	mContext->mZipFilePath = zipItem;

	unz_file_info fileInfo;
	if(!mContext->getFileInfo(zipItem, mImpl->mFileMap, fileInfo))
		return false;

	if(MCD::isDirectory(fileInfo.external_fa))	// Return null if it's a directory
		return false;

	return unzOpenCurrentFile(mContext->mZipHandle) == UNZ_OK;
}

size_t ZipFileSystem::ZipStreamProxy::doUnzip()
{
	const size_t cBufSize = 512;	// NOTE: Seems 64 is a minimum, haven't study the reason.

	if(!rawBufPtr()) {
		char* buffer = (char*)::malloc(cBufSize);
		if(!buffer)
			return 0;
		setbuf(buffer, cBufSize, cBufSize, mStreamBuf);
	}

	int actualRead = unzReadCurrentFile(mContext->mZipHandle, rawBufPtr(), cBufSize);
	if(actualRead <= 0)
		return 0;

	setbuf(rawBufPtr(), cBufSize, actualRead, mStreamBuf);

	mStreamBuf->pubseekoff(0, std::ios_base::beg, std::ios_base::in);

	return actualRead;
}

size_t ZipFileSystem::ZipStreamProxy::read(char* data, size_t size)
{
	size_t unzipedSize = doUnzip();
	if(size > unzipedSize)
		size = unzipedSize;

	if(size > 0) {
		::memcpy(data, rawBufPtr(), size);
		mStreamBuf->pubseekoff(size, std::ios_base::cur, std::ios_base::in);
	}

	return size;
}

ZipFileSystem::ZipFileSystem(const Path& zipFilePath)
{
	mImpl = new Impl;
	if(!ZipFileSystem::setRoot(zipFilePath))
		throwError("The zip file ", zipFilePath.getString(), " does not exist or corrupted");
}

ZipFileSystem::~ZipFileSystem()
{}

Path ZipFileSystem::getRoot() const
{
	ScopeRecursiveLock lock(mImpl->mMutex);
	return mImpl->mZipFilePath;
}

bool ZipFileSystem::setRoot(const Path& zipFilePath)
{
	return mImpl->setRoot(zipFilePath);
}

bool ZipFileSystem::isExists(const Path& path) const
{
	ScopeRecursiveLock lock(mImpl->mMutex);
	unz_file_info fileInfo;
	return mImpl->getFileInfo(path, fileInfo);
}

bool ZipFileSystem::isDirectory(const Path& path) const
{
	ScopeRecursiveLock lock(mImpl->mMutex);
	unz_file_info fileInfo;
	if(mImpl->getFileInfo(path, fileInfo))
		return MCD::isDirectory(fileInfo.external_fa);
	else throwError("Directory ", path.getString(), " does not exist");
	noReturn();
}

uint64_t ZipFileSystem::getSize(const Path& path) const
{
	ScopeRecursiveLock lock(mImpl->mMutex);
	unz_file_info fileInfo;
	if(mImpl->getFileInfo(path, fileInfo))
		return fileInfo.uncompressed_size;
	else throwError("File ", path.getString(), " does not exist");
	noReturn();
}

std::time_t ZipFileSystem::getLastWriteTime(const Path& path) const
{
	ScopeRecursiveLock lock(mImpl->mMutex);
	unz_file_info fileInfo;
	if(mImpl->getFileInfo(path, fileInfo))
		return fileInfo.dosDate;
	return 0;
}

bool ZipFileSystem::makeDir(const Path& path) const {
	return false;
}

bool ZipFileSystem::remove(const Path& path) const {
	return false;
}

std::auto_ptr<std::istream> ZipFileSystem::openRead(const Path& path) const
{
	ScopeRecursiveLock lock(mImpl->mMutex);
	return mImpl->openRead(path);
}

std::auto_ptr<std::ostream> ZipFileSystem::openWrite(const Path& path) const
{
	ScopeRecursiveLock lock(mImpl->mMutex);
	MCD_ASSERT(false && "Not supported");
	return auto_ptr<ostream>(nullptr);
}

void* ZipFileSystem::openFirstFileInFolder(const Path& folder) const
{
	Path normalizedPath = folder;
	normalizedPath.normalize();

	FileMap::const_iterator i;
	if(normalizedPath.getString().empty())
		i = mImpl->mFileMap.begin();
	else
		i = mImpl->mFileMap.find(normalizedPath.getString());

	if(i == mImpl->mFileMap.end())
		return nullptr;

	Impl::FileInFolderContext* c = new Impl::FileInFolderContext;
	c->path = normalizedPath;
	c->iter = i;

	// Skip the first entry, which is the folder's path
	if(!normalizedPath.getString().empty())
		++(c->iter);

	return c;
}

Path ZipFileSystem::getNextFileInFolder(void* context) const
{
	Path ret;
	Impl::FileInFolderContext* c = reinterpret_cast<Impl::FileInFolderContext*>(context);

	if(!c || c->iter == mImpl->mFileMap.end())
		return ret;

	FileMap::const_iterator& iter = c->iter;

	const std::string& s = iter->first;
	if(s.find(c->path.getString()) == std::string::npos)
		return ret;

	// Skip all child directory
	while(isDirectory(s)) {
		Path p(s);
		while(true) {
			if(++iter == mImpl->mFileMap.end())
				return ret;
			if(iter->first.find(c->path.getString()) == std::string::npos)
				break;
		}
	}

	ret = s;
	++iter;

	return ret;
}

void ZipFileSystem::closeFirstFileInFolder(void* context) const
{
	Impl::FileInFolderContext* c = reinterpret_cast<Impl::FileInFolderContext*>(context);
	delete c;
}

}	// namespace MCD
