#ifndef __MCD_STUDIO_GIZMO__
#define __MCD_STUDIO_GIZMO__

#include "../../MCD/Core/Entity/Entity.h"

namespace MCD {
class ResourceManager;
}

//! An entity that representing the Gizmo
class Gizmo : public MCD::Entity
{
public:
	Gizmo(MCD::ResourceManager& resourceManager);

	// TODO: Select multiple Entity
	void setSelectedEntity(const MCD::EntityPtr& selectedEntity);

	const MCD::EntityPtr& selectedEntity() const;

	void mouseDown(int x, int y);
	void mouseMove(int x, int y);
	void mouseUp(int x, int y);

	/*!	Which Gizmo entity's component is currently dragging.
		May be the translation arrow, or a translation plane.
	 */
	MCD::ComponentPtr dragging;

protected:
	int mLastMousePosition[2];

	//! A shortcut to the pick detection component.
	MCD::ComponentPtr mPickComponent;
};	// Gizmo

#endif	// __MCD_STUDIO_GIZMO__
