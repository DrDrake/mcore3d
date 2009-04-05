#include "stdafx.h"
#include "Gizmo.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Components/MeshComponent.h"
#include "../../MCD/Render/Components/PickComponent.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Math/Intersection.h"
#include "../../MCD/Core/Math/Plane.h"
#include "../../MCD/Core/Math/Ray.h"
#include "../../MCD/Core/Math/Vec2.h"
#include "../../MCD/Core/System/Utility.h"
#include "../../3Party/glew/glew.h"
#pragma comment(lib, "glew")

using namespace MCD;

namespace {

Vec3f unProject(const Vec3f& p)
{
	GLdouble model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	GLdouble v[3];
	MCD_VERIFY(gluUnProject(p[0], p[1], p[2], model, projection, viewPort, &v[0], &v[1], &v[2]) == GL_TRUE);

	return Vec3f(float(v[0]), float(v[1]), float(v[2]));
}

Vec3f projectToScreen(const Vec3f& p)
{
	GLdouble model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	GLdouble v[3];
	MCD_VERIFY(gluProject(p[0], p[1], p[2], model, projection, viewPort, &v[0], &v[1], &v[2]) == GL_TRUE);

	return Vec3f(float(v[0]), float(v[1]), float(v[2]));
}

//! Create a ray casting towards the sceen in world space, x and y are in windows's coordinate.
Ray createPickingRay(int x, int y)
{
	// Adjust the difference between window and opengl screen coordinate
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);
	y = viewPort[3] - y;

	// Calculate manually projected start and end points at the near and far
	// clip planes where the mouse ray would intersect them.
	Vec3f near = unProject(Vec3f(float(x), float(y), 0));
	Vec3f far = unProject(Vec3f(float(x), float(y), 1));

	return Ray(near, (far - near).normalizedCopy());
}

class MyMeshComponent : public MeshComponent
{
public:
	typedef Vec2<int> Vec2i;

	MyMeshComponent(const ColorRGBAf& c)
		: color(c), defaultColor(c), backToDefaultColor(true)
	{}

	sal_override void render()
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
		glEnable(GL_BLEND);
		glColor4fv(color.rawPointer());
		MeshComponent::render();
		glDisable(GL_BLEND);

		if(backToDefaultColor)
			color = defaultColor;
	}

	virtual Vec3f mouseMove(const Vec2i& oldPos, const Vec2i& newPos) const = 0;

	ColorRGBAf color;
	const ColorRGBAf defaultColor;
	bool backToDefaultColor;
};	// MyMeshComponent

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

	sal_override Vec3f mouseMove(const Vec2i& oldPos, const Vec2i& newPos) const
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

		// Return the delta value in 3D.
		return end - start;
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

	sal_override Vec3f mouseMove(const Vec2i& oldPos, const Vec2i& newPos) const
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

		return p2 - p1;
	}

	//! When normal of the dragging plane.
	Vec3f planeNormal;
};	// PlaneComponent

class FollowTransformComponent : public BehaviourComponent
{
public:
	sal_override void update()
	{
		Gizmo* gizmo = polymorphic_downcast<Gizmo*>(entity());
		if(!gizmo || !selectedEntity.get())
			return;
		gizmo->localTransform = selectedEntity->worldTransform();
	}

	EntityPtr selectedEntity;
};	// FollowTransformComponent

static const ColorRGBAf cHighlightedColor(1, 1, 0, 1);

class MyPickComponent : public PickComponent
{
public:
	MyPickComponent() : selectedMesh(nullptr) {}

	sal_override void update()
	{
		PickComponent::update();
		selectedMesh = nullptr;

		// Handle picking result
		for(size_t i=0; i<hitCount(); ++i) {
			EntityPtr e = hitAtIndex(i);
			if(!e.get())
				continue;

			// High light the arrow
			MyMeshComponent* c = e->findComponent<MyMeshComponent>(typeid(RenderableComponent));
			if(c) {
				c->color = cHighlightedColor;
				selectedMesh = c;
			}
			break;	// We only pick the Entity that nearest to the camera
		}
		clearResult();
	}

	MyMeshComponent* selectedMesh;
};	// MyPickComponent

}	// namespace

Gizmo::Gizmo(ResourceManager& resourceManager)
{
	enabled = false;
	dragging = nullptr;

	addComponent(new FollowTransformComponent);

	Entity* translationEntity = new Entity();
	translationEntity->name = L"Translation";
	translationEntity->asChildOf(this);

	{	// Add an entity for picking
		Entity* e = new Entity();
		e->name = L"Gizmo picker";
		e->asChildOf(this);
		MyPickComponent* c = new MyPickComponent;
		mPickComponent = c;
		c->entityToPick = translationEntity;
		e->addComponent(c);
		e->enabled = true;
	}

	// Add child entities
	{	Entity* e = new Entity();
		e->name = L"X arrow";
		e->asChildOf(translationEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(1, 0, 0, 0.8f));
		c->dragDirection = Vec3f::c100;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(0, 0, -Mathf::cPiOver2())) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = L"Y arrow";
		e->asChildOf(translationEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(0, 1, 0, 0.8f));
		c->dragDirection = Vec3f::c010;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
	}

	{	Entity* e = new Entity();
		e->name = L"Z arrow";
		e->asChildOf(translationEntity);
		ArrowComponent* c = new ArrowComponent(resourceManager, ColorRGBAf(0, 0, 1, 0.8f));
		c->dragDirection = Vec3f::c001;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(Mathf::cPiOver2(), 0, 0)) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = L"YZ plane";
		e->asChildOf(translationEntity);
		PlaneComponent* c = new PlaneComponent(resourceManager, ColorRGBAf(1, 0, 0, 0.8f));
		c->planeNormal = Vec3f::c100;
		e->addComponent(c);
		e->localTransform.setTranslation(MCD::Vec3f(0.1f, 0.1f, 0));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(0, -Mathf::cPiOver2(), 0)) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = L"XY plane";
		e->asChildOf(translationEntity);
		PlaneComponent* c = new PlaneComponent(resourceManager, ColorRGBAf(0, 0, 1, 0.8f));
		c->planeNormal = Vec3f::c001;
		e->addComponent(c);
		e->localTransform.setTranslation(MCD::Vec3f(0.1f, 0.1f, 0));
	}

	{	Entity* e = new Entity();
		e->name = L"XZ plane";
		e->asChildOf(translationEntity);
		PlaneComponent* c = new PlaneComponent(resourceManager, ColorRGBAf(0, 1, 0, 0.8f));
		c->planeNormal = Vec3f::c010;;
		e->addComponent(c);
		e->localTransform.setTranslation(MCD::Vec3f(0.1f, 0.1f, 0));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(Mathf::cPiOver2(), 0, 0)) * e->localTransform;
	}
}

void Gizmo::setSelectedEntity(const EntityPtr& selectedEntity)
{
	enabled = (selectedEntity.get() != nullptr);

	FollowTransformComponent* component = findComponent<FollowTransformComponent>(typeid(BehaviourComponent));
	component->selectedEntity = selectedEntity;
}

const EntityPtr& Gizmo::selectedEntity() const
{
	FollowTransformComponent* component = findComponent<FollowTransformComponent>(typeid(BehaviourComponent));
	return component->selectedEntity;
}

void Gizmo::mouseDown(int x, int y)
{
	MyPickComponent* picker = dynamic_cast<MyPickComponent*>(mPickComponent.get());
	MyMeshComponent* mesh = picker->selectedMesh;

	mLastMousePosition[0] = x;
	mLastMousePosition[1] = y;

	// A control mesh is picked
	if(mesh) {
		dragging = mesh;
		picker->entity()->enabled = false;	// Temporary disable the pick detection until mouse up
		mesh->color = cHighlightedColor;
		mesh->backToDefaultColor = false;
	}
}

void Gizmo::mouseMove(int x, int y)
{
	MyPickComponent* picker = dynamic_cast<MyPickComponent*>(mPickComponent.get());
	picker->setPickRegion(x, y, 1, 1);

	// Preforming dragging
	// Reference: "3D Transformation Manipulators (Translation/Rotation/Scale)"
	// http://www.ziggyware.com/readarticle.php?article_id=189
	Ray ray;
	if(dragging.get())
	{
		Vec3f delta = dynamic_cast<MyMeshComponent*>(dragging.get())->mouseMove(
			MyMeshComponent::Vec2i(mLastMousePosition[0], mLastMousePosition[1]),
			MyMeshComponent::Vec2i(x, y)
		);

		// Apply the delta value.
		selectedEntity()->localTransform.setTranslation(
			selectedEntity()->localTransform.translation() + delta
		);

		mLastMousePosition[0] = x;
		mLastMousePosition[1] = y;
	}
}

void Gizmo::mouseUp(int x, int y)
{
	MyPickComponent* picker = dynamic_cast<MyPickComponent*>(mPickComponent.get());
	picker->entity()->enabled = true;

	if(dragging.get())
		dynamic_cast<MyMeshComponent*>(dragging.get())->backToDefaultColor = true;

	// Any dragging will cancel
	dragging = nullptr;
}
