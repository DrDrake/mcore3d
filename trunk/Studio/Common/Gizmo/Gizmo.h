#ifndef __MCD_STUDIO_GIZMO_GIZMO__
#define __MCD_STUDIO_GIZMO_GIZMO__

#include "../../../MCD/Core/Entity/Entity.h"

namespace MCD {
class ResourceManager;
}

/*!	An entity that representing the Gizmo.
	It centralized the 3 kinds of Gizmo (translation, rotation and scale).

	To render the Gizmo on top of all other objects, RenderableComponent::render() should
	be called separately from others.
 */
class Gizmo : public MCD::Entity
{
public:
	Gizmo(MCD::ResourceManager& resourceManager);

// Operation
	void mouseDown(int x, int y);
	void mouseMove(int x, int y);
	void mouseUp(int x, int y);

// Attrubute
	void setActiveGizmo(const MCD::ComponentPtr& gizmo);

	sal_maybenull MCD::Component* activeGizmo() const;

	void setSelectedEntity(sal_maybenull MCD::Entity* entity);

	sal_maybenull MCD::Entity* selectedEntity() const;

	bool isDragging() const;

	MCD::ComponentPtr translationGizmo;
	MCD::ComponentPtr rotationGizmo;
	MCD::ComponentPtr scaleGizmo;

protected:
	MCD::ComponentPtr mActiveGizmo;
};	// Gizmo

#endif	// __MCD_STUDIO_GIZMO_GIZMO__
