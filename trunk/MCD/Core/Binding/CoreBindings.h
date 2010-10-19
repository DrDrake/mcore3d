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

class Component;
class Entity;
class InputComponent;

namespace Binding {

MCD_CORE_API void registerCoreBinding(VMCore& vm);

// Math
SCRIPT_CLASS_DECLAR_EXPORT(Mat44f, MCD_CORE_API);
SCRIPT_CLASS_DECLAR_EXPORT(Vec2f, MCD_CORE_API);
SCRIPT_CLASS_DECLAR_EXPORT(Vec3f, MCD_CORE_API);

// Entity
SCRIPT_CLASS_DECLAR_EXPORT(Entity, MCD_CORE_API);
MCD_CORE_API void push(HSQUIRRELVM v, Entity*);
MCD_CORE_API SQRESULT setInstanceUp(HSQUIRRELVM, SQInteger, Entity*, Entity*);
MCD_CORE_API void destroy(Entity*, Entity*);
inline void push(HSQUIRRELVM v, Entity& e) { push(v, &e); }

SCRIPT_CLASS_DECLAR_EXPORT(Component, MCD_CORE_API);
MCD_CORE_API void push(HSQUIRRELVM, Component*);
MCD_CORE_API SQRESULT setInstanceUp(HSQUIRRELVM, SQInteger, Component*, Component*);
MCD_CORE_API void destroy(Component*, Component*);
template<typename T> SQRESULT setInstanceUp(HSQUIRRELVM v, SQInteger i, Component* c, T*) { return setInstanceUp(v, i, c, c); }
template<typename T> void destroy(Component* c, T*) { destroy(c, c); }
inline void push(HSQUIRRELVM v, Component& c) { push(v, &c); }

// Input
SCRIPT_CLASS_DECLAR_EXPORT(InputComponent, MCD_CORE_API);

}	// namespace Binding

}	// namespace MCD

#endif	// __MCD_CORE_BINDING_COREBINDING__
