#ifndef __MCD_CORE_BINDING_COREBINDING__
#define __MCD_CORE_BINDING_COREBINDING__

#include "ClassTraits.h"
#include "../Math/Mat44.h"
#include "../Math/Vec2.h"
#include "../Math/Vec3.h"
#include "../ShareLib.h"

typedef int SQInteger;
typedef int SQRESULT;
typedef void* SQUserPointer;

namespace MCD {

// Entity
class Entity;

namespace Binding {

MCD_CORE_API void registerCoreBinding(VMCore& vm);

// Math
SCRIPT_CLASS_DECLAR_EXPORT(Mat44f, MCD_CORE_API);
SCRIPT_CLASS_DECLAR_EXPORT(Vec2f, MCD_CORE_API);
SCRIPT_CLASS_DECLAR_EXPORT(Vec3f, MCD_CORE_API);

// Entity
SCRIPT_CLASS_DECLAR_EXPORT(Entity, MCD_CORE_API);
MCD_CORE_API void push(HSQUIRRELVM v, Entity* obj);
MCD_CORE_API SQRESULT setInstanceUp(HSQUIRRELVM v, SQInteger idx, Entity* dummy, Entity* instance);
MCD_CORE_API SQRESULT fromInstanceUp(HSQUIRRELVM v, SQInteger idx, Entity* dummy, Entity*& instance, SQUserPointer typetag);
MCD_CORE_API void destroy(Entity*, Entity*);

}	// namespace Binding

}	// namespace MCD

#endif	// __MCD_CORE_BINDING_COREBINDING__
