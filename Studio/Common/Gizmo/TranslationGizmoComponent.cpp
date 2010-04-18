#include "stdafx.h"
#include "TranslationGizmoComponent.h"
#include "../../../MCD/Render/Mesh.h"
#include "../../../MCD/Render/Model.h"
#include "../../../MCD/Core/Entity/Entity.h"
#include "../../../MCD/Core/Math/Intersection.h"
#include "../../../MCD/Core/Math/Plane.h"
#include "../../../MCD/Core/System/ResourceManager.h"
#include "../../../3Party/glew/glew.h"

using namespace MCD;

namespace {

//! The translation arrow for dragging.
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
		cb->model = dynamic_cast<Model*>(resourceManager.load("Arrow.3ds").get());
		cb->setMajorDependency("Arrow.3ds");
		resourceManager.addCallback(cb);
	}

	sal_override void mouseMove(Vec2i& oldPos, const Vec2i& newPos,
		const MCD::Mat44f& oldTransform, MCD::Mat44f& transform) const
	{
		// Preforming dragging
		// Reference: "3D Transformation Manipulators (Translation/Rotation/Scale)"
		// http://www.ziggyware.com/readarticle.php?article_id=189

		// Transform the axis direction on the screen space
		// NOTE: projectToScreen() may fail if oldTransform.translation() is behind the camera.
		Vec3f start, end;
		if(!projectToScreen(oldTransform.translation(), start)) return;
		if(!projectToScreen(oldTransform.translation() + dragDirection, end)) return;
		Vec3f screenDir = (end - start).normalizedCopy();

		// Project the mouse dragging direciton to the screen space arrow direction
		Vec2i delta = newPos - oldPos;
		Vec3f mouseDir(float(delta.x), -float(delta.y), 0);
		mouseDir = (screenDir % mouseDir) * screenDir;
		end = start + mouseDir;

		// Un-project the arrow aligned mouse drag direction back to 3D
		start = unProject(start);
		end = unProject(end);

		// Apply the delta value.
		transform.setTranslation(oldTransform.translation() + end - start);
	}

	//! When dragging this mesh, which direction to move.
	Vec3f dragDirection;
};	// ArrowComponent

//! The translation plane for dragging in a fixed plane.
class PlaneComponent : public MyMeshComponent
{
public:
	explicit PlaneComponent(IResourceManager& resourceManager, const ColorRGBAf& c)
		: MyMeshComponent(c)
	{
	}

	sal_override void renderFaceOnly()
	{
		PlaneComponent::render();
	}

	sal_override void render()
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glColor4fv(color.rawPointer());
		glPushMatrix();
		glMultTransposeMatrixf(entity()->worldTransform().getPtr());
		glBegin(GL_QUADS);
			glVertex3f(-0.1f, -0.1f,  0);
			glVertex3f( 0.1f, -0.1f,  0);
			glVertex3f( 0.1f,  0.1f,  0);
			glVertex3f(-0.1f,  0.1f,  0);
		glEnd();
		glPopMatrix();
		glEnable(GL_CULL_FACE);
//		MeshComponent::render();
		glDisable(GL_BLEND);

		if(backToDefaultColor)
			color = defaultColor;
	}

	sal_override void mouseMove(Vec2i& oldPos, const Vec2i& newPos,
		const MCD::Mat44f& oldTransform, MCD::Mat44f& transform) const
	{
		// Preforming dragging
		// Reference: "3D Transformation Manipulators (Translation/Rotation/Scale)"
		// http://www.ziggyware.com/readarticle.php?article_id=189

		Vec3f transformedPlaneNormal = planeNormal;
		oldTransform.transformNormal(transformedPlaneNormal);

		// Project the mouse position onto the plane
		Ray ray1 = createPickingRay(oldPos.x, oldPos.y);
		Ray ray2 = createPickingRay(newPos.x, newPos.y);

		float distanceAlongRay;
		Plane plane(transformedPlaneNormal, oldTransform.translation());
		Vec3f p1(0), p2(0);
		if(Intersects(ray1, plane, distanceAlongRay))
			p1 = ray1.getPoint(distanceAlongRay);
		if(Intersects(ray2, plane, distanceAlongRay))
			p2 = ray2.getPoint(distanceAlongRay);

		// Apply the delta value.
		transform.setTranslation(oldTransform.translation() + p2 - p1);
	}

	//! Normal of the dragging plane.
	Vec3f planeNormal;
};	// PlaneComponent

}	// namespace

TranslationGizmoComponent::TranslationGizmoComponent(IResourceManager& resourceManager, Entity* hostEntity, InputComponent* inputComponent)
	: GizmoBaseComponent(hostEntity, inputComponent)
{
	referenceFrame = Global;

	// Add child entities
	{	Entity* e = new Entity();
		e->name = "X arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(1, 0, 0, 0.8f));
		c->dragDirection = Vec3f::c100;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::makeXYZRotation(0, 0, -Mathf::cPiOver2())) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = "Y arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(0, 1, 0, 0.8f));
		c->dragDirection = Vec3f::c010;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
	}

	{	Entity* e = new Entity();
		e->name = "Z arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(0, 0, 1, 0.8f));
		c->dragDirection = Vec3f::c001;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::makeXYZRotation(Mathf::cPiOver2(), 0, 0)) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = "YZ plane";
		e->asChildOf(hostEntity);
		PlaneComponent* c = new PlaneComponent(resourceManager, ColorRGBAf(1, 0, 0, 0.8f));
		c->planeNormal = Vec3f::c100;
		e->addComponent(c);
		e->localTransform.setTranslation(MCD::Vec3f(0.1f, 0.1f, 0));
		e->localTransform = Mat44f(Mat33f::makeXYZRotation(0, -Mathf::cPiOver2(), 0)) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = "XY plane";
		e->asChildOf(hostEntity);
		PlaneComponent* c = new PlaneComponent(resourceManager, ColorRGBAf(0, 0, 1, 0.8f));
		c->planeNormal = Vec3f::c001;
		e->addComponent(c);
		e->localTransform.setTranslation(MCD::Vec3f(0.1f, 0.1f, 0));
	}

	{	Entity* e = new Entity();
		e->name = "XZ plane";
		e->asChildOf(hostEntity);
		PlaneComponent* c = new PlaneComponent(resourceManager, ColorRGBAf(0, 1, 0, 0.8f));
		c->planeNormal = Vec3f::c010;
		e->addComponent(c);
		e->localTransform.setTranslation(MCD::Vec3f(0.1f, 0.1f, 0));
		e->localTransform = Mat44f(Mat33f::makeXYZRotation(Mathf::cPiOver2(), 0, 0)) * e->localTransform;
	}
}
