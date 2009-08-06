#include "stdafx.h"
#include "ScaleGizmoComponent.h"
#include "../DefaultResourceManager.h"
#include "../../../MCD/Render/Mesh.h"
#include "../../../MCD/Render/Model.h"
#include "../../../MCD/Core/Entity/Entity.h"

using namespace MCD;

namespace {

//! The translation arrow for dragging.
class ArrowComponent : public MyMeshComponent
{
public:
	explicit ArrowComponent(ResourceManager& resourceManager, const ColorRGBAf& c)
		: MyMeshComponent(c)
	{
		ModelPtr model = dynamic_cast<Model*>(resourceManager.load(L"Scale.3ds", true).get());
		dynamic_cast<DefaultResourceManager&>(resourceManager).processLoadingEvents();
		if(!model->mMeshes.isEmpty())
			this->mesh = model->mMeshes.front().mesh;
	}

	sal_override void mouseMove(Vec2i& oldPos, const Vec2i& newPos,
		const MCD::Mat44f& oldTransform, MCD::Mat44f& transform) const
	{
		Vec3f oldScale = oldTransform.scale();
		oldScale[scalingAxis] -= (float(newPos.y - oldPos.y)) / 200;

		// Prevent scale <= 0
		const float cMinScale = 1.0f / 200;
		oldScale[scalingAxis] = oldScale[scalingAxis] < cMinScale ? cMinScale : oldScale[scalingAxis];

		// Apply the delta value.
		transform.setScale(oldScale);
	}

	//! When dragging this mesh, which axis [0, 1, 2] to scale.
	size_t scalingAxis;
};	// ArrowComponent

}	// namespace

ScaleGizmoComponent::ScaleGizmoComponent(ResourceManager& resourceManager, Entity* hostEntity)
	: GizmoBaseComponent(hostEntity)
{
	// Add child entities
	{	Entity* e = new Entity();
		e->name = L"X arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(1, 0, 0, 0.8f));
		c->scalingAxis = 0;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::makeXYZRotation(0, 0, -Mathf::cPiOver2())) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = L"Y arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(0, 1, 0, 0.8f));
		c->scalingAxis = 1;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
	}

	{	Entity* e = new Entity();
		e->name = L"Z arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(0, 0, 1, 0.8f));
		c->scalingAxis = 2;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::makeXYZRotation(Mathf::cPiOver2(), 0, 0)) * e->localTransform;
	}
}
