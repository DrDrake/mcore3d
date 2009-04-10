#include "stdafx.h"
#include "Gizmo.h"
#include "RotationGizmoComponent.h"
#include "ScaleGizmoComponent.h"
#include "TranslationGizmoComponent.h"
#include "../../../MCD/Core/Entity/BehaviourComponent.h"

using namespace MCD;

namespace {

class FollowTransformComponent : public BehaviourComponent
{
public:
	sal_override void update()
	{
		Entity* e = entity();
		MCD_ASSUME(e);
		if(!selectedEntity)
			return;
		e->localTransform = selectedEntity->worldTransform();
	}

	EntityPtr selectedEntity;
};	// FollowTransformComponent

}	// namespace

Gizmo::Gizmo(ResourceManager& resourceManager)
{
	addComponent(new FollowTransformComponent);

	{	Entity* e = new Entity();
		e->name = L"Translation Gizmo";
		e->asChildOf(this);

		translationGizmo = new TranslationGizmoComponent(resourceManager, e);
		e->addComponent(translationGizmo.get());
	}

	{	Entity* e = new Entity();
		e->name = L"Rotation Gizmo";
		e->asChildOf(this);

		rotationGizmo = new RotationGizmoComponent(resourceManager, e);
		e->addComponent(rotationGizmo.get());
	}

	{	Entity* e = new Entity();
		e->name = L"Scale Gizmo";
		e->asChildOf(this);
		scaleGizmo = new ScaleGizmoComponent(resourceManager, e);
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
		gizmo->mouseDown(x, y, selectedEntity()->localTransform);
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
		gizmo->mouseMove(x, y, selectedEntity()->localTransform);
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
