#include "Pch.h"
#include "RawFileSystemMonitor.h"
#include "Log.h"
#include "PlatformInclude.h"
#include "StrUtility.h"
#include <list>

namespace MCD {

#ifdef MCD_VC	// Currently only support windows

class RawFileSystemMonitor::Impl
{
public:
	Impl(const char* path, bool recursive)
		: mMonitringPath(path), mRecursive(recursive)
	{
		MCD_ASSERT(int(mBuffer) % 4 == 0 && "Address of mBuffer must be 4-byte aligned");

		std::wstring wideStr;
		MCD_VERIFY(utf8ToWStr(path, wideStr));

		mDirectory = ::CreateFileW(
			wideStr.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS,	// ReadDirectoryChangesW() needs FILE_FLAG_BACKUP_SEMANTICS
			0
		);

		memset(&mOverlapped, 0, sizeof(mOverlapped));
		if(!mDirectory || !readChange()) {
			::CloseHandle(mDirectory);
			mDirectory = nullptr;
			Log::format(Log::Warn, "Fail to watch directory: %s", path ? path : "");
		}
	}

	~Impl()
	{
		::CloseHandle(mDirectory);
	}

	bool readChange() const
	{
		return ::ReadDirectoryChangesW(
			mDirectory,
			mBuffer, sizeof(mBuffer),
			mRecursive,
			FILE_NOTIFY_CHANGE_LAST_WRITE,
			nullptr,		// bytesRetured
			&mOverlapped,
			0				// callBack
		) != 0;
	}

	std::string getChangedFile() const
	{
		// We will try to call GetOverlappedResult() even there are entries inside
		// mFiles, so that it's less possible for the mBuffer to be overflowed.

		// For some unknown reason(s) ReadDirectoryChangesW() will report the file twice,
		// therefore we add a loop to filter out those duplicated entries.
		for(size_t i=2; i--;)
		{
			DWORD bytesRetured = 0;
			// Note that if mDirectory = null, GetOverlappedResult() will return 0
			if(0 == ::GetOverlappedResult(mDirectory, &mOverlapped, &bytesRetured, false))
				goto CACHED;	// The use of goto here makes the code clean.

			if(bytesRetured == 0) {
				// TODO: To reduce the chance of insufficient buffer, we can move the code to another thread.
				Log::write(Log::Warn, "Error returned by ReadDirectoryChangesW(), most likely the internal buffer is too small");
				readChange();
				goto CACHED;
			}

			FILE_NOTIFY_INFORMATION* p = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(mBuffer);
			while(true)
			{
				std::wstring fileName(p->FileName, p->FileNameLength / sizeof(wchar_t));
				std::string utf8Str;
				MCD_VERIFY(wStrToUtf8(fileName, utf8Str));

				// Skip duplicated entry
				if(mFiles.empty() || utf8Str != mFiles.back())
					mFiles.push_back(utf8Str);

				if(p->NextEntryOffset == 0)
					break;

				p = reinterpret_cast<FILE_NOTIFY_INFORMATION*>((char*)p + p->NextEntryOffset);

				// Do some extra buffer overflow check.
				if((char*)p - (char*)mBuffer > sizeof(mBuffer))
					break;
			}

			if(!readChange())
				return "";
		}

	CACHED:
		if(!mFiles.empty()) {
			std::string ret = mFiles.front();
			mFiles.pop_front();
			return ret;
		}

		return "";
	}

	std::string mMonitringPath;
	HANDLE mDirectory;
	bool mRecursive;
	mutable int mBuffer[2048];	//!< This buffer must be 4-byte aligned, therefore we use int as the type.
	mutable OVERLAPPED mOverlapped;
	mutable std::list<std::string> mFiles;	//!< A list of string acting as a circular buffer.
};	// Impl

RawFileSystemMonitor::RawFileSystemMonitor(const char* path, bool recursive)
	: mImpl(*new Impl(path, recursive))
{
}

RawFileSystemMonitor::~RawFileSystemMonitor()
{
	delete &mImpl;
}

std::string RawFileSystemMonitor::getChangedFile() const
{
	return mImpl.getChangedFile();
}

std::string& RawFileSystemMonitor::monitringPath() const
{
	return mImpl.mMonitringPath;
}

#else

RawFileSystemMonitor::RawFileSystemMonitor(const char* path, bool recursive)
	: mImpl(*((Impl*)nullptr))
{
}

RawFileSystemMonitor::~RawFileSystemMonitor()
{
}

std::string RawFileSystemMonitor::getChangedFile() const
{
	return "";
}

std::string& RawFileSystemMonitor::monitringPath() const
{
	static std::string s;
	return s;
}

#endif	// MCD_VC

}	// namespace MCD
