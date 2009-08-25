#ifndef __MCD_STUDIO_GIZMO_TRANSLATIONGIZMOCOMPONENT__
#define __MCD_STUDIO_GIZMO_TRANSLATIONGIZMOCOMPONENT__

#include "GizmoBaseComponent.h"

//! A Gizmo that controls translation.
class TranslationGizmoComponent : public GizmoBaseComponent
{
public:
	TranslationGizmoComponent(MCD::IResourceManager& resourceManager, sal_in MCD::Entity* hostEntity);
};	// TranslationGizmoComponent

#endif	// __MCD_STUDIO_GIZMO_TRANSLATIONGIZMOCOMPONENT__
