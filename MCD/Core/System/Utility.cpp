#include "Pch.h"
#include "Utility.h"
#include "ErrorCode.h"

namespace MCD {

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
	int errCode = MCD::getLastError();
	if(errCode == 0)
		msg += "S_OK";
	else
		msg +=MCD::getErrorMessage(nullptr, errCode);

	msg += "\"";
	throw std::runtime_error(msg);
}

void swapMemory(void* p1, void* p2, size_t size)
{
	char* _p1 = (char*)p1;
	char* _p2 = (char*)p2;
	for(size_t i=0; i<size; ++i, ++_p1, ++_p2) {
		char tmp = *_p1;
		*_p1 = *_p2;
		*_p2 = tmp;
	}
}

}	// namespace MCD
