#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Camera.h"
#include "../../MCD/Render/Chamferbox.h"
#include "../../MCD/Render/Light.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Renderer.h"
#include "../../MCD/Render/RenderTargetComponent.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/WindowEvent.h"
#include "../../MCD/Core/System/ResourceLoader.h"
#include "../../MCD/Core/System/RawFileSystem.h"

#include "../../MCD/Component/Input/FpsControllerComponent.h"
#include "../../MCD/Component/Input/WinMessageInputComponent.h"

using namespace MCD;

TEST(RendererTest)
{
	DeltaTimer timer;
	DefaultResourceManager resourceManager(*new RawFileSystem("Media"));

	RenderWindow mainWindow;
	mainWindow.create("title=RendererTest;width=800;height=600;fullscreen=0;FSAA=4");
	CHECK(mainWindow.makeActive());

	RenderWindow subWindow;
	subWindow.create("title=Sub-window;width=400;height=300;fullscreen=0;FSAA=4");

	Entity root;
	RendererComponent* renderer = new RendererComponent;
	root.addComponent(renderer);

	// Camera
	CameraComponent2* camera = new CameraComponent2(renderer);

	{	Entity* e = new Entity;
		e->name = "Camera";
		e->asChildOf(&root);
		e->localTransform.setTranslation(Vec3f(0, 0, 10));
		e->addComponent(camera);
	}

	// Render target
	WindowRenderTargetComponent* mainRenderTarget = new WindowRenderTargetComponent;
	mainRenderTarget->window = &mainWindow;
	mainRenderTarget->entityToRender = &root;
	mainRenderTarget->cameraComponent = camera;
	mainRenderTarget->rendererComponent = renderer;

	{	Entity* e = new Entity;
		e->name = "Main window render target";
		e->asChildOf(&root);
		e->addComponent(mainRenderTarget);
	}

	WindowRenderTargetComponent* subRenderTarget = new WindowRenderTargetComponent;
	subRenderTarget->window = &subWindow;
	subRenderTarget->entityToRender = &root;
	subRenderTarget->cameraComponent = camera;
	subRenderTarget->rendererComponent = renderer;

	{	Entity* e = new Entity;
		e->name = "Sub-window render target";
		e->asChildOf(&root);
		e->addComponent(subRenderTarget);
	}

	// Light
	{	LightComponent* light = new LightComponent;
		light->color = ColorRGBf(1, 0.8f, 0.8f);
		Entity* e = new Entity;
		e->name = "Light1";
		e->asChildOf(&root);
		e->localTransform.setTranslation(Vec3f(10, 10, 0));
		e->addComponent(light);
	}

	{	LightComponent* light = new LightComponent;
		light->color = ColorRGBf(0.8f, 1, 0.8f);
		Entity* e = new Entity;
		e->name = "Light2";
		e->asChildOf(&root);
		e->localTransform.setTranslation(Vec3f(0, 10, 10));
		e->addComponent(light);
	}

	// Input
	WinMessageInputComponent* winMsg = new WinMessageInputComponent;
	winMsg->attachTo(mainWindow);
	root.addComponent(winMsg);

	// Fps controller
	FpsControllerComponent* fpsControl = new FpsControllerComponent;
	fpsControl->target = camera->entity();
	fpsControl->inputComponent = winMsg;

	{	Entity* e = new Entity;
		e->name = "Fps controller";
		e->asChildOf(&root);
		e->addComponent(fpsControl);
	}

	// Material
	MaterialComponent* material1 = new MaterialComponent;
	material1->diffuseMap = dynamic_cast<Texture*>(resourceManager.load("InterlacedTrans256x256.png").get());
	material1->opacity = 1.0f;

	MaterialComponent* material2 = new MaterialComponent;
	material2->specularExponent = 200;
	material2->opacity = 0.8f;

	// Create mesh
	MeshComponent2* boxMesh = new MeshComponent2;
	boxMesh->mesh = new Mesh("");
	CHECK(boxMesh->mesh->create(ChamferBoxBuilder(0.5f, 5, true), Mesh::Static));

	EntityPtr boxes = new Entity;
	boxes->name = "Boxes";
	boxes->addComponent(material1);
	boxes->asChildOf(&root);

	{	Entity* e = new Entity;
		e->name = "Chamfer box1";
		e->asChildOf(boxes.get());
		e->addComponent(boxMesh);
		e->localTransform.translateBy(Vec3f(2, 0, -2));
	}

	{	Entity* e = new Entity;
		e->name = "Chamfer box2";
		e->asChildOf(boxes.get());
		e->addComponent(boxMesh->clone());
		e->localTransform.translateBy(Vec3f(-2, 0, -2));
	}

	MeshComponent2* sphereMesh = new MeshComponent2;
	sphereMesh->mesh = new Mesh("");
	CHECK(sphereMesh->mesh->create(ChamferBoxBuilder(1, 5, true), Mesh::Static));

	EntityPtr spheres = new Entity;
	spheres->name = "Spheres";
	spheres->addComponent(material2);
	spheres->asChildOf(&root);

	{	Entity* e = new Entity;
		e->name = "Sphere1";
		e->asChildOf(spheres.get());
		e->addComponent(sphereMesh);
		e->localTransform.translateBy(Vec3f(2, 0, 2));
	}

	{	Entity* e = new Entity;
		e->name = "Sphere2";
		e->asChildOf(spheres.get());
		e->addComponent(material2);
		e->addComponent(sphereMesh->clone());
		e->localTransform.translateBy(Vec3f(-2, 0, 2));
	}

	while(true)
	{
		Event e;
		// Check for window events
		if(mainWindow.popEvent(e, false) && e.Type == Event::Closed)
			break;

		subWindow.popEvent(e, false);

		resourceManager.processLoadingEvents();

		const float dt = (float)timer.getDelta().asSecond();
		BehaviourComponent::traverseEntities(&root, dt);

		renderer->render(root);
	}

	CHECK(true);
}
