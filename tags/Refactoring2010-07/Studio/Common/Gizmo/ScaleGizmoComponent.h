#ifndef __MCD_STUDIO_GIZMO_SCALEGIZMOCOMPONENT__
#define __MCD_STUDIO_GIZMO_SCALEGIZMOCOMPONENT__

#include "GizmoBaseComponent.h"
#include "../../../MCD/Core/Entity/InputComponent.h"

//! A Gizmo that controls scale.
class ScaleGizmoComponent : public GizmoBaseComponent
{
public:
	ScaleGizmoComponent(MCD::IResourceManager& resourceManager, sal_in MCD::Entity* hostEntity, sal_in_opt MCD::InputComponent* inputComponent);
};	// ScaleGizmoComponent

#endif	// __MCD_STUDIO_GIZMO_SCALEGIZMOCOMPONENT__
