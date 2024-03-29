#pragma once

#include "../../MCD/Core/System/FileSystemCollection.h"
#include "../../MCD/Core/System/PtrVector.h"
#include "../../MCD/Core/System/RawFileSystemMonitor.h"

namespace Binding {

class RawFileMonitors : public MCD::ptr_vector<MCD::RawFileSystemMonitor> {};

/*!	A place for everyone to access
 */
public ref class FileSystemCollection
{
public:
	FileSystemCollection();

	RawFileMonitors& monitors();

// Functions from MCD::FileSystemCollection
	System::String^ getRoot();

	bool setRoot(System::String^ rootPath);

	bool isExists(System::String^ path);

	bool isDirectory(System::String^ path);

	uint64_t getSize(System::String^ path);

//	std::time_t getLastWriteTime(System::String^ path);

// Function that are specifics for the .Net binding
	sal_notnull MCD::FileSystemCollection* getRawPtr();

	//! Add a file system, the path can be a folder path or a zip file.
	void addFileSystem(System::String^ pathToFileSystem);

	bool removeFileSystem(System::String^ pathToFileSystem);

	/*!	Open the file and return it as a whole single string.
		\note It gives you a raw array of char representation of the file content,
			don't expect to have any line ending or unicode converstion.
	 */
	System::String^ openAsString(System::String^ path);

	/*!	Save a text string to a file.
		The string will be saved in UTF-8 encoding.
	 */
	bool saveString(System::String^ path, System::String^ str);

	typedef System::Collections::Generic::List<System::String^> FileSystems;

	typedef System::Collections::Specialized::StringCollection StringCollection;

	// TODO: Can make it IEnumerable?
	property StringCollection^ fileSystems {
		StringCollection^ get();
	}

	//! Gets all the first level directories under \em path.
	StringCollection^ getDirectories(System::String^ path);

	//! Gets all the first level files under \em path.
	StringCollection^ getFiles(System::String^ path);

protected:
	~FileSystemCollection();
	!FileSystemCollection();

	MCD::FileSystemCollection* mImpl;
	RawFileMonitors* mMonitors;

	StringCollection^ mFileSystems;
};	// FileSystemCollection

}	// namespace Binding
