#include "stdafx.h"
#include "GizmoBaseComponent.h"
#include "Gizmo.h"
#include "../../../MCD/Core/Entity/Entity.h"
#include "../../../MCD/Component/Render/PickComponent.h"
#include "../../../3Party/glew/glew.h"

using namespace MCD;

namespace {

static const ColorRGBAf cHighlightedColor(1, 1, 0, 1);

class MyPickComponent : public PickComponent
{
public:
	MyPickComponent() : selectedMesh(nullptr) {}

	sal_override void update(float dt)
	{
		PickComponent::update(dt);
		selectedMesh = nullptr;

		// Handle picking result
		for(size_t i=0; i<hitCount(); ++i) {
			EntityPtr e = hitAtIndex(i);
			if(!e)
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

Vec3f unProject(const Vec3f& p)
{
	GLdouble model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	GLdouble v[3] = {0};
	MCD_VERIFY(gluUnProject(p[0], p[1], p[2], model, projection, viewPort, &v[0], &v[1], &v[2]) == GL_TRUE);

	return Vec3f(float(v[0]), float(v[1]), float(v[2]));
}

bool projectToScreen(const Vec3f& p, Vec3f& result)
{
	GLdouble model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	GLdouble v[3] = {0};

	bool ok = gluProject(p[0], p[1], p[2], model, projection, viewPort, &v[0], &v[1], &v[2]) == GL_TRUE;
	result.x = float(v[0]);
	result.y = float(v[1]);
	result.z = float(v[2]);

	return ok;
}

Vec3f projectToScreenNoModelView(const Vec3f& p)
{
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	GLdouble v[3] = {0};
	// gluProject will fail at certain view position/direction (not study yet, but seems so when camera at (0,0,0)
	if(gluProject(p[0], p[1], p[2], Mat44<double>::cIdentity.getPtr(), projection, viewPort, &v[0], &v[1], &v[2]) == GL_TRUE)
		return Vec3f(float(v[0]), float(v[1]), float(v[2]));
	else
		return Vec3f(0);
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

MyMeshComponent::MyMeshComponent(const ColorRGBAf& c)
	: color(c), defaultColor(c), backToDefaultColor(true)
{}

void MyMeshComponent::render()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
	glEnable(GL_BLEND);
	glColor4fv(color.rawPointer());
	MeshComponent::render();
	glDisable(GL_BLEND);

	if(backToDefaultColor)
		color = defaultColor;
}

GizmoBaseComponent::GizmoBaseComponent(Entity* hostEntity, InputComponent* inputComponent)
	: mInputComponent(inputComponent)
{
	dragging = nullptr;

	// Add an entity for picking
	Entity* e = new Entity();
	e->name = L"Picker";
	e->asChildOf(hostEntity);
	MyPickComponent* c = new MyPickComponent;
	mPickComponent = c;
	c->entityToPick = hostEntity;
	e->addComponent(c);
}

void GizmoBaseComponent::update(float)
{
	InputComponent* input = mInputComponent.get();
	Gizmo* gizmo = dynamic_cast<Gizmo*>(entity()->parent()->parent());
	Entity* selectedEntity = gizmo ? gizmo->selectedEntity() : nullptr;

	if(input && selectedEntity)
	{
		const Vec2i currentMouseAxis = Vec2i(
			int(input->getAxis(L"mouse x")),
			int(input->getAxis(L"mouse y"))
		);

		if(input->getMouseButton(0))
			mouseDown(currentMouseAxis.x, currentMouseAxis.y, selectedEntity->localTransform);

		mouseMove(currentMouseAxis.x, currentMouseAxis.y, selectedEntity->localTransform);
	}
}

void GizmoBaseComponent::mouseDown(int x, int y, Mat44f& transform)
{
	MyPickComponent* picker = dynamic_cast<MyPickComponent*>(mPickComponent.get());
	MCD_ASSUME(picker);
	MyMeshComponent* mesh = picker->selectedMesh;

	mBackupMatrix = transform;
	mOldMousePosition[0] = x;
	mOldMousePosition[1] = y;

	// A control mesh is picked
	if(mesh) {
		dragging = mesh;
		picker->entity()->enabled = false;	// Temporary disable the pick detection until mouse up
		mesh->color = cHighlightedColor;
		mesh->backToDefaultColor = false;
	}
}

void GizmoBaseComponent::mouseMove(int x, int y, Mat44f& transform)
{
	MyPickComponent* picker = dynamic_cast<MyPickComponent*>(mPickComponent.get());
	MCD_ASSUME(picker);
	picker->setPickRegion(x, y, 1, 1);

	if(dragging)
	{
		dynamic_cast<MyMeshComponent*>(dragging.get())->mouseMove(
			mOldMousePosition,
			MyMeshComponent::Vec2i(x, y),
			mBackupMatrix, transform
		);
	}
}

void GizmoBaseComponent::mouseUp(int x, int y)
{
	MyPickComponent* picker = dynamic_cast<MyPickComponent*>(mPickComponent.get());
	picker->entity()->enabled = true;

	if(dragging)
		dynamic_cast<MyMeshComponent*>(dragging.get())->backToDefaultColor = true;

	// Any dragging will cancel
	dragging = nullptr;
}
