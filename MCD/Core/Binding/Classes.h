#ifndef __MCD_CORE_BINDING_CLASSES__
#define __MCD_CORE_BINDING_CLASSES__

#include "ScriptObject.h"
#include <memory.h>
#include <typeinfo>

namespace MCD {
namespace Binding {

typedef void* ClassID;

class MCD_CORE_API ClassesManager
{
public:
	/// \param ns stands for namespace
	static ScriptObject createClass(HSQUIRRELVM v, ScriptObject& ns, ClassID classType, const char* name, ClassID parentClass);

	/// Creates a Squirrel object instance of type classId, and left it on the top of the stack.
	static void createObjectInstanceOnStack(HSQUIRRELVM v, ClassID classId, const void* objPtr);

	/// Register the type name to return when invoking the Squirrel meta function _typeof().
	static void registerTypeOf(HSQUIRRELVM v, ScriptObject& classObj, const char* typeName);

	/// Add the weakref function (apparently not provided by default)
	static void registerWeakRef(HSQUIRRELVM v, ScriptObject& classObj);

	/// Disable clonning of the specificed class.
	static void disableCloning(HSQUIRRELVM v, ScriptObject& classObj);

	/// Reserve a table slot for registering memory cleanup function later on.
	static void createMemoryControllerSlot(HSQUIRRELVM v, ScriptObject& classObj);

	static const int MEMORY_CONTROLLER_PARAM = 0x4567C0DE;

	static void registerGetSetTable(HSQUIRRELVM v, ScriptObject& classObj);

	static void setClassIdForRtti(const std::type_info& typeInfo, ClassID classID);
	static ClassID getClassIdFromRtti(const std::type_info& typeInfo, ClassID fallback=nullptr);

private:
	/// Returns the class table by a given ClassID.
	static ScriptObject findClass(HSQUIRRELVM v, ClassID classType);
};	// ClassesManager

// Member functions
template<typename Func>
void pushFunctionPointer(HSQUIRRELVM v, Func func)
{
	SQUserPointer data = sq_newuserdata(v, sizeof(func));
	memcpy(data, &func, sizeof(Func));
}

template<typename Func>
Func getFunctionPointer(HSQUIRRELVM v, int idx)
{
	SQUserPointer up = nullptr;
	MCD_VERIFY(SQ_SUCCEEDED(sq_getuserdata(v, idx, &up, nullptr)));
	return *(Func*)up;
}

// Static functions
template<typename Func>
void pushStaticFunctionPointer(HSQUIRRELVM v, Func func)
{
	sq_pushuserpointer(v, (void*)func);
}

template<typename Func>
Func getStaticFunctionPointer(HSQUIRRELVM v, int idx)
{
	StackHandler sh(v);
	return (Func)sh.getUserPointer(idx);
}

// Events objects
template<typename EventType>
void pushEventPointer(HSQUIRRELVM v, EventType ev)
{
	SQUserPointer data = sq_newuserdata(v, sizeof(ev));
	memcpy(data, &ev, sizeof(EventType));
}

template<typename EventType>
EventType getEventPointer(HSQUIRRELVM v, int idx)
{
	SQUserPointer tag;
	SQUserPointer up;
	MCD_VERIFY(SQ_SUCCEEDED(sq_getuserdata(v, idx, &up, &tag)));
	MCD_ASSERT(ClassTraits<EventType>::classID() == tag); (void)tag;
	return *(EventType*)up;
}

// Fields
template<typename Field>
void pushFieldPointer(HSQUIRRELVM v, Field field)
{
	SQUserPointer data = sq_newuserdata(v, sizeof(field));
	memcpy(data, &field, sizeof(Field));
}

template<typename Field>
Field getFieldPointer(HSQUIRRELVM v, int idx)
{
	SQUserPointer tag;
	SQUserPointer up;
	MCD_VERIFY(SQ_SUCCEEDED(sq_getuserdata(v, idx, &up, &tag)));
	return *(Field*)up;
}

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_CORE_BINDING_CLASSES__
