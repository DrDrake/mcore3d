#ifndef __MCD_CORE_SYSTEM_LOG__
#define __MCD_CORE_SYSTEM_LOG__

#include "../ShareLib.h"
#include "NonCopyable.h"
#include <iosfwd>

namespace MCD {

/*! A simple logging class.
	\note Multiple threads can call write()/format() but not with start()/stop()

	Example:
	\code
	\endcode
 */
class MCD_CORE_API Log : Noncopyable
{
public:
	enum Level
	{
		None = 0,
		Error = 1 << 0,
		Warn = 1 << 1,
		Info = 1 << 2
	};

	/*!	Associate a std output stream to the log.
		The ownership is given to the log once the stream is associated with the log,
		and it will be deleted in stop().
	 */
	static void start(sal_in std::wostream* os);

	/*!	Set the logging level.
		All level are on by default.
	 */
	static void setLevel(Level level);

	//! Write the message to the log.
	static void write(Level level, sal_in_z const wchar_t* msg);

	//! Write a formatted message to the log, similar to what printf does.
#ifdef MCD_VC
	static void format(Level level, sal_format_guard const wchar_t* fmt, ...);
#elif __GNUC__ >= 4
	static void format(Level level, const wchar_t* fmt, ...) /*__attribute__((format(wprintf, 1, 2)))*/;
#else
	static void format(Level level, const wchar_t* fmt, ...);
#endif
	;

	//! De-initialize the log, with the option to delete the stream or not.
	static void stop(bool destroyStream=true);
};	// Log

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_LOG__
