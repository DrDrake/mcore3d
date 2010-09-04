#include "Pch.h"
#include "Utility.h"
#include "ErrorCode.h"
#include "Log.h"

namespace MCD {

void logSystemErrorMessage(
	sal_in_z sal_notnull const char* prefixMessage
	)
{
	const int errCode = MCD::getLastError();
	Log::format(Log::Warn, "%s Reason:\"%s\"", prefixMessage,
		errCode == 0 ? "S_OK" : MCD::getErrorMessage(nullptr, errCode).c_str()
	);
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

bool isLittleEndian()
{
	short int word = 0x0001;
	char* byte = (char*)&word;
	return byte[0] != 0;
}

}	// namespace MCD
