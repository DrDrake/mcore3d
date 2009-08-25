#ifndef __MCD_STUDIO_GIZMO_ROTATIONGIZMOCOMPONENT__
#define __MCD_STUDIO_GIZMO_ROTATIONGIZMOCOMPONENT__

#include "GizmoBaseComponent.h"

//! A Gizmo that controls rotation.
class RotationGizmoComponent : public GizmoBaseComponent
{
public:
	RotationGizmoComponent(MCD::IResourceManager& resourceManager, sal_in MCD::Entity* hostEntity);
};	// RotationGizmoComponent

#endif	// __MCD_STUDIO_GIZMO_ROTATIONGIZMOCOMPONENT__
