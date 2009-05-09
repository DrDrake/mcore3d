#include "Pch.h"
#include "ZipFileSystem.h"
#include "Log.h"
#include "Stream.h"
#include "StrUtility.h"
#include <fstream>
#include <map>
#include <stdexcept>

#define ZIP_STD
#undef UNICODE
#include "ZipFileSystem.inc"

#ifdef MCD_VC
#	pragma comment(lib, "zlib")
#endif	// MCD_VC

using namespace std;

namespace MCD {

static void throwError(const std::string& prefix, const std::wstring& pathStr, const std::string& sufix)
{
	throw std::runtime_error(
		prefix + "\"" + wStrToStr(pathStr) + "\"" + sufix
	);
}

/*!	The private implementation of ZipFileSystem stores the handle to the zip file.
	It need to be shared among opened iostreams so that the opened iostream can
	keep working while the ZipFileSystem itself is already destroyed or it's zip
	file path is changed.
 */
class ZipFileSystem::Impl
{
	/*!	This stream proxy will perform a lazy unzip, that means the actual unzip
		operation is done during call of read().
	 */
	class ZipStreamProxy : public StreamProxy
	{
		friend class std::auto_ptr<ZipStreamProxy>;

	public:
		ZipStreamProxy(const ZIPENTRY& file, const SharedPtr<ZipFileSystem::Impl>& impl)
			: mFile(file), mUnzippedBytes(0), mImpl(impl)
		{
		}

	protected:
		sal_override ~ZipStreamProxy()
		{
			::free(rawBufPtr());
		}

		size_t doUnzip()
		{
			// TODO: The UnzipItem() function didn't work correctly if an (~6M) unzipped
			// file is unzipped by spliting it up to several chunk. So currectly a large
			// enough buffer is allocated at once, not good for a memory limited system.
//			static const size_t cBufSize = 512;
			const size_t cBufSize = mFile.unc_size;

			if(!rawBufPtr()) {
				char* buffer = (char*)::malloc(cBufSize);
				if(!buffer)
					return 0;
				setbuf(buffer, cBufSize, cBufSize, mStreamBuf);
			}

			MCD_ASSERT(mImpl->mZipHandle);
			ZRESULT ret = UnzipItem(mImpl->mZipHandle, mFile.index, rawBufPtr(), cBufSize);

			size_t actualRead;

			if(ret == ZR_MORE)
				actualRead = cBufSize;
			else if(ret == ZR_OK) {
				actualRead = mFile.unc_size - mUnzippedBytes;
				setbuf(rawBufPtr(), cBufSize, actualRead, mStreamBuf);
			}
			else
				actualRead = 0;

			mUnzippedBytes += actualRead;
			mStreamBuf->pubseekoff(0, std::ios_base::beg, std::ios_base::in);

			return actualRead;
		}

		sal_override size_t read(char* data, size_t size)
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

		const ZIPENTRY& mFile;
		size_t mUnzippedBytes;	//! How much we have actually unzipped
		// Hold a reference to ZipFileSystem::Impl so that it won't destroyed too soon
		const SharedPtr<ZipFileSystem::Impl> mImpl;
	};	// ZipStreamProxy

public:
	Impl() : mZipHandle(nullptr) {}

	// This function will create a std::map from file path+name to file handle ZIPENTRY
	bool init(const char* file)
	{
		MCD_ASSERT(!mZipHandle && "init should called once");

		mZipHandle = OpenZip(file, nullptr/*password*/);

		if(!mZipHandle)
			return false;

		ZIPENTRY zeAll;
		if(GetZipItem(mZipHandle, -1, &zeAll) != ZR_OK)	// -1 gives overall information about the zipfile
			return false;

		int numitems = zeAll.index;

		for(int i = 0; i < numitems; ++i)
		{
			ZIPENTRY ze;
			if(GetZipItem(mZipHandle, i, &ze) != ZR_OK) // Fetch individual details
				continue;

			std::wstring filePath;

			// We assume the file name stored in the zip file use utf-8 encoding
			if(!utf8ToWStr(ze.name, MAX_PATH, filePath) || filePath.empty()) {
				Log::format(Log::Warn, L"The zip file: \"%s\" contains a file path that is not encoded in UTF-8", mZipFilePath.getString().c_str());
				continue;
			}

			// Remove trailling '/' if any
			if(filePath[filePath.size() - 1] == L'/')
				filePath.resize(filePath.size() - 1);

			mFileMap[filePath] = ze;
		}

		return true;
	}

	~Impl()
	{
		// Should have no more stream referencing the zip file, close it
		CloseZip(mZipHandle);
	}

	const ZIPENTRY* getFile(const Path& path)
	{
		FileMap::const_iterator i = mFileMap.find(path.getString());
		if(i == mFileMap.end())
			return nullptr;

		return &(i->second);
	}

	std::auto_ptr<istream> openRead(const Path& path, const SharedPtr<Impl>& impl)
	{
		MCD_ASSERT(impl == this);
		auto_ptr<istream> is(nullptr);

		const ZIPENTRY* f = getFile(path);

		if(!f || (f->attr & S_IFDIR) > 0)	// Return null if it's a directory
			return is;

		// Note that we pass impl instead of "this" to ZipStreamProxy constructor
		// for the non-intrusive shard pointer to work correctly
		std::auto_ptr<ZipStreamProxy> proxy(new ZipStreamProxy(*f, impl));
		if(!proxy.get())
			return is;

		is.reset(new Stream(*proxy));
		if(is.get())
			proxy.release();

		return is;
	}

	HZIP mZipHandle;
	typedef std::map<std::wstring, ZIPENTRY> FileMap;
	FileMap mFileMap;
	Path mZipFilePath;
};	// Impl

ZipFileSystem::ZipFileSystem(const Path& zipFilePath)
{
	if(!ZipFileSystem::setRoot(zipFilePath))
		throwError("The zip file ", zipFilePath.getString(), " does not exist or corrupted");
}

ZipFileSystem::~ZipFileSystem()
{
}

Path ZipFileSystem::getRoot() const {
	return mZipFilePath;
}

bool ZipFileSystem::setRoot(const Path& zipFilePath)
{
	Path absolutePath = zipFilePath;
	if(!absolutePath.hasRootDirectory())
		absolutePath = Path::getCurrentPath() / absolutePath;

	if(absolutePath != mZipFilePath)
	{
		mImpl = new Impl;
		bool ok = mImpl->init(wStrToStr(zipFilePath.getString()).c_str());
		if(ok)
			mImpl->mZipFilePath = mZipFilePath = absolutePath;
		return ok;
	}

	return true;
}

bool ZipFileSystem::isExists(const Path& path) const
{
	return mImpl->getFile(Path(path).normalize()) != nullptr;
}

bool ZipFileSystem::isDirectory(const Path& path) const
{
	const ZIPENTRY* f = mImpl->getFile(Path(path).normalize());

	if(f) return (f->attr & S_IFDIR) > 0;
	else throwError("Directory ", path.getString(), " does not exist");
	noReturn();
}

uint64_t ZipFileSystem::getSize(const Path& path) const
{
	const ZIPENTRY* f = mImpl->getFile(Path(path).normalize());

	if(f) return f->unc_size;
	else throwError("File ", path.getString(), " does not exist");
	noReturn();
}

std::time_t ZipFileSystem::getLastWriteTime(const Path& path) const
{
	const ZIPENTRY* f = mImpl->getFile(Path(path).normalize());
	return f ? f->mtime : 0;
}

bool ZipFileSystem::makeDir(const Path& path) const {
	return false;
}

bool ZipFileSystem::remove(const Path& path) const {
	return false;
}

std::auto_ptr<std::istream> ZipFileSystem::openRead(const Path& path) const
{
	return mImpl->openRead(Path(path).normalize(), mImpl);
}

std::auto_ptr<std::ostream> ZipFileSystem::openWrite(const Path& path) const
{
	MCD_ASSERT(false && "Not supported");
	return auto_ptr<ostream>(nullptr);
}

}	// namespace MCD
