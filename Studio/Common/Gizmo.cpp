#include "stdafx.h"
#include "Gizmo.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Components/MeshComponent.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/System/Utility.h"
#include "../../3Party/glew/glew.h"

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

}

Gizmo::Gizmo(ResourceManager& resourceManager)
{
	enabled = false;

	ModelPtr model = dynamic_cast<Model*>(resourceManager.load(L"Arrow.3ds", true).get());
	dynamic_cast<DefaultResourceManager&>(resourceManager).processLoadingEvents();
	MeshPtr mesh = model->mMeshes.front().mesh;

	addComponent(new FollowTransformComponent);

//	this->localTransform.setScale(Vec3f(0.02f, 0.02f, 0.02f));

	// Add child entities
	{	Entity* e = new Entity();
		e->name = L"X arrow";
		e->link(this);
		MyMeshComponent* c = new MyMeshComponent;
		c->color = ColorRGBAf(1, 0, 0, 1);
		c->mesh = mesh;
		e->addComponent(c);
		e->localTransform = Mat44f(Mat33f::rotateXYZ(0, 0, -Mathf::cPiOver2()));
	}

	{	Entity* e = new Entity();
		e->name = L"Y arrow";
		e->link(this);
		MyMeshComponent* c = new MyMeshComponent;
		c->color = ColorRGBAf(0, 1, 0, 1);
		c->mesh = mesh;
		e->addComponent(c);
	}

	{	Entity* e = new Entity();
		e->name = L"Z arrow";
		e->link(this);
		MyMeshComponent* c = new MyMeshComponent;
		c->color = ColorRGBAf(0, 0, 1, 1);
		c->mesh = mesh;
		e->addComponent(c);
		e->localTransform = Mat44f(Mat33f::rotateXYZ(Mathf::cPiOver2(), 0, 0));
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
	FollowTransformComponent* component = polymorphic_downcast<FollowTransformComponent*>(
		this->findComponent(typeid(BehaviourComponent)));
	return component->selectedEntity;
}
