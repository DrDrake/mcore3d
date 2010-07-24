#ifndef __MCD_STUDIO_GIZMO_ROTATIONGIZMOCOMPONENT__
#define __MCD_STUDIO_GIZMO_ROTATIONGIZMOCOMPONENT__

#include "GizmoBaseComponent.h"
#include "../../../MCD/Core/Entity/InputComponent.h"

//! A Gizmo that controls rotation.
class RotationGizmoComponent : public GizmoBaseComponent
{
public:
	RotationGizmoComponent(MCD::IResourceManager& resourceManager, sal_in MCD::Entity* hostEntity, sal_in_opt MCD::InputComponent* inputComponent);
};	// RotationGizmoComponent

#endif	// __MCD_STUDIO_GIZMO_ROTATIONGIZMOCOMPONENT__
