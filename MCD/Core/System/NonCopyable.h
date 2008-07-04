#ifndef __SGE_CORE_SYSTEM_NONCOPYABLE__
#define __SGE_CORE_SYSTEM_NONCOPYABLE__

#include "Platform.h"

namespace SGE {

namespace Impl  // Protection from unintended ADL
{
	class Noncopyable
	{
	protected:
		Noncopyable() {}
		~Noncopyable() throw() {}
	private:	// Emphasize the following members are private
		Noncopyable(const Noncopyable&);
		const Noncopyable& operator=(const Noncopyable&);
	};	// Noncopyable
}	// namespace Impl

/*!	Private copy constructor and copy assignment ensure classes derived from
	Noncopyable is a base class.  Derive your own class from Noncopyable
	when you want to prohibit copy construction and copy assignment.
	\sa http://www.boost.org/libs/utility/utility.htm
 */
typedef Impl::Noncopyable Noncopyable;

}	// namespace SGE

#endif	// __SGE_CORE_SYSTEM_NONCOPYABLE__
