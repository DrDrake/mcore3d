#ifndef __MCD_STUDIO_GIZMO_TRANSLATIONGIZMOCOMPONENT__
#define __MCD_STUDIO_GIZMO_TRANSLATIONGIZMOCOMPONENT__

#include "GizmoBaseComponent.h"
#include "../../../MCD/Component/Input/InputComponent.h"

/*!	A Gizmo that controls translation.
	By default TranslationGizmoComponent use Global as the reference frame.
 */
class TranslationGizmoComponent : public GizmoBaseComponent
{
public:
	TranslationGizmoComponent(MCD::IResourceManager& resourceManager, sal_in MCD::Entity* hostEntity, sal_in_opt MCD::InputComponent* inputComponent);
};	// TranslationGizmoComponent

#endif	// __MCD_STUDIO_GIZMO_TRANSLATIONGIZMOCOMPONENT__
