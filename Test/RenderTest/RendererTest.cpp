#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Camera.h"
#include "../../MCD/Render/Chamferbox.h"
#include "../../MCD/Render/Light.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Renderer.h"
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

	RenderWindow window;
	window.create("title=RendererTest;width=800;height=600;fullscreen=0;FSAA=4");
	CHECK(window.makeActive());

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
	winMsg->attachTo(window);
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

	// Create mesh
	MeshComponent2* mesh = new MeshComponent2;
	mesh->mesh = new Mesh("");
	CHECK(mesh->mesh->create(ChamferBoxBuilder(0.5f, 5, true), Mesh::Static));

	{	Entity* e = new Entity;
		e->name = "Chamfer box1";
		e->asChildOf(&root);
		e->addComponent(mesh);
		e->localTransform.translateBy(Vec3f(2, 0, 0));
	}

	{	Entity* e = new Entity;
		e->name = "Chamfer box2";
		e->asChildOf(&root);
		e->addComponent(mesh->clone());
		e->localTransform.translateBy(Vec3f(-2, 0, 0));
	}

	// Material
	MaterialComponent* material = new MaterialComponent;
	root.addComponent(material);
	material->diffuseMap = dynamic_cast<Texture*>(resourceManager.load("InterlacedTrans256x256.png").get());

	while(true)
	{
		Event e;
		// Check for window events
		if(window.popEvent(e, false) && e.Type == Event::Closed)
			break;

		const float dt = (float)timer.getDelta().asSecond();
		BehaviourComponent::traverseEntities(&root, dt);

		resourceManager.processLoadingEvents();
		window.preUpdate();

		RenderableComponent2::traverseEntities(&root);
		renderer->render(root);

		window.postUpdate();
	}

	CHECK(true);
}
