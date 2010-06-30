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

	EntityPtr guiLayer = new Entity;
	guiLayer->name = "Gui layer";
//	guiLayer->enabled = false;
	guiLayer->asChildOf(&root);

	// Camera
	CameraComponent2* mainCamera = new CameraComponent2(renderer);

	{	Entity* e = new Entity;
		e->name = "Camera";
		e->asChildOf(&root);
		e->localTransform.setTranslation(Vec3f(0, 0, 10));
		e->addComponent(mainCamera);
	}

	CameraComponent2* guiCamera = new CameraComponent2(renderer);

	{	Entity* e = new Entity;
		e->name = "Gui camera";
		e->asChildOf(guiLayer.get());
		e->localTransform.setTranslation(Vec3f(0, 0, 5));
		e->addComponent(guiCamera);
	}

	// Render target
	RenderTargetComponent* mainRenderTarget = new RenderTargetComponent;
	mainRenderTarget->window = &mainWindow;
	mainRenderTarget->entityToRender = &root;
	mainRenderTarget->cameraComponent = mainCamera;
	mainRenderTarget->rendererComponent = renderer;

	{	Entity* e = new Entity;
		e->name = "Main window render target";
		e->asChildOf(&root);
		e->addComponent(mainRenderTarget);
	}

	RenderTargetComponent* subRenderTarget = new RenderTargetComponent;
	subRenderTarget->clearColor = ColorRGBAf(0.3f, 0.3f, 0.3f, 0);
	subRenderTarget->window = &subWindow;
	subRenderTarget->entityToRender = &root;
	subRenderTarget->cameraComponent = mainCamera;
	subRenderTarget->rendererComponent = renderer;

	RenderTargetComponent* guiRenderTarget = new RenderTargetComponent;
	guiRenderTarget->shouldClearColor = false;
	guiRenderTarget->clearColor = ColorRGBAf(0, 0);
	guiRenderTarget->window = &mainWindow;
	guiRenderTarget->entityToRender = guiLayer.get();
	guiRenderTarget->cameraComponent = guiCamera;
	guiRenderTarget->rendererComponent = renderer;

	{	Entity* e = new Entity;
		e->name = "Sub-window render target";
		// NOTE: Due to some restriction on wglShareLists, the order of window
		// render target component does have a effect. So prefer the main window's
		// render target appear first in the Entity tree.
		e->insertAfter(mainRenderTarget->entity());
		e->addComponent(subRenderTarget);
	}

	{	Entity* e = new Entity;
		e->name = "Gui render target";
		e->insertAfter(mainRenderTarget->entity());
		e->addComponent(guiRenderTarget);
	}

	RenderTargetComponent* textureRenderTarget = new RenderTargetComponent;
	textureRenderTarget->clearColor = ColorRGBAf(0.3f, 0.6f, 0.9f, 1);
	textureRenderTarget->window = nullptr;
	textureRenderTarget->entityToRender = &root;
	textureRenderTarget->cameraComponent = mainCamera;
	textureRenderTarget->rendererComponent = renderer;
	textureRenderTarget->textures[0] = textureRenderTarget->createTexture(GpuDataFormat::get("uintRGBA8"), 512, 512);

	{	Entity* e = new Entity;
		e->name = "Texture render target";
		e->asChildOf(&root);
		e->addComponent(textureRenderTarget);
	}

	// Material
	MaterialComponent* material1 = new MaterialComponent;
	material1->diffuseColor = ColorRGBAf(1, 1, 0.5f, 1);
	material1->diffuseMap = dynamic_cast<Texture*>(resourceManager.load("InterlacedTrans256x256.png").get());
	material1->opacity = 1.0f;

	MaterialComponent* material2 = new MaterialComponent;
	material2->specularExponent = 120;
	material2->opacity = 0.8f;
	material2->diffuseMap = textureRenderTarget->textures[0];

	root.addComponent(material1);

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
	fpsControl->target = mainCamera->entity();
	fpsControl->inputComponent = winMsg;

	{	Entity* e = new Entity;
		e->name = "Fps controller";
		e->asChildOf(&root);
		e->addComponent(fpsControl);
	}

	// Create mesh
	MeshComponent2* boxMesh = new MeshComponent2;
	boxMesh->mesh = new Mesh("");
	CHECK(boxMesh->mesh->create(ChamferBoxBuilder(0.5f, 5, true), Mesh::Static));

	EntityPtr boxes = new Entity;
	boxes->name = "Boxes";
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

	{	Entity* e = new Entity;
		e->name = "Gui";
		e->asChildOf(guiLayer.get());
		e->addComponent(material2);
		e->addComponent(sphereMesh->clone());
		e->localTransform.translateBy(Vec3f(0, 0, 0));
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
