#include "stdafx.h"
#include "Gizmo.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Components/MeshComponent.h"
#include "../../MCD/Render/Components/PickComponent.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/System/Utility.h"
#include "../../3Party/glew/glew.h"
#include <iostream>

using namespace MCD;

namespace {

class MyMeshComponent : public MeshComponent
{
public:
	sal_override void render()
	{
		glColor3fv(color.rawPointer());
		MeshComponent::render();
	}

	ColorRGBAf color;
};

class FollowTransformComponent : public BehaviourComponent
{
public:
	sal_override void update()
	{
		Gizmo* gizmo = polymorphic_downcast<Gizmo*>(entity());
		if(!gizmo || !selectedEntity.get())
			return;
		gizmo->localTransform = selectedEntity->localTransform;
	}

	EntityPtr selectedEntity;
};

class MyPickComponent : public PickComponent
{
public:
	sal_override void update()
	{
//		entityToPick = entity()->parent()->parent()->firstChild()->nextSlibing()->nextSlibing();
		PickComponent::update();

		// Handle picking result
		for(size_t i=0; i<hitCount(); ++i) {
			EntityPtr e = hitAtIndex(i);
			if(!e.get())
				continue;
			std::wcout << e->name << std::endl;
			break;	// We only pick the Entity that nearest to the camera
		}
		clearResult();
	}
};

}

Gizmo::Gizmo(ResourceManager& resourceManager)
{
	enabled = false;

	ModelPtr model = dynamic_cast<Model*>(resourceManager.load(L"Arrow.3ds", true).get());
	dynamic_cast<DefaultResourceManager&>(resourceManager).processLoadingEvents();

	addComponent(new FollowTransformComponent);

	if(model->mMeshes.empty())	// Fail to load the Gizmo model
		return;

	MeshPtr mesh = model->mMeshes.front().mesh;

	{	// Add an entity for picking
		Entity* e = new Entity();
		e->name = L"Gizmo picker";
		e->link(this);
		MyPickComponent* c = new MyPickComponent;
		c->entityToPick = this;
		e->addComponent(c);
		e->enabled = true;
	}

	// Add child entities
	{	Entity* e = new Entity();
		e->name = L"X arrow";
		e->link(this);
		MyMeshComponent* c = new MyMeshComponent;
		c->color = ColorRGBAf(1, 0, 0, 1);
		c->mesh = mesh;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(0, 0, -Mathf::cPiOver2())) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = L"Y arrow";
		e->link(this);
		MyMeshComponent* c = new MyMeshComponent;
		c->color = ColorRGBAf(0, 1, 0, 1);
		c->mesh = mesh;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
	}

	{	Entity* e = new Entity();
		e->name = L"Z arrow";
		e->link(this);
		MyMeshComponent* c = new MyMeshComponent;
		c->color = ColorRGBAf(0, 0, 1, 1);
		c->mesh = mesh;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(Mathf::cPiOver2(), 0, 0)) * e->localTransform;
	}
}

void Gizmo::setSelectedEntity(const EntityPtr& selectedEntity)
{
	enabled = (selectedEntity.get() != nullptr);

	FollowTransformComponent* component = polymorphic_downcast<FollowTransformComponent*>(
		this->findComponent(typeid(BehaviourComponent)));
	component->selectedEntity = selectedEntity;
}

const EntityPtr& Gizmo::selectedEntity() const
{
	FollowTransformComponent* component = findComponent<FollowTransformComponent>(typeid(BehaviourComponent));
	return component->selectedEntity;
}

void Gizmo::mouseDown(uint x, uint y)
{
}

void Gizmo::mouseMove(uint x, uint y)
{
	Entity* e = this->findEntityInChildren(L"Gizmo picker");
	if(!e)
		return;
	MyPickComponent* component = e->findComponent<MyPickComponent>(typeid(BehaviourComponent));
}

void Gizmo::mouseUp(uint x, uint y)
{
}
