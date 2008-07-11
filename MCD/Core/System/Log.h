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
	/*!	Associate a std output stream to the log.
		The ownership is given to the log once the stream is associated with the log,
		and it will be deleted in stop().
	 */
	static void start(sal_in std::wostream* os);

	//! Write the message to the log.
	static void write(sal_in_z const wchar_t* msg);

	//! Write a formatted message to the log, similar to what printf does.
#ifdef MCD_VC
	static void format(_In_z_ _Printf_format_string_ const wchar_t* fmt, ...);
#elif __GNUC__ >= 4
	static void format(const wchar_t* fmt, ...) __attribute__((format(wprintf, 1, 2)));
#else
	static void format(const wchar_t* fmt, ...);
#endif
	;

	//! De-initialize the log
	static void stop();
};	// Log

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_LOG__
