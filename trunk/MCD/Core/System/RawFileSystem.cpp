#include "Pch.h"
#include "RawFileSystem.h"
#include "ErrorCode.h"
#include "Log.h"
#include "PlatformInclude.h"
#include "StrUtility.h"
#include <fstream>
#include <malloc.h>	// _malloca

#ifdef MCD_VC
#	include <ShellAPI.h>	// SHFileOperation
#	include <shlobj.h>		// SHCreateDirectoryEx
#else
#	include <errno.h>
#endif

#ifdef MCD_IPHONE
#	import <Foundation/NSBundle.h>
#	import <Foundation/NSString.h>
#endif

#include <sys/stat.h>		// stat

namespace MCD {

static std::string getSysErrorMsg(const std::string& prefix)
{
	int errCode = MCD::getLastError();
	if(errCode == 0)
		return "";
	return MCD::getErrorMessage(prefix.c_str(), errCode);
}

static bool logError(const std::string& prefix, const std::string& pathStr)
{
	Log::write(Log::Warn, getSysErrorMsg(prefix + "\"" + pathStr + "\": ").c_str());
	return false;
}

static bool logError(const char* prefix, const char* pathStr, const char* sufix)
{
	Log::format(Log::Warn, "%s\"%s\"%s", prefix, pathStr, sufix);
	return false;
}

// Convert the generic path into platform specific format
void toNativePath(Path& path)
{
#ifdef MCD_VC
	Path::string_type& str = const_cast<Path::string_type&>(path.getString());
	for(size_t i=0; i< str.size(); ++i)
		if(str[i] == '/')
			str[i] = '\\';
#else
	// No need to do anything for Linux systems
#endif
}

bool isExistsImpl(sal_in_z sal_notnull const char* path)
{
#ifdef MCD_VC
	std::wstring wideStr;
	MCD_VERIFY(utf8ToWStr(path, wideStr));
	MCD_ASSERT(wideStr.length() < MAX_PATH);
	if(::GetFileAttributesW(wideStr.c_str()) == INVALID_FILE_ATTRIBUTES) {
		// GetFileAttributes failed because the path does not exist
		// for any other error we assume the file does exist and fall through,
		// this may not be the best policy though...  (JM 20040330)
		switch(::GetLastError()) {
		case ERROR_FILE_NOT_FOUND:
		case ERROR_INVALID_PARAMETER:
		case ERROR_NOT_READY:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_INVALID_NAME:
		case ERROR_BAD_NETPATH:
			return false;
		default:
			return true;
		}
	}
#else
	if(strcmp("/", path) == 0) return true;
	struct stat pathStat;
	errno = 0;
	if(::stat(path, &pathStat) != 0) {
		// stat failed because the path does not exist
		// for any other error we assume the file does exist and fall through,
		// this may not be the best policy though...  (JM 20040330)
		if((errno == ENOENT) || (errno == ENOTDIR))
			return false;
	}
#endif

	return true;
}

bool isDirectoryImpl(sal_in_z sal_notnull const char* path)
{
#ifdef MCD_VC
	std::wstring wideStr;
	MCD_VERIFY(utf8ToWStr(path, wideStr));
	DWORD attributes = ::GetFileAttributesW(wideStr.c_str());
	if(attributes == 0xFFFFFFFF)
		return logError("Error getting directory for ", path);
	return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
	struct stat pathStat;
	if(::stat(path, &pathStat) != 0)
		return logError("Error getting directory for ", path);
	return S_ISDIR(pathStat.st_mode);
#endif
}

RawFileSystem::RawFileSystem(const Path& rootPath)
{
	if(!RawFileSystem::setRoot(rootPath))
		logError("The path ", rootPath.getString().c_str(), " does not exist or not a directory");
}

Path RawFileSystem::getRoot() const {
	return mRootPath;
}

bool RawFileSystem::setRoot(const Path& rootPath)
{
	Path absolutePath = rootPath;
	if(!absolutePath.hasRootDirectory())
		absolutePath = Path::getCurrentPath() / absolutePath;

	const Path::char_type* str = absolutePath.getString().c_str();

	// The input must be an existing directory
	if(isExistsImpl(str) && isDirectoryImpl(str)) {
		mRootPath = absolutePath;
		return true;
	}
	else {	// Search for existance of a "*.dir" file which act as a shortcut file
		Path::string_type shortCut = rootPath.getString() + ".dir";
		if(isExistsImpl(shortCut.c_str()) && !isDirectoryImpl(shortCut.c_str()))
		{
			std::auto_ptr<std::istream> is = openRead(shortCut);
			if(is.get() && is->good())
			{
				// Read the directory that this shortcut points to
				std::string dir;
				(*is) >> dir;
				mRootPath = dir;
				return true;
			}
		}
	}

	return false;
}

Path RawFileSystem::toAbsolutePath(const Path& path) const
{
#ifdef MCD_IPHONE
	// Adjust the final path into the resource bundle.
	Path tmp(path);
	if(!path.hasRootDirectory())
		tmp = mRootPath / path;

	const char* cpath = tmp.getString().c_str();
	// Skip '/'
	if(cpath[0] == '/')
		++cpath;

	NSString* fullPath = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:cpath] ofType:nil];
	if(fullPath)
		return Path([fullPath UTF8String]);
	else
		return tmp;
#else
	if(!path.hasRootDirectory())
		return mRootPath / path;
	return path;
#endif
}

bool RawFileSystem::isExists(const Path& path) const
{
	Path::string_type absolutePath = toAbsolutePath(path).getString();
	return isExistsImpl(absolutePath.c_str());
}

bool RawFileSystem::isDirectory(const Path& path) const
{
	Path::string_type absolutePath = toAbsolutePath(path).getString();
	return isDirectoryImpl(absolutePath.c_str());
}

uint64_t RawFileSystem::getSize(const Path& path) const
{
	Path::string_type absolutePath = toAbsolutePath(path).getString();
#ifdef MCD_VC
	std::wstring wideStr;
	MCD_VERIFY(utf8ToWStr(absolutePath.c_str(), wideStr));
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if(!::GetFileAttributesExW(wideStr.c_str(), ::GetFileExInfoStandard, &fad))
		return logError("Error getting file size for ", absolutePath);
	if((fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		return logError("The path ", absolutePath.c_str(), " is a directory");
	return (static_cast<uint64_t>(fad.nFileSizeHigh)
		<< (sizeof(fad.nFileSizeLow)*8))
		+ fad.nFileSizeLow;
#else
	struct stat pathStat;
	if(::stat(absolutePath.c_str(), &pathStat) != 0)
		return logError("Error getting file size for ", absolutePath);
	if(S_ISDIR(pathStat.st_mode))
		return logError("The path ", absolutePath.c_str(), " is a directory");
	return static_cast<uint64_t>(pathStat.st_size);
#endif
}

std::time_t RawFileSystem::getLastWriteTime(const Path& path) const
{
	struct stat pathStat;
	Path::string_type absolutePath = toAbsolutePath(path).getString();
	if(::stat(absolutePath.c_str(), &pathStat) != 0)
		return logError("Error getting last write time ", absolutePath);
	return pathStat.st_mtime;
}

bool RawFileSystem::makeDir(const Path& path) const
{
#ifdef MCD_VC
	Path absolutePath = toAbsolutePath(path);
	toNativePath(absolutePath);
	std::wstring wideStr;
	MCD_VERIFY(utf8ToWStr(absolutePath.getString().c_str(), wideStr));
	return ::SHCreateDirectoryExW(nullptr, wideStr.c_str(), nullptr) == 0;
#else
	struct LocalClass {
		static bool mkdir(const std::string& path) {
			// The mkdir mode is set the 777, see http://docs.sun.com/app/docs/doc/816-5167/chmod-2?a=view
			return ::mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
		}
	};	// LocalClass

	bool success = false;
	PathIterator itr(path);
	while(true) {
		std::string s = itr.next();
		if(s.empty())
			break;
		if(isExists(s)) {
			if(isDirectory(s))	// Directory already exist
				continue;
			else
				break;
		}
		if(!LocalClass::mkdir(s))
			break;
		success = true;
	}
	return success;
#endif
}

bool RawFileSystem::remove(const Path& path) const
{
#ifdef MCD_VC
	Path absolutePath = toAbsolutePath(path);
	toNativePath(absolutePath);

	std::wstring wideStr;
	MCD_VERIFY(utf8ToWStr(absolutePath.getString(), wideStr));

	size_t len = wideStr.size();

	SHFILEOPSTRUCT fileOp;
	wchar_t* buf = (wchar_t*)MCD_STACKALLOCA(sizeof(wchar_t) * (len + 2));
	::memcpy(buf, wideStr.c_str(), len * sizeof(wchar_t));
	buf[len] = buf[len+1] = L'\0';	// Double null terminated
	::ZeroMemory(&fileOp, sizeof(fileOp));

	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = buf;
	fileOp.fFlags = FOF_SILENT | FOF_ALLOWUNDO | FOF_NOCONFIRMATION;

	int ret = ::SHFileOperationW(&fileOp);
	MCD_STACKFREE(buf);
	return ret == 0;
#else
	// TODO: Not recursive remove is not implemented yet,
	// unless directory listing is ready
	return ::rmdir(path.getString().c_str()) == 0;
#endif
}

std::auto_ptr<std::istream> RawFileSystem::openRead(const Path& path) const
{
	using namespace std;
	auto_ptr<istream> is;
	Path::string_type absolutePath = toAbsolutePath(path).getString();

#ifdef MCD_VC
	std::wstring wideStr;
	if(utf8ToWStr(absolutePath, wideStr))
		is.reset(new ifstream(wideStr.c_str(), ios::in | ios::binary));
#else
	is.reset(new ifstream(absolutePath.c_str(), ios::in | ios::binary));
#endif

	if(!is.get() || !(*is))
		is.reset();

	return is;
}

std::auto_ptr<std::ostream> RawFileSystem::openWrite(const Path& path) const
{
	using namespace std;
	auto_ptr<ostream> os;
	Path::string_type absolutePath = toAbsolutePath(path).getString();

#ifdef MCD_VC
	std::wstring wideStr;
	if(utf8ToWStr(absolutePath, wideStr))
		os.reset(new ofstream(wideStr.c_str(), ios::out | ios::binary));
#else
	os.reset(new ofstream(absolutePath.c_str(), ios::out | ios::binary));
#endif

	if(!os.get() || !(*os))
		os.reset();

	return os;
}

namespace {

struct SearchContext
{
#ifdef MCD_VC
	~SearchContext() {
		if(handle != INVALID_HANDLE_VALUE)
			::FindClose(handle);
	}

	HANDLE handle;
	WIN32_FIND_DATA data;
#endif

};	// SearchFileContext

}	// namespace

void* RawFileSystem::openFirstChildFolder(const Path& folder) const
{
#ifdef MCD_VC
	SearchContext* c = new SearchContext;
	Path::string_type absolutePath = toAbsolutePath(folder).getString() + "/*";

	std::wstring wideStr;
	MCD_VERIFY(utf8ToWStr(absolutePath, wideStr));
	c->handle = ::FindFirstFileW(wideStr.c_str(), &(c->data));

	if(c->handle == INVALID_HANDLE_VALUE)
		goto Fail;

	while(!(c->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
		::wcscmp(c->data.cFileName, L".") == 0 || ::wcscmp(c->data.cFileName, L"..") == 0)
	{
		if(!FindNextFile(c->handle, &(c->data)))
			goto Fail;
	}

	return c;

Fail:
	closeFirstChildFolder(c);
#endif

	return nullptr;
}

Path RawFileSystem::getNextSiblingFolder(void* context) const
{
	Path ret;
	if(!context)
		return ret;

#ifdef MCD_VC
	SearchContext* c = reinterpret_cast<SearchContext*>(context);
	std::string utf8Str;
	MCD_VERIFY(wStrToUtf8(c->data.cFileName, utf8Str));
	ret = utf8Str;

	do {
		if(!::FindNextFileW(c->handle, &(c->data))) {
			c->data.cFileName[0] = '\0';
			break;
		}
	} while(!(c->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
#endif

	return ret;
}

void RawFileSystem::closeFirstChildFolder(void* context) const
{
	SearchContext* c = reinterpret_cast<SearchContext*>(context);
	delete c;
}

void* RawFileSystem::openFirstFileInFolder(const Path& folder) const
{
#ifdef MCD_VC
	SearchContext* c = new SearchContext;
	Path::string_type absolutePath = toAbsolutePath(folder).getString() + "/*";
	std::wstring wideStr;
	MCD_VERIFY(utf8ToWStr(absolutePath, wideStr));
	c->handle = ::FindFirstFileW(wideStr.c_str(), &(c->data));

	if(c->handle == INVALID_HANDLE_VALUE)
		goto Fail;

	while(c->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if(!FindNextFile(c->handle, &(c->data)))
			goto Fail;
	}

	return c;

Fail:
	closeFirstChildFolder(c);
#endif

	return nullptr;
}

Path RawFileSystem::getNextFileInFolder(void* context) const
{
	Path ret;
	if(!context)
		return ret;

#ifdef MCD_VC
	SearchContext* c = reinterpret_cast<SearchContext*>(context);
	std::string utf8Str;
	MCD_VERIFY(wStrToUtf8(c->data.cFileName, utf8Str));
	ret = utf8Str;

	do {
		if(!::FindNextFileW(c->handle, &(c->data))) {
			c->data.cFileName[0] = '\0';
			break;
		}
	} while(c->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#endif

	return ret;
}

void RawFileSystem::closeFirstFileInFolder(void* context) const
{
	closeFirstChildFolder(context);
}

}	// namespace MCD
