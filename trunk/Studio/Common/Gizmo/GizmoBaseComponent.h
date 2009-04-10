#ifndef __MCD_STUDIO_GIZMOBASECOMPONENT__
#define __MCD_STUDIO_GIZMOBASECOMPONENT__

#include "../../../MCD/Core/Entity/Component.h"
#include "../../../MCD/Core/Math/Mat44.h"
#include "../../../MCD/Core/Math/Ray.h"
#include "../../../MCD/Core/Math/Vec2.h"
#include "../../../MCD/Render/Components/MeshComponent.h"
#include "../../../MCD/Render/Color.h"

namespace MCD {
class ResourceManager;
}

MCD::Vec3f unProject(const MCD::Vec3f& p);

MCD::Vec3f projectToScreen(const MCD::Vec3f& p);

MCD::Ray createPickingRay(int x, int y);

class MyMeshComponent : public MCD::MeshComponent
{
public:
	typedef MCD::Vec2<int> Vec2i;

	MyMeshComponent(const MCD::ColorRGBAf& c);

	sal_override void render();

	/*! 
		\param oldPos By default, it will be the mouse position at the time of mouse down.
			But overrided function can change this value so that it become the last mouse position.
		\param newPos The lastest mouse position.
	 */
	virtual void mouseMove(Vec2i& oldPos, const Vec2i& newPos,
		const MCD::Mat44f& oldTransform, MCD::Mat44f& transform) const = 0;

	MCD::ColorRGBAf color;
	const MCD::ColorRGBAf defaultColor;
	bool backToDefaultColor;
};	// MyMeshComponent

//! An entity that representing the Gizmo
class GizmoBaseComponent : public MCD::Component
{
public:
	GizmoBaseComponent(sal_in MCD::Entity* hostEntity);

	sal_override const std::type_info& familyType() const {
		return typeid(GizmoBaseComponent);
	}

	virtual void mouseDown(int x, int y, MCD::Mat44f& transform);

	virtual void mouseMove(int x, int y, MCD::Mat44f& transform);

	virtual void mouseUp(int x, int y);

	MCD::ComponentPtr dragging;

protected:
	MCD::Mat44f mBackupMatrix;

	MyMeshComponent::Vec2i mOldMousePosition;

	//! A shortcut to the pick detection component.
	MCD::ComponentPtr mPickComponent;
};	// RotationGizmoComponent

#endif	// __MCD_STUDIO_GIZMOBASECOMPONENT__
