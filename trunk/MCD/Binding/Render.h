#ifndef __MCD_BINDING_RENDER__
#define __MCD_BINDING_RENDER__

#include "ShareLib.h"
#include "../../3Party/jkbind/Declarator.h"

namespace MCD {

class Effect;
class Mesh;

}	// namespace MCD

namespace script {

SCRIPT_CLASS_DECLAR_EXPORT(MCD::Effect, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::Mesh, MCD_BINDING_API);

}	// namespace script
	
#endif	// __MCD_BINDING_RENDER__
