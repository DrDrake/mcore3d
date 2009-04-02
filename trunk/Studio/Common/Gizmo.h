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

	/*!	Which Gizmo entity is currently draggin.
		May be the translation arrow, or a rotation axis.
	 */
	Entity* draggingEntity;

protected:
	int mLastMousePosition[2];

	//! Intermediate varible to store draggingEntity's mesh component
	MCD::ComponentPtr mDraggingMeshComponent;

	//! A shortcut to the pick detection component.
	MCD::ComponentPtr mPickComponent;
};	// Gizmo

#endif	// __MCD_STUDIO_GIZMO__
