#ifndef __MCD_CORE_BINDING_COREBINDING__
#define __MCD_CORE_BINDING_COREBINDING__

#include "ClassTraits.h"
#include "../Math/Mat44.h"
#include "../Math/Vec2.h"
#include "../Math/Vec3.h"
#include "../ShareLib.h"

typedef ssize_t SQInteger;
typedef SQInteger SQRESULT;
typedef void* SQUserPointer;

namespace MCD {

class Component;
class Entity;
class InputComponent;
class Resource;
class ResourceManager;
class ScriptComponent;
class Timer;

namespace Binding {

MCD_CORE_API void registerCoreBinding(VMCore& vm);

// System
SCRIPT_CLASS_DECLAR_EXPORT(Resource, MCD_CORE_API);
SCRIPT_CLASS_CUSTOM_EXPORT(Resource, MCD_CORE_API);
MCD_CORE_API SQRESULT setInstanceUp(HSQUIRRELVM, SQInteger, Resource* p, Resource*);
template<typename T> SQRESULT setInstanceUp(HSQUIRRELVM v, SQInteger i, T* p, Resource*) { return setInstanceUp(v, i, (Resource*)p, (Resource*)p); }

SCRIPT_CLASS_DECLAR_EXPORT(ResourceManager, MCD_CORE_API);
SCRIPT_CLASS_DECLAR_EXPORT(Timer, MCD_CORE_API);

// Math
SCRIPT_CLASS_DECLAR_EXPORT(Mat44f, MCD_CORE_API);
SCRIPT_CLASS_DECLAR_EXPORT(Vec2f, MCD_CORE_API);
SCRIPT_CLASS_DECLAR_EXPORT(Vec3f, MCD_CORE_API);

// Entity
SCRIPT_CLASS_DECLAR_EXPORT(Entity, MCD_CORE_API);
SCRIPT_CLASS_CUSTOM_EXPORT(Entity, MCD_CORE_API);
MCD_CORE_API SQRESULT setInstanceUp(HSQUIRRELVM, SQInteger, Entity* p, Entity*);

// Component
SCRIPT_CLASS_DECLAR_EXPORT(Component, MCD_CORE_API);
SCRIPT_CLASS_CUSTOM_EXPORT(Component, MCD_CORE_API);
MCD_CORE_API SQRESULT setInstanceUp(HSQUIRRELVM, SQInteger, Component* p, Component*);
template<typename T> SQRESULT setInstanceUp(HSQUIRRELVM v, SQInteger i, T* p, Component*) { return setInstanceUp(v, i, (Component*)p, (Component*)p); }

// ScriptComponent
SCRIPT_CLASS_DECLAR_EXPORT(ScriptComponent, MCD_CORE_API);

// Input
SCRIPT_CLASS_DECLAR_EXPORT(InputComponent, MCD_CORE_API);

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_CORE_BINDING_COREBINDING__
