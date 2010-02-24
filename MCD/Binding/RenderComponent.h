#ifndef __MCD_BINDING_RENDERCOMPONENT__
#define __MCD_BINDING_RENDERCOMPONENT__

#include "ShareLib.h"
#include "../../3Party/jkbind/Declarator.h"

namespace MCD {

class AnimationComponent;
class AnimationUpdaterComponent;
class CameraComponent;
class MeshComponent;
class PickComponent;
class RenderableComponent;

}	// namespace MCD

namespace script {

SCRIPT_CLASS_DECLAR_EXPORT(MCD::AnimationComponent, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::AnimationUpdaterComponent, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::CameraComponent, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::MeshComponent, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::PickComponent, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::RenderableComponent, MCD_BINDING_API);

}	// namespace script
	
#endif	// __MCD_BINDING_RENDERCOMPONENT__
