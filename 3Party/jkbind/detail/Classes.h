#ifndef ___SCRIPT_CLASSES___
#define ___SCRIPT_CLASSES___

#include "../VMCore.h"
#include "ClassTraits.h"
#include "ScriptObject.h"
#include "StackHandler.h"
#include <memory.h>
#include <typeinfo>

namespace script {
namespace detail {

class ClassesManager
{
public:
	JKBIND_API static ScriptObject createClass(HSQUIRRELVM v, ScriptObject& root, ClassID classType, const xchar* name, ClassID parentClass);
	JKBIND_API static void createObjectInstanceOnStackPure(HSQUIRRELVM v, ClassID classType, const void* c_this);

	JKBIND_API static void typeofForClass(HSQUIRRELVM v, ScriptObject& classObj, const xchar* typeName);
	JKBIND_API static void disableCloningForClass(HSQUIRRELVM v, ScriptObject& classObj);
	JKBIND_API static void createMemoryControllerSlotForClass(HSQUIRRELVM v, ScriptObject& classObj);
		   
	JKBIND_API static const int MEMORY_CONTROLLER_PARAM = 0x4567C0DE;

	/*!	A callback function for the user to intercept the class registration process
		so that they have a chance to associate a std::type_info with the ClassID.
		By default this function pointer is null, set it to your own function if you
		want to enable the feature "Returns most derived class".

		\sa getClassIDFromObject()

		Sample implementation using std map:
		\code
		struct TypeInfo {
			const std::type_info& typeInfo;
			TypeInfo(const std::type_info& t) : typeInfo(t) {}
			bool operator<(const TypeInfo& rhs) const
			{	return typeInfo.before(rhs.typeInfo) > 0;	}
		};	// TypeInfo
		typedef std::map<TypeInfo, ClassID> TypeMap;
		static TypeMap typeMap;

		void associateClassID(const std::type_info& typeInfo, script::ClassID classID)
		{	typeMap[typeInfo] = classID;	}

		ClassID getClassIDFromObject(const MyPolymorphicBaseClass* obj, ClassID original) {
			TypeMap::const_iterator i = typeMap.find(typeid(*obj));
			if(i != typeMap.end())
				return i->second;
			return original;
		}
		\endcode
	 */
	typedef void (*AssociateClassID)(const std::type_info& typeInfo, ClassID classID);
	JKBIND_API static AssociateClassID associateClassID();
	JKBIND_API static void setAssociateClassID(AssociateClassID);

private:
	static ScriptObject _findClass(HSQUIRRELVM v, ClassID classType);
};

ScriptObject createTable(HSQUIRRELVM v);

ScriptObject createArray(HSQUIRRELVM v, int size);

ScriptObject createFunction(HSQUIRRELVM v, ScriptObject& root, SQFUNCTION func, const SQChar* scriptFuncName, const xchar* typeMask=0);

//
// thiscall Functions
//

template<typename Func>
void pushFunctionPointer(HSQUIRRELVM v, Func func)
{
	SQUserPointer data = sq_newuserdata(v, sizeof(func));
	memcpy(data, &func, sizeof(Func));
}

template<typename Func>
Func getFunctionPointer(HSQUIRRELVM v, int ndx)
{
	StackHandler sh(v);
	return *(Func*)sh.getUserData(ndx);
}

//
// Static functions
//

template<typename Func>
void pushStaticFunctionPointer(HSQUIRRELVM v, Func func)
{
	sq_pushuserpointer(v, (void*)func);
}

template<typename Func>
Func getStaticFunctionPointer(HSQUIRRELVM v, int ndx)
{
	StackHandler sh(v);
	return (Func)sh.getUserPointer(ndx);
}

//
// Events objects
//

template<typename EventType>
void pushEventPointer(HSQUIRRELVM v, EventType ev)
{
	SQUserPointer data = sq_newuserdata(v, sizeof(ev));
	memcpy(data, &ev, sizeof(EventType));
}

template<typename EventType>
EventType getEventPointer(HSQUIRRELVM v, int ndx)
{
	StackHandler sh(v);
	return *(EventType*)sh.getUserData(ndx);
}

//
// Fields
//

template<typename Field>
void pushFieldPointer(HSQUIRRELVM v, Field field)
{
	SQUserPointer data = sq_newuserdata(v, sizeof(field));
	memcpy(data, &field, sizeof(Field));
}

template<typename Field>
Field getFieldPointer(HSQUIRRELVM v, int ndx)
{
	StackHandler sh(v);
	return *(Field*)sh.getUserData(ndx);
}

}	//namespace detail
}	//namespace script

#endif//___SCRIPT_CLASSES___
