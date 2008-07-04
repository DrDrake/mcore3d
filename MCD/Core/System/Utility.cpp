#include "Pch.h"
#include "Utility.h"
#include "ErrorCode.h"

namespace SGE {

void throwIfNull(
	sal_maybenull const void* pointerToCheck,
	sal_in_z sal_notnull const char* message
	) throw(std::runtime_error)
{
	if(pointerToCheck == nullptr)
		throw std::runtime_error(message);
}

void throwSystemErrorMessage(
	sal_in_z sal_notnull const char* prefixMessage
	) throw(std::runtime_error)
{
	std::string msg(prefixMessage);
	msg += " Reason :\"";
	int errCode = SGE::getLastError();
	if(errCode == 0)
		msg += "S_OK";
	else
		msg +=SGE::getErrorMessage(nullptr, errCode);

	msg += "\"";
	throw std::runtime_error(msg);
}

}	// namespace SGE
