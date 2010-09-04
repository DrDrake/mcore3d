#ifndef __MCD_CORE_SYSTEM_NONCOPYABLE__
#define __MCD_CORE_SYSTEM_NONCOPYABLE__

#include "Platform.h"

namespace MCD {

/*!	Private copy constructor and copy assignment ensure classes derived from
	Noncopyable is a base class.  Derive your own class from Noncopyable
	when you want to prohibit copy construction and copy assignment.
	\sa http://www.boost.org/libs/utility/utility.htm
 */
class Noncopyable
{
protected:
	Noncopyable() {}
	~Noncopyable() {}

private:
	// Emphasize the following members are private
	Noncopyable(const Noncopyable&);
	const Noncopyable& operator=(const Noncopyable&);
};	// Noncopyable

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_NONCOPYABLE__
