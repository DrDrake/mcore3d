#ifndef __SGE_CORE_SYSTEM_EXCEPTION__
#define __SGE_CORE_SYSTEM_EXCEPTION__

#include "Platform.h"
#include "../ShareLib.h"
#include <stdexcept>

namespace SGE {

//! An interface that will provide a wide char version of error message.
class SGE_NOVTABLE IException
{
public:
	virtual const wchar_t* wwhat() const throw() = 0;
};	// IException

//! Extening the std::runtime_error with wchar_t support.
class SGE_CORE_API RuntimeError : public std::runtime_error, public IException
{
public:
	explicit RuntimeError(sal_in_z sal_notnull const char* msg) : std::runtime_error(msg) {}
	explicit RuntimeError(sal_in_z sal_notnull const wchar_t* msg);

	sal_override ~RuntimeError() throw() {}

	/*!	Override function of std::exception.
		Orginal error message may not shown correctly if
		it cannot be displayed in the current code page.
	 */
    sal_override const char* what() const throw();

	//! Wide char version of what()
	sal_override const wchar_t* wwhat() const throw();

protected:
	std::wstring mMessage;
};	// RuntimeError

}	// namespace SGE

#endif	// __SGE_CORE_SYSTEM_EXCEPTION__
