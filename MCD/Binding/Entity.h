#ifndef __MCD_BINDING_ENTITY__
#define __MCD_BINDING_ENTITY__

#include "ShareLib.h"
#include "../Core/Entity/Entity.h"
#include "../../3Party/jkbind/Declarator.h"

namespace script {

SCRIPT_CLASS_DECLAR_EXPORT(MCD::Component, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::Entity, MCD_BINDING_API);

namespace types {

MCD_BINDING_API void addHandleToObject(HSQUIRRELVM v, MCD::Component* obj, int idx);

MCD_BINDING_API bool pushHandleFromObject(HSQUIRRELVM v, MCD::Component* obj);

MCD_BINDING_API void addHandleToObject(HSQUIRRELVM v, MCD::Entity* obj, int idx);

MCD_BINDING_API bool pushHandleFromObject(HSQUIRRELVM v, MCD::Entity* obj);

}	// namespace types

}	// namespace script

#endif	// __MCD_BINDING_ENTITY__
