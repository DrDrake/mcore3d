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

	addComponent(new FollowTransformComponent);

	if(model->mMeshes.empty())	// Fail to load the Gizmo model
		return;

	MeshPtr mesh = model->mMeshes.front().mesh;

//	this->localTransform.setScale(Vec3f(0.02f, 0.02f, 0.02f));

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
	FollowTransformComponent* component = polymorphic_downcast<FollowTransformComponent*>(
		this->findComponent(typeid(BehaviourComponent)));
	return component->selectedEntity;
}
