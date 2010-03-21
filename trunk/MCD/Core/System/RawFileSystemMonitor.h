#ifndef __MCD_CORE_SYSTEM_RAWFILESYSTEMMONITOR__
#define __MCD_CORE_SYSTEM_RAWFILESYSTEMMONITOR__

#include "../ShareLib.h"
#include "NonCopyable.h"

namespace MCD {

/*!	To monitor file changes under a particular folder.
	Example:
	\code
	RawFileSystemMonitor monitor("pathToMonitor", true);
	// In your main loop:
	while(true) {
		std::wstring path;
		while(!(path = monitor.getChangedFile()).empty()) {
			std::wcout << path << std::endl;
		}
	}
	\endcode
 */
class MCD_CORE_API RawFileSystemMonitor : Noncopyable
{
public:
	RawFileSystemMonitor(const char* path, bool recursive);

	~RawFileSystemMonitor();

	/*!	Get which file under the watching directory is changed.
		This function is non-blocking and if there is no changes in the
		file system, it will simple return an empty string.

		\note
			The current implementation use a fixed buffer to capture all the
			file changes between calls of getChangedFile().
	 */
	std::string getChangedFile() const;

	//!	Returns the path that pass to RawFileSystemMonitor's constructor
	std::string& monitringPath() const;

private:
	class Impl;
	Impl& mImpl;
};	// RawFileSystemMonitor

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RAWFILESYSTEMMONITOR__
