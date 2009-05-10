#include "Pch.h"
#include "RawFileSystem.h"
#include "ErrorCode.h"
#include "PlatformInclude.h"
#include "StrUtility.h"
#include <fstream>
#include <stdexcept>

#ifdef MCD_VC
#	include <ShellAPI.h>	// SHFileOperation
#	include <shlobj.h>		// SHCreateDirectoryEx
#else
#	include <errno.h>
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

static void throwError(const std::string& prefix, const std::wstring& pathStr)
{
	throw std::runtime_error(getSysErrorMsg(
		prefix + "\"" + wStrToStr(pathStr) + "\": "
	));
}

static void throwError(const std::string& prefix, const std::wstring& pathStr, const std::string& sufix)
{
	throw std::runtime_error(
		prefix + "\"" + wStrToStr(pathStr) + "\"" + sufix
	);
}

// Convert the generic path into platform specific format
void toNativePath(Path& path)
{
#ifdef MCD_VC
	Path::string_type& str = const_cast<Path::string_type&>(path.getString());
	for(size_t i=0; i< str.size(); ++i)
		if(str[i] == L'/')
			str[i] = L'\\';
#else
	// No need to do anything for Linux systems
#endif
}

bool isExistsImpl(sal_in_z sal_notnull const wchar_t* path)
{
#ifdef MCD_VC
	MCD_ASSERT(::wcslen(path) < MAX_PATH);
	if(::GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) {
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
	struct stat pathStat;
	std::string narrowStr;
	MCD_VERIFY(wStrToStr(path, narrowStr));
	if(::stat(narrowStr.c_str(), &pathStat) != 0) {
		// stat failed because the path does not exist
		// for any other error we assume the file does exist and fall through,
		// this may not be the best policy though...  (JM 20040330)
		if((errno == ENOENT) || (errno == ENOTDIR))
			return false;
	}
#endif

	return true;
}

bool isDirectoryImpl(sal_in_z sal_notnull const wchar_t* path)
{
#ifdef MCD_VC
	DWORD attributes = ::GetFileAttributesW(path);
	if(attributes == 0xFFFFFFFF)
		throwError("Error getting directory for ", path);
	return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
	struct stat pathStat;
	std::string narrowStr;
	MCD_VERIFY(wStrToStr(path, narrowStr));
	if(::stat(narrowStr.c_str(), &pathStat) != 0)
		throwError("Error getting directory for ", path);
	return S_ISDIR(pathStat.st_mode);
#endif
}

RawFileSystem::RawFileSystem(const Path& rootPath)
{
	if(!RawFileSystem::setRoot(rootPath))
		throwError("The path ", rootPath.getString(), " does not exist or not a directory");
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
	return false;
}

Path RawFileSystem::toAbsolutePath(const Path& path) const
{
	if(!path.hasRootDirectory())
		return mRootPath / path;
	return path;
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
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if(!::GetFileAttributesExW(absolutePath.c_str(), ::GetFileExInfoStandard, &fad))
		throwError("Error getting file size for ", absolutePath);
	if((fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		throwError("The path ", absolutePath, " is a directory");
	return (static_cast<uint64_t>(fad.nFileSizeHigh)
		<< (sizeof(fad.nFileSizeLow)*8))
		+ fad.nFileSizeLow;
#else
	struct stat pathStat;
	std::string narrowStr;
	MCD_VERIFY(wStrToStr(absolutePath.c_str(), narrowStr));
	if(::stat(narrowStr.c_str(), &pathStat) != 0)
		throwError("Error getting file size for ", absolutePath);
	if(S_ISDIR(pathStat.st_mode))
		throwError("The path ", absolutePath, " is a directory");
	return static_cast<uint64_t>(pathStat.st_size);
#endif
}

std::time_t RawFileSystem::getLastWriteTime(const Path& path) const
{
	struct stat pathStat;
	Path::string_type absolutePath = toAbsolutePath(path).getString();
	std::string narrowStr;
	MCD_VERIFY(wStrToStr(absolutePath.c_str(), narrowStr));
	if(::stat(narrowStr.c_str(), &pathStat) != 0)
		throwError("Error getting last write time ", absolutePath);
	return pathStat.st_mtime;
}

bool RawFileSystem::makeDir(const Path& path) const
{
#ifdef MCD_VC
	Path absolutePath = toAbsolutePath(path);
	toNativePath(absolutePath);
	return ::SHCreateDirectoryEx(nullptr, absolutePath.getString().c_str(), nullptr) == 0;
#else
	struct LocalClass {
		static bool mkdir(const std::wstring& path) {
			std::string narrowStr;
			MCD_VERIFY(wStrToStr(path.c_str(), narrowStr));
			// The mkdir mode is set the 777, see http://docs.sun.com/app/docs/doc/816-5167/chmod-2?a=view
			return ::mkdir(narrowStr.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
		}
	};	// LocalClass

	bool success = false;
	PathIterator itr(path);
	while(true) {
		std::wstring s = itr.next();
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
	size_t len = absolutePath.getString().size();

	SHFILEOPSTRUCT fileOp;
	Path::char_type* buf = (Path::char_type*)MCD_STACKALLOCA(sizeof(Path::char_type) * (absolutePath.getString().size() + 2));
	::memcpy(buf, absolutePath.getString().c_str(), len * sizeof(Path::char_type));
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
	std::string narrowStr;
	MCD_VERIFY(wStrToStr(path.getString().c_str(), narrowStr));
	return ::rmdir(narrowStr.c_str()) == 0;
#endif
}

std::auto_ptr<std::istream> RawFileSystem::openRead(const Path& path) const
{
	using namespace std;
	Path::string_type absolutePath = toAbsolutePath(path).getString();

#ifdef MCD_VC
	auto_ptr<istream> is(new ifstream(absolutePath.c_str(), ios::in | ios::binary));
#else
	string aStr;
	MCD_VERIFY(wStrToStr(absolutePath.c_str(), aStr));
	auto_ptr<istream> is(new ifstream(aStr.c_str(), ios::in | ios::binary));
#endif

	if(!is.get() || !(*is))
		is.reset();

	return is;
}

std::auto_ptr<std::ostream> RawFileSystem::openWrite(const Path& path) const
{
	using namespace std;
	Path::string_type absolutePath = toAbsolutePath(path).getString();

#ifdef MCD_VC
	auto_ptr<ostream> os(new ofstream(absolutePath.c_str(), ios::out | ios::binary));
#else
	string aStr;
	MCD_VERIFY(wStrToStr(absolutePath.c_str(), aStr));
	auto_ptr<ostream> os(new ofstream(aStr.c_str(), ios::out | ios::binary));
#endif

	if(!os.get() || !(*os))
		os.reset();

	return os;
}

}	// namespace MCD
