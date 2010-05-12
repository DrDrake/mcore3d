#ifndef __MCD_CORE_SYSTEM_EXCEPTION__
#define __MCD_CORE_SYSTEM_EXCEPTION__

#include "Platform.h"
#include "../ShareLib.h"
#include <stdexcept>

namespace MCD {

//! An interface that will provide a wide char version of error message.
class MCD_ABSTRACT_CLASS IException
{
public:
	virtual ~IException() {}
	virtual const wchar_t* wwhat() const throw() = 0;
};	// IException

//! Extending the std::runtime_error with wchar_t support.
class MCD_CORE_API RuntimeError : public std::runtime_error, public IException
{
public:
	explicit RuntimeError(sal_in_z sal_notnull const char* msg);
	explicit RuntimeError(sal_in_z sal_notnull const wchar_t* msg);

	sal_override ~RuntimeError() throw() {}

	/*!	Override function of std::exception.
		Original error message may not shown correctly if
		it cannot be displayed in the current code page.
	 */
	sal_override const char* what() const throw();

	//! Wide char version of what()
	sal_override const wchar_t* wwhat() const throw();

protected:
	mutable std::string mAMessage;	//! Storage for ansi string
	mutable std::wstring mWMessage;	//! Storage for wide string
};	// RuntimeError

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_EXCEPTION__
