#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Render/Camera.h"
#include "../../MCD/Render/Chamferbox.h"
#include "../../MCD/Render/Font.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/QuadComponent.h"
#include "../../MCD/Render/Renderer.h"
#include "../../MCD/Render/RenderTargetComponent.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/StrUtility.h"
#include "../../MCD/Core/System/WindowEvent.h"

using namespace MCD;

TEST(RendererTest)
{
	DeltaTimer timer;
	Framework framework;
	CHECK(framework.addFileSystem("Media"));
	CHECK(framework.initWindow("title=RendererTest;width=800;height=600;fullscreen=0;FSAA=4"));

	RenderWindow subWindow;
	subWindow.create("title=Sub-window;width=400;height=300;fullscreen=0;FSAA=4");

	ResourceManager& resourceManager = framework.resourceManager();

	Entity& root = framework.rootEntity();
	Entity& scene = framework.sceneLayer();
	RenderTargetComponent* textureRenderTarget = nullptr;
	RendererComponent* renderer = root.findComponentInChildrenExactType<RendererComponent>();
	CameraComponent2* sceneCamera = scene.findComponentInChildrenExactType<CameraComponent2>();

	{	// Setup sub-window
		EntityPtr e = new Entity("Sub-window render target");
		// NOTE: Due to some restriction on wglShareLists, the order of window
		// render target component does have a effect. So prefer the main window's
		// render target appear first in the Entity tree.
		e->insertAfter(&framework.guiLayer());

		RenderTargetComponent* subRenderTarget = new RenderTargetComponent;
		e->addComponent(subRenderTarget);
		subRenderTarget->clearColor = ColorRGBAf(0.3f, 0.3f, 0.3f, 0);
		subRenderTarget->window = &subWindow;
		subRenderTarget->entityToRender = &scene;
		subRenderTarget->cameraComponent = sceneCamera;
		subRenderTarget->rendererComponent = renderer;
	}

	{	// Setup render to texture
		EntityPtr e = new Entity("Texture render target");
		e->insertBefore(&scene);

		textureRenderTarget = new RenderTargetComponent;
		e->addComponent(textureRenderTarget);
		textureRenderTarget->clearColor = ColorRGBAf(0.3f, 0.6f, 0.9f, 1);
		textureRenderTarget->window = nullptr;
		textureRenderTarget->entityToRender = &scene;
		textureRenderTarget->cameraComponent = sceneCamera;
		textureRenderTarget->rendererComponent = renderer;
		textureRenderTarget->textures[0] = textureRenderTarget->createTexture(GpuDataFormat::get("uintRGBA8"), 512, 512);
	}

	{	// Setup GUI layer
		Entity* e = new Entity("Text");
		e->localTransform.setTranslation(Vec3f(0, 600, 0));
//		e->localTransform.setMat33(Mat33f::makeXYZRotation(0, 0, 3.14f/4));
		framework.guiLayer().addChild(e);
		TextLabelComponent* text = new TextLabelComponent;
		e->addComponent(text);
		text->text = "This is a text label\nMCore rocks!";
	}

	// Create material
	MaterialComponent* material1 = new MaterialComponent;
	scene.addComponent(material1);
	material1->diffuseColor = ColorRGBAf(1, 1, 0.5f, 1);
	material1->diffuseMap = dynamic_cast<Texture*>(resourceManager.load("InterlacedTrans256x256.png").get());
	material1->opacity = 1.0f;

	MaterialComponent* material2 = new MaterialComponent;
	material2->specularExponent = 120;
	material2->opacity = 0.8f;
	material2->diffuseMap = textureRenderTarget->textures[0];

	// Create mesh
	MeshComponent2* boxMesh = new MeshComponent2;
	boxMesh->mesh = new Mesh("");
	CHECK(boxMesh->mesh->create(ChamferBoxBuilder(0.5f, 5, true), Mesh::Static));

	Entity* boxes = new Entity("Boxes");
	scene.addChild(boxes);

	{	Entity* e = new Entity("Chamfer box1");
		boxes->addChild(e);
		e->addComponent(boxMesh);
		e->localTransform.translateBy(Vec3f(2, 0, -2));
	}

	{	Entity* e = new Entity("Chamfer box2");
		boxes->addChild(e);
		e->addComponent(boxMesh->clone());
		e->localTransform.translateBy(Vec3f(-2, 0, -2));
	}

	MeshComponent2* sphereMesh = new MeshComponent2;
	sphereMesh->mesh = new Mesh("");
	CHECK(sphereMesh->mesh->create(ChamferBoxBuilder(1, 5, true), Mesh::Static));

	Entity* spheres = new Entity("Spheres");
	scene.addChild(spheres);
	spheres->addComponent(material2);

	{	Entity* e = new Entity("Sphere1");
		spheres->addChild(e);
		e->addComponent(sphereMesh);
		e->localTransform.translateBy(Vec3f(2, 0, 2));
	}

	{	Entity* e = new Entity("Sphere2");
		spheres->addChild(e);
		e->addComponent(material2->clone());
		e->addComponent(sphereMesh->clone());
		e->localTransform.translateBy(Vec3f(-2, 0, 2));
	}

	while(true)
	{
		Event e;
		framework.update(e);

		if(e.Type == Event::Closed)
			break;

		subWindow.popEvent(e, false);

		const float dt = (float)timer.getDelta().asSecond();
		BehaviourComponent::traverseEntities(&root, dt);
		renderer->render(root);
	}

	CHECK(true);
}
