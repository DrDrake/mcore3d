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
#include "../../MCD/Core/Math/Ray.h"
#include "../../MCD/Core/System/Utility.h"
#include "../../3Party/glew/glew.h"

using namespace MCD;

namespace {

class MyMeshComponent : public MeshComponent
{
public:
	explicit MyMeshComponent(const ColorRGBAf& c)
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

	ColorRGBAf color;
	const ColorRGBAf defaultColor;

	//! When dragging this mesh, which direction to move.
	Vec3f dragDirection;

	bool backToDefaultColor;
};	// MyMeshComponent

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
	draggingEntity = nullptr;

	ModelPtr model = dynamic_cast<Model*>(resourceManager.load(L"Arrow.3ds", true).get());
	dynamic_cast<DefaultResourceManager&>(resourceManager).processLoadingEvents();

	addComponent(new FollowTransformComponent);

	if(model->mMeshes.empty())	// Fail to load the Gizmo model
		return;

	MeshPtr mesh = model->mMeshes.front().mesh;

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
		MyMeshComponent* c = new MyMeshComponent(ColorRGBAf(1, 0, 0, 0.8f));
		c->mesh = mesh;
		c->dragDirection = Vec3f::c100;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
		e->localTransform = Mat44f(Mat33f::rotateXYZ(0, 0, -Mathf::cPiOver2())) * e->localTransform;
	}

	{	Entity* e = new Entity();
		e->name = L"Y arrow";
		e->asChildOf(translationEntity);
		MyMeshComponent* c = new MyMeshComponent(ColorRGBAf(0, 1, 0, 0.8f));
		c->mesh = mesh;
		c->dragDirection = Vec3f::c010;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
	}

	{	Entity* e = new Entity();
		e->name = L"Z arrow";
		e->asChildOf(translationEntity);
		MyMeshComponent* c = new MyMeshComponent(ColorRGBAf(0, 0, 1, 0.8f));
		c->mesh = mesh;
		c->dragDirection = Vec3f::c001;
		e->addComponent(c);
		e->localTransform.setScale(MCD::Vec3f(1, 1.5f, 1));
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

//! Create a ray casting towards the sceen in world space, x and y are in windows's coordinate.
static Ray createPickingRay(float x, float y)
{
	// Calculate manually projected start and end points at the near and far
	// clip planes where the mouse ray would intersect them.

	GLdouble model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	// Adjust the difference between window and opengl screen coordinate
	y = viewPort[3] - y;

	double near[3];
	double far[3];
	MCD_VERIFY(gluUnProject(x, y, 0, model, projection, viewPort, &near[0], &near[1], &near[2]) == GL_TRUE);
	MCD_VERIFY(gluUnProject(x, y, 1, model, projection, viewPort, &far[0], &far[1], &far[2]) == GL_TRUE);

	return Ray(
		Vec3f(float(near[0]), float(near[1]), float(near[2])),
		Vec3f(float(far[0]-near[0]), float(far[1]-near[1]), float(far[2]-near[2])).normalizedCopy()
	);
}

static Vec3f unProject(const Vec3f& p)
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

static Vec3f projectToScreen(const Vec3f& p)
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

void Gizmo::mouseDown(int x, int y)
{
	MyPickComponent* picker = dynamic_cast<MyPickComponent*>(mPickComponent.get());
	MyMeshComponent* mesh = picker->selectedMesh;

	mLastMousePosition[0] = x;
	mLastMousePosition[1] = y;

	// A control mesh is picked
	if(mesh) {
		mDraggingMeshComponent = mesh;
		picker->entity()->enabled = false;	// Temporary disable the pick detection until mouse up
		mesh->color = cHighlightedColor;
		mesh->backToDefaultColor = false;
		draggingEntity = mesh->entity();
	}
}

void Gizmo::mouseMove(int x, int y)
{
	MyPickComponent* picker = dynamic_cast<MyPickComponent*>(mPickComponent.get());
	picker->setPickRegion(x, y, 5, 5);

	// Preforming dragging
	// Reference: "3D Transformation Manipulators (Translation/Rotation/Scale)"
	// http://www.ziggyware.com/readarticle.php?article_id=189
	Ray ray;
	if(draggingEntity)
	{
		Vec3f transformedDragDir = picker->selectedMesh->dragDirection;
		worldTransform().transformNormal(transformedDragDir);

		// Transform the axis direction on the screen space
		Vec3f start = projectToScreen(worldTransform().translation());
		Vec3f end = projectToScreen(worldTransform().translation() + transformedDragDir);
		Vec3f screenDir = (end - start).normalizedCopy();

		// Project the mouse dragging direciton to the screen space arrow direction
		Vec3f mouseDir(float(x - mLastMousePosition[0]), -float(y - mLastMousePosition[1]), 0);
		mouseDir = (screenDir % mouseDir) * screenDir;
		end = start + mouseDir;

		// Un-project the arrow aligned mouse drag direction back to 3D
		start = unProject(start);
		end = unProject(end);

		// Apply the delta value.
		selectedEntity()->localTransform.setTranslation(
			selectedEntity()->localTransform.translation() + (end - start)
		);

		mLastMousePosition[0] = x;
		mLastMousePosition[1] = y;
	}
}

void Gizmo::mouseUp(int x, int y)
{
	MyPickComponent* picker = dynamic_cast<MyPickComponent*>(mPickComponent.get());
	picker->entity()->enabled = true;

	if(draggingEntity)
		dynamic_cast<MyMeshComponent*>(mDraggingMeshComponent.get())->backToDefaultColor = true;

	// Any dragging will cancel
	draggingEntity = nullptr;
}
