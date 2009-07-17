#ifndef __MCD_BINDING_RENDER__
#define __MCD_BINDING_RENDER__

#include "ShareLib.h"
#include "../Component/Render/CameraComponent.h"
#include "../Component/Render/MeshComponent.h"
#include "../../3Party/jkbind/Declarator.h"

namespace script {

SCRIPT_CLASS_DECLAR_EXPORT(MCD::CameraComponent, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::MeshComponent, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::RenderableComponent, MCD_BINDING_API);
//SCRIPT_CLASS_DECLAR_EXPORT(MCD::Mesh, MCD_BINDING_API);

}	// namespace script
	
#endif	// __MCD_BINDING_RENDER__
