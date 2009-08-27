#include "stdafx.h"
#include "RotationGizmoComponent.h"
#include "../../../MCD/Render/Mesh.h"
#include "../../../MCD/Render/Model.h"
#include "../../../MCD/Core/Entity/Entity.h"
#include "../../../MCD/Core/System/ResourceManager.h"

using namespace MCD;

namespace {

//! The rotation arrow for dragging.
class ArrowComponent : public MyMeshComponent
{
	struct Callback : public ResourceManagerCallback
	{
		sal_override void doCallback() {
			MCD_ASSERT(model);
			if(model && !model->mMeshes.isEmpty())
				backRef->mesh = model->mMeshes.front().mesh;
		}
		ModelPtr model;
		ArrowComponent* backRef;
	};	// Callback

public:
	explicit ArrowComponent(IResourceManager& resourceManager, const ColorRGBAf& c)
		: MyMeshComponent(c)
	{
		Callback* cb = new Callback();
		cb->backRef = this;
		cb->model = dynamic_cast<Model*>(resourceManager.load(L"Rotate.3ds", true).get());
		cb->addDependency(L"Rotate.3ds");
		resourceManager.addCallback(cb);
	}

	sal_override void mouseMove(Vec2i& oldPos, const Vec2i& newPos,
		const MCD::Mat44f& oldTransform, Mat44f& transform) const
	{
		// Preforming dragging
		// Reference: "3D Transformation Manipulators (Translation/Rotation/Scale)"
		// http://www.ziggyware.com/readarticle.php?article_id=189
		Vec3f transformedRotationAxis = rotationAxis;
		oldTransform.transformNormal(transformedRotationAxis);

		// Rotate along the axis with an amount controled by the vertical displacement of the mouse.
		Mat33f deltaRotation = Mat33f::makeAxisRotation(
			transformedRotationAxis,
			Mathf::toRadian(float(newPos.y - oldPos.y))
		);

		// Ensure we are modifing the pure rotation only
		Vec3f scale = oldTransform.scale();
		Mat33f pureRotaton = oldTransform.mat33();
		pureRotaton.setScale(Vec3f(1));

		Mat33f final = deltaRotation * pureRotaton;
		final.setScale(scale);
		transform.setMat33(final);
	}

	//! When dragging, which axis to rotate.
	Vec3f rotationAxis;
};	// ArrowComponent

}	// namespace

RotationGizmoComponent::RotationGizmoComponent(IResourceManager& resourceManager, Entity* hostEntity, InputComponent* inputComponent)
	: GizmoBaseComponent(hostEntity, inputComponent)
{
	// Add child entities
	{	Entity* e = new Entity();
		e->name = L"X arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(1, 0, 0, 0.8f));
		c->rotationAxis = Vec3f::c100;
		e->addComponent(c);
		e->localTransform.setScale(Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::makeXYZRotation(0, 0, -Mathf::cPiOver2())) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = L"Y arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(0, 1, 0, 0.8f));
		c->rotationAxis = Vec3f::c010;
		e->addComponent(c);
		e->localTransform.setScale(Vec3f(1, 1.5f, 1));
	}

	{	Entity* e = new Entity();
		e->name = L"Z arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(0, 0, 1, 0.8f));
		c->rotationAxis = Vec3f::c001;
		e->addComponent(c);
		e->localTransform.setScale(Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::makeXYZRotation(Mathf::cPiOver2(), 0, 0)) * e->localTransform;
	}
}
