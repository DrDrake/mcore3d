#include "stdafx.h"
#include "Gizmo.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Components/MeshComponent.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Core/System/ResourceManager.h"
#include "../../MCD/Render/MaterialProperty.h"
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

}

Gizmo::Gizmo(ResourceManager& resourceManager)
{
	ModelPtr model = dynamic_cast<Model*>(resourceManager.load(L"Arrow.3ds", true).get());
	dynamic_cast<DefaultResourceManager&>(resourceManager).processLoadingEvents();
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
