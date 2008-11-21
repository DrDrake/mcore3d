#include "Pch.h"
#include "ZipFileSystem.h"
#include "Stream.h"
#include "StrUtility.h"
#include <fstream>
#include <map>

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
		prefix + "\"" + wStr2Str(pathStr) + "\"" + sufix
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
		operation is done on the first call of read() or seek(). Once unzipped,
		the WHOLE unzipped file is right inside the memory buffer.
	 */
	class ZipStreamProxy : public StreamProxy
	{
		friend class std::auto_ptr<ZipStreamProxy>;

	public:
		ZipStreamProxy(const ZIPENTRY& file, const SharedPtr<ZipFileSystem::Impl>& impl)
			: mFile(file), mImpl(impl)
		{
		}

	protected:
		sal_override ~ZipStreamProxy()
		{
			::free(rawBufPtr());
		}

		size_t doUnzipIfNeeded()
		{
			// Make sure we only unzip once
			if(rawBufPtr())
				return 0;

			// Get the uncompressed file size
			long fileSize = mFile.unc_size;
			if(fileSize <= 0)
				return 0;

			char* buffer = (char*)::malloc(fileSize);
			if(!buffer)
				return 0;

			MCD_ASSERT(mImpl->mZipHandle);
			if(UnzipItem(mImpl->mZipHandle, mFile.index, buffer, fileSize) != ZR_OK)
				return 0;

			setbuf(buffer, fileSize, fileSize, mStreamBuf);
			return fileSize;
		}

		/*	This function should only be invoked in 2 cases:
			1) The unzip operation is not carried out yet, so the buffer is empty and
				under flow occur.
			2) End of stream is reached
		 */
		sal_override size_t read(char* data, size_t size)
		{
			size_t unzipedSize = doUnzipIfNeeded();
			if(size > unzipedSize)
				size = unzipedSize;

			if(size > 0) {
				::memcpy(data, rawBufPtr(), size);
				mStreamBuf->pubseekoff(size, std::ios_base::cur, std::ios_base::in);
			}

			return size;
		}

		const ZIPENTRY& mFile;
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
			if(!str2WStr(ze.name, filePath) || filePath.empty())
				continue;

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
		bool ok = mImpl->init(wStr2Str(zipFilePath.getString()).c_str());
		if(ok)
			mZipFilePath = absolutePath;
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

void ZipFileSystem::makeDir(const Path& path) const {
	MCD_ASSERT(false && "Not supported");
}

void ZipFileSystem::remove(const Path& path) const {
	MCD_ASSERT(false && "Not supported");
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
