#ifndef __MCD_CORE_BINDING_VMCORE__
#define __MCD_CORE_BINDING_VMCORE__

#include "../ShareLib.h"
#include "../../../3Party/squirrel/squirrel.h"
#include <iosfwd>

namespace MCD {
namespace Binding {

class MCD_CORE_API VMCore
{
public:
	explicit VMCore(int initialStackSize=256);

	~VMCore();

	enum State
	{
		OPENING,
		ACTIVE,
		CLOSING
	};

	State state() const { return mState; }

	HSQUIRRELVM getVM() const;

// Operations
	void collectGarbage();

	/// Compile source code as a closure and push on to the stack
	sal_checkreturn bool loadScript(
		const char* script,
		int lenInByte=-1,
		const char* scriptName="unnamed"
	);

	/// Load source/byte code as a closure and push on to the stack
	sal_checkreturn bool loadScript(
		std::istream& is,
		int sizeInByte=-1,
		const char* scriptName="unnamed"
	);

	/// Run script from an UTF-8 encoded source sring
	sal_checkreturn bool runScript(
		const char* script,
		int lenInByte=-1,
		bool retVal=false,
		bool leftClouseOnStack=false,
		const char* scriptName="unnamed"
	);

	/// Run script from an input stream with optional specific byte length, the source can be text or compiled byte code
	/// Make sure the stream is in binary mode for loading byte code
	sal_checkreturn bool runScript(
		std::istream& is,
		int sizeInByte=-1,
		bool retVal=false,
		bool leftClouseOnStack=false,
		const char* scriptName="unnamed"
	);

	/// Save the closure at the top of the stack to an output stream
	/// Make sure the stream is in binary mode for writing byte code
	sal_checkreturn bool saveByteCode(
		std::ostream& os,
		bool leftClouseOnStack=false
	);

// Static helpers
	static sal_checkreturn bool loadScript(
		HSQUIRRELVM v,
		const char* script,
		int lenInByte=-1,
		const char* scriptName="unnamed"
	);

	static sal_checkreturn bool loadScript(
		HSQUIRRELVM v,
		std::istream& is,
		int sizeInByte=-1,
		const char* scriptName="unnamed"
	);

	static sal_checkreturn bool runScript(
		HSQUIRRELVM v,
		const char* script,
		int lenInByte=-1,
		bool retVal=false,
		bool leftClouseOnStack=false,
		const char* scriptName="unnamed"
	);

	static sal_checkreturn bool runScript(
		HSQUIRRELVM v,
		std::istream& is,
		int sizeInByte=-1,
		bool retVal=false,
		bool leftClouseOnStack=false,
		const char* scriptName="unnamed"
	);

	static sal_checkreturn bool saveByteCode(
		HSQUIRRELVM v,
		std::ostream& os,
		bool leftClouseOnStack=false
	);

	static sal_checkreturn bool call(HSQUIRRELVM v, bool retVal);

	static bool printError(HSQUIRRELVM v);	// Always return false

protected:
	State mState;
	HSQUIRRELVM mSqvm;
	HSQOBJECT mClassesTable;

	friend class ClassesManager;
};	// VMCore

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_CORE_BINDING_VMCORE__
