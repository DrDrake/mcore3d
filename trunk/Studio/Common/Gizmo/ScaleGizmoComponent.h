#ifndef __MCD_STUDIO_GIZMO_SCALEGIZMOCOMPONENT__
#define __MCD_STUDIO_GIZMO_SCALEGIZMOCOMPONENT__

#include "GizmoBaseComponent.h"

//! A Gizmo that controls scale.
class ScaleGizmoComponent : public GizmoBaseComponent
{
public:
	ScaleGizmoComponent(MCD::IResourceManager& resourceManager, sal_in MCD::Entity* hostEntity);
};	// ScaleGizmoComponent

#endif	// __MCD_STUDIO_GIZMO_SCALEGIZMOCOMPONENT__
