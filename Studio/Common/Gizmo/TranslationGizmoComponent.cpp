#include "stdafx.h"
#include "TranslationGizmoComponent.h"
#include "../DefaultResourceManager.h"
#include "../../../MCD/Render/Mesh.h"
#include "../../../MCD/Render/Model.h"
#include "../../../MCD/Core/Entity/Entity.h"
#include "../../../MCD/Core/Math/Intersection.h"
#include "../../../MCD/Core/Math/Plane.h"

using namespace MCD;

namespace {

//! The translation arrow for dragging.
class ArrowComponent : public MyMeshComponent
{
public:
	explicit ArrowComponent(ResourceManager& resourceManager, const ColorRGBAf& c)
		: MyMeshComponent(c)
	{
		ModelPtr model = dynamic_cast<Model*>(resourceManager.load(L"Arrow.3ds", true).get());
		dynamic_cast<DefaultResourceManager&>(resourceManager).processLoadingEvents();
		if(!model->mMeshes.empty())
			this->mesh = model->mMeshes.front().mesh;
	}

	sal_override void mouseMove(const Vec2i& oldPos, const Vec2i& newPos, MCD::Mat44f& transform) const
	{
		// Preforming dragging
		// Reference: "3D Transformation Manipulators (Translation/Rotation/Scale)"
		// http://www.ziggyware.com/readarticle.php?article_id=189
		Vec3f transformedDragDir = dragDirection;
		entity()->parent()->worldTransform().transformNormal(transformedDragDir);

		// Transform the axis direction on the screen space
		Vec3f start = projectToScreen(entity()->worldTransform().translation());
		Vec3f end = projectToScreen(entity()->worldTransform().translation() + transformedDragDir);
		Vec3f screenDir = (end - start).normalizedCopy();

		// Project the mouse dragging direciton to the screen space arrow direction
		Vec2i delta = newPos - oldPos;
		Vec3f mouseDir(float(delta.x), -float(delta.y), 0);
		mouseDir = (screenDir % mouseDir) * screenDir;
		end = start + mouseDir;

		// Un-project the arrow aligned mouse drag direction back to 3D
		start = unProject(start);
		end = unProject(end);

		// Append the delta value.
		transform.setTranslation(transform.translation() + end - start);
	}

	//! When dragging this mesh, which direction to move.
	Vec3f dragDirection;
};	// ArrowComponent

//! The translation plane for dragging in a fixed plane.
class PlaneComponent : public MyMeshComponent
{
public:
	explicit PlaneComponent(ResourceManager& resourceManager, const ColorRGBAf& c)
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

	sal_override void mouseMove(const Vec2i& oldPos, const Vec2i& newPos, MCD::Mat44f& transform) const
	{
		// Preforming dragging
		// Reference: "3D Transformation Manipulators (Translation/Rotation/Scale)"
		// http://www.ziggyware.com/readarticle.php?article_id=189

		Vec3f transformedPlaneNormal = planeNormal;
		entity()->parent()->worldTransform().transformNormal(transformedPlaneNormal);

		// Project the mouse position onto the plane
		Ray ray1 = createPickingRay(oldPos.x, oldPos.y);
		Ray ray2 = createPickingRay(newPos.x, newPos.y);

		float distanceAlongRay;
		Plane plane(transformedPlaneNormal, entity()->worldTransform().translation());
		Vec3f p1(0), p2(0);
		if(Intersects(ray1, plane, distanceAlongRay))
			p1 = ray1.getPoint(distanceAlongRay);
		if(Intersects(ray2, plane, distanceAlongRay))
			p2 = ray2.getPoint(distanceAlongRay);

		// Append the delta value.
		transform.setTranslation(transform.translation() + p2 - p1);
	}

	//! Normal of the dragging plane.
	Vec3f planeNormal;
};	// PlaneComponent

}	// namespace

TranslationGizmoComponent::TranslationGizmoComponent(ResourceManager& resourceManager, Entity* hostEntity)
	: GizmoBaseComponent(hostEntity)
{
	// Add child entities
	{	Entity* e = new Entity();
		e->name = L"X arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(1, 0, 0, 0.8f));
		c->dragDirection = Vec3f::c100;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(0, 0, -Mathf::cPiOver2())) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = L"Y arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(0, 1, 0, 0.8f));
		c->dragDirection = Vec3f::c010;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
	}

	{	Entity* e = new Entity();
		e->name = L"Z arrow";
		e->asChildOf(hostEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(0, 0, 1, 0.8f));
		c->dragDirection = Vec3f::c001;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(Mathf::cPiOver2(), 0, 0)) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = L"YZ plane";
		e->asChildOf(hostEntity);
		PlaneComponent* c = new PlaneComponent(resourceManager, ColorRGBAf(1, 0, 0, 0.8f));
		c->planeNormal = Vec3f::c100;
		e->addComponent(c);
		e->localTransform.setTranslation(MCD::Vec3f(0.1f, 0.1f, 0));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(0, -Mathf::cPiOver2(), 0)) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = L"XY plane";
		e->asChildOf(hostEntity);
		PlaneComponent* c = new PlaneComponent(resourceManager, ColorRGBAf(0, 0, 1, 0.8f));
		c->planeNormal = Vec3f::c001;
		e->addComponent(c);
		e->localTransform.setTranslation(MCD::Vec3f(0.1f, 0.1f, 0));
	}

	{	Entity* e = new Entity();
		e->name = L"XZ plane";
		e->asChildOf(hostEntity);
		PlaneComponent* c = new PlaneComponent(resourceManager, ColorRGBAf(0, 1, 0, 0.8f));
		c->planeNormal = Vec3f::c010;
		e->addComponent(c);
		e->localTransform.setTranslation(MCD::Vec3f(0.1f, 0.1f, 0));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(Mathf::cPiOver2(), 0, 0)) * e->localTransform;
	}
}
