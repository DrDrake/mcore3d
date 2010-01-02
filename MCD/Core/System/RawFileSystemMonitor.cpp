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
	Impl(const wchar_t* path, bool recursive)
		: mMonitringPath(path), mRecursive(recursive)
	{
		MCD_ASSERT(int(mBuffer) % 4 == 0 && "Address of mBuffer must be 4-byte aligned");

		mDirectory = ::CreateFileW(
			path,
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
			Log::format(Log::Warn, L"Fail to watch directory: %s", path ? path : L"");
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

	std::wstring getChangedFile() const
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
				Log::write(Log::Warn, L"Error returned by ReadDirectoryChangesW(), most likely the internal buffer is too small");
				readChange();
				goto CACHED;
			}

			FILE_NOTIFY_INFORMATION* p = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(mBuffer);
			while(true)
			{
				std::wstring fileName(p->FileName, p->FileNameLength / sizeof(wchar_t));

				// Skip duplicated entry
				if(mFiles.empty() || fileName != mFiles.back())
					mFiles.push_back(fileName);

				if(p->NextEntryOffset == 0)
					break;

				p = reinterpret_cast<FILE_NOTIFY_INFORMATION*>((char*)p + p->NextEntryOffset);

				// Do some extra buffer overflow check.
				if((char*)p - (char*)mBuffer > sizeof(mBuffer))
					break;
			}

			if(!readChange())
				return L"";
		}

	CACHED:
		if(!mFiles.empty()) {
			std::wstring ret = mFiles.front();
			mFiles.pop_front();
			return ret;
		}

		return L"";
	}

	std::wstring mMonitringPath;
	HANDLE mDirectory;
	bool mRecursive;
	mutable int mBuffer[2048];	//!< This buffer must be 4-byte aligned, therefore we use int as the type.
	mutable OVERLAPPED mOverlapped;
	mutable std::list<std::wstring> mFiles;	//!< A list of wstring acting as a circular buffer.
};	// Impl

RawFileSystemMonitor::RawFileSystemMonitor(const wchar_t* path, bool recursive)
	: mImpl(*new Impl(path, recursive))
{
}

RawFileSystemMonitor::~RawFileSystemMonitor()
{
	delete &mImpl;
}

std::wstring RawFileSystemMonitor::getChangedFile() const
{
	return mImpl.getChangedFile();
}

std::wstring& RawFileSystemMonitor::monitringPath() const
{
	return mImpl.mMonitringPath;
}

#else

RawFileSystemMonitor::RawFileSystemMonitor(const wchar_t* path, bool recursive)
	: mImpl(*((Impl*)nullptr))
{
}

RawFileSystemMonitor::~RawFileSystemMonitor()
{
}

std::wstring RawFileSystemMonitor::getChangedFile() const
{
	return L"";
}

std::wstring& RawFileSystemMonitor::monitringPath() const
{
	static std::wstring s;
	return s;
}

#endif	// MCD_VC

}	// namespace MCD
