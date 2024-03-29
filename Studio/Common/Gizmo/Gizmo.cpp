#include "stdafx.h"
#include "Gizmo.h"
#include "RotationGizmoComponent.h"
#include "ScaleGizmoComponent.h"
#include "TranslationGizmoComponent.h"
#include "../../../MCD/Core/Entity/BehaviourComponent.h"

using namespace MCD;

namespace {

//! Make the entity containing this component follow the transform of a selected entity.
class FollowTransformComponent : public BehaviourComponent
{
public:
	FollowTransformComponent() : ignoreRotation(false) {}
	sal_override void update(float)
	{
		Entity* e = entity();
		MCD_ASSUME(e);
		if(!selectedEntity)
			return;
		e->localTransform = selectedEntity->worldTransform();

		// Undo any scaling
		e->localTransform.setScale(Vec3f(1));

		if(ignoreRotation)
			e->localTransform.setMat33(Mat33f::cIdentity);
	}

	EntityPtr selectedEntity;
	bool ignoreRotation;
};	// FollowTransformComponent

//!	Adjust the scaling transform so that all child entities will have a constance size on screen.
class FixedScreenSizeComponent : public BehaviourComponent
{
public:
	sal_override void update(float)
	{
		Entity* e = entity();
		MCD_ASSUME(e);

		// Get only the translation part of the targeting entity.
		Vec3f p1 = e->worldTransform().translation();

		// Transform it to camera space.
		if(camera)
			camera->worldTransform().inverse().transformPoint(p1);

		Vec3f p2 = p1 + Vec3f::c100;

		// Test an unit lenght vector to see it's projected lenght on 2D screen.
		p1 = projectToScreenNoModelView(p1);
		p2 = projectToScreenNoModelView(p2);

		if(p1 == p2)
			return;

		float len = (p2 - p1).length();

		const float cPixelSizeForUnitLength = 100.0f;
		e->localTransform.setScale(Vec3f(cPixelSizeForUnitLength / len));
	}

	//! To get the camera's world transform.
	EntityPtr camera;
};	// FixedScreenSizeComponent

}	// namespace

Gizmo::Gizmo(IResourceManager& resourceManager, InputComponent* inputComponent)
{
	addComponent(new FollowTransformComponent);

	Entity* sizeFixer = new Entity();
	sizeFixer->name = "Fixed screenSize";
	sizeFixer->addComponent(new FixedScreenSizeComponent);
	sizeFixer->asChildOf(this);

	{	Entity* e = new Entity();
		e->name = "Translation Gizmo";
		e->asChildOf(sizeFixer);

		translationGizmo = new TranslationGizmoComponent(resourceManager, e, inputComponent);
		e->addComponent(translationGizmo.get());
	}

	{	Entity* e = new Entity();
		e->name = "Rotation Gizmo";
		e->asChildOf(sizeFixer);

		rotationGizmo = new RotationGizmoComponent(resourceManager, e, inputComponent);
		e->addComponent(rotationGizmo.get());
	}

	{	Entity* e = new Entity();
		e->name = "Scale Gizmo";
		e->asChildOf(sizeFixer);
		scaleGizmo = new ScaleGizmoComponent(resourceManager, e, inputComponent);
		e->addComponent(scaleGizmo.get());
	}

	// Set translationGizmo as the default one.
	setActiveGizmo(translationGizmo);
}

void Gizmo::mouseDown(int x, int y)
{
	if(!selectedEntity())
		return;
	for(ComponentPreorderIterator itr(this); !itr.ended(); itr.next()) {
		Entity* e = itr->entity();
		MCD_ASSUME(e);
		if(!e->enabled)
			continue;
		GizmoBaseComponent* gizmo = dynamic_cast<GizmoBaseComponent*>(itr.current());
		if(!gizmo)
			continue;
		MCD_ASSERT(selectedEntity());
		gizmo->mouseDown(x, y, *selectedEntity());
	}
}

void Gizmo::mouseMove(int x, int y)
{
	if(!selectedEntity())
		return;
	for(ComponentPreorderIterator itr(this); !itr.ended(); itr.next()) {
		Entity* e = itr->entity();
		MCD_ASSUME(e);
		if(!e->enabled)
			continue;
		GizmoBaseComponent* gizmo = dynamic_cast<GizmoBaseComponent*>(itr.current());
		if(!gizmo)
			continue;
		gizmo->mouseMove(x, y, *selectedEntity());
	}
}

void Gizmo::mouseUp(int x, int y)
{
	if(!selectedEntity())
		return;
	for(ComponentPreorderIterator itr(this); !itr.ended(); itr.next()) {
		Entity* e = itr->entity();
		MCD_ASSUME(e);
		if(!e->enabled)
			continue;
		GizmoBaseComponent* gizmo = dynamic_cast<GizmoBaseComponent*>(itr.current());
		if(!gizmo)
			continue;
		gizmo->mouseUp(x, y);
	}
}

void Gizmo::setActiveGizmo(const ComponentPtr& gizmo)
{
	// Force a mouse up when changing the active Gizmo
	mouseUp(0, 0);

	if(translationGizmo) translationGizmo->entity()->enabled = false;
	if(rotationGizmo) rotationGizmo->entity()->enabled = false;
	if(scaleGizmo) scaleGizmo->entity()->enabled = false;
	if(gizmo) gizmo->entity()->enabled = true;
	mActiveGizmo = gizmo;

	// Ignore the roation part of the selected entity, if the active gizmo is translation gizmo
	FollowTransformComponent* component = findComponent<FollowTransformComponent>(typeid(BehaviourComponent));
	component->ignoreRotation = (gizmo == translationGizmo);
}

Component* Gizmo::activeGizmo() const
{
	return mActiveGizmo.get();
}

void Gizmo::setSelectedEntity(Entity* entity)
{
	// Force a mouse up when selected entity is changed
	mouseUp(0, 0);

	enabled = (entity != nullptr);

	FollowTransformComponent* component = findComponent<FollowTransformComponent>(typeid(BehaviourComponent));
	MCD_ASSUME(component);
	component->selectedEntity = entity;
}

Entity* Gizmo::selectedEntity() const
{
	FollowTransformComponent* component = findComponent<FollowTransformComponent>(typeid(BehaviourComponent));
	MCD_ASSUME(component);
	return component->selectedEntity.get();
}

bool Gizmo::isDragging() const
{
	GizmoBaseComponent* gizmo = dynamic_cast<GizmoBaseComponent*>(mActiveGizmo.get());
	if(!gizmo)
		return false;
	return gizmo->dragging != nullptr;
}

void Gizmo::setCamrea(const EntityPtr& camera)
{
	FixedScreenSizeComponent* component = firstChild()->findComponent<FixedScreenSizeComponent>(typeid(BehaviourComponent));
	MCD_ASSUME(component);
	component->camera = camera;
}
