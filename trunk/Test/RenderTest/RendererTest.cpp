#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Loader/ResourceLoaderFactory.h"
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
#include "../../MCD/Core/System/WindowEvent.h"

using namespace MCD;

TEST(RendererTest)
{
	RenderWindow subWindow;
	DeltaTimer timer;
	Framework framework;
	framework.addFileSystem("Media");
	framework.initWindow("title=RendererTest;width=800;height=600;fullscreen=0;FSAA=4");
	subWindow.create("title=Sub-window;width=400;height=300;fullscreen=0;FSAA=4");

	ResourceManager& resourceManager = framework.resourceManager();

	{	// Register various resource loaders
		framework.addLoaderFactory(new FntLoaderFactory(resourceManager));
		framework.addLoaderFactory(new PngLoaderFactory);
	}

	Entity& root = framework.rootEntity();
	Entity& scene = framework.sceneLayer();
	RenderTargetComponent* textureRenderTarget = nullptr;
	RendererComponent* renderer = root.findComponentInChildrenExactType<RendererComponent>();
	CameraComponent2* sceneCamera = scene.findComponentInChildrenExactType<CameraComponent2>();


	{	// Setup sub-window
		RenderTargetComponent* subRenderTarget = new RenderTargetComponent;
		subRenderTarget->clearColor = ColorRGBAf(0.3f, 0.3f, 0.3f, 0);
		subRenderTarget->window = &subWindow;
		subRenderTarget->entityToRender = &scene;
		subRenderTarget->cameraComponent = sceneCamera;
		subRenderTarget->rendererComponent = renderer;

		EntityPtr e = new Entity("Sub-window render target");
		// NOTE: Due to some restriction on wglShareLists, the order of window
		// render target component does have a effect. So prefer the main window's
		// render target appear first in the Entity tree.
		e->insertAfter(&framework.guiLayer());
		e->addComponent(subRenderTarget);
	}

	{	// Setup render to texture
		textureRenderTarget = new RenderTargetComponent;
		textureRenderTarget->clearColor = ColorRGBAf(0.3f, 0.6f, 0.9f, 1);
		textureRenderTarget->window = nullptr;
		textureRenderTarget->entityToRender = &scene;
		textureRenderTarget->cameraComponent = sceneCamera;
		textureRenderTarget->rendererComponent = renderer;
		textureRenderTarget->textures[0] = textureRenderTarget->createTexture(GpuDataFormat::get("uintRGBA8"), 512, 512);

		EntityPtr e = new Entity("Texture render target");
		e->insertBefore(&scene);
		e->addComponent(textureRenderTarget);
	}

	{	// Setup GUI layer
		BmpFontMaterialComponent* material = new BmpFontMaterialComponent;
		material->bmpFont = dynamic_cast<BmpFont*>(resourceManager.load("Font-Arial.fnt").get());
		framework.guiLayer().addComponent(material);

		Entity* e = new Entity("Text");
		e->localTransform.setTranslation(Vec3f(0, 600, 0));
//		e->localTransform.setMat33(Mat33f::makeXYZRotation(0, 0, 3.14f/4));
		e->asChildOf(&framework.guiLayer());
		TextLabelComponent* font = new TextLabelComponent;
		e->addComponent(font);
		font->text = "This is a text label\nMCore rocks!";
	}

	// Create material
	MaterialComponent* material1 = new MaterialComponent;
	material1->diffuseColor = ColorRGBAf(1, 1, 0.5f, 1);
	material1->diffuseMap = dynamic_cast<Texture*>(resourceManager.load("InterlacedTrans256x256.png").get());
	material1->opacity = 1.0f;

	MaterialComponent* material2 = new MaterialComponent;
	material2->specularExponent = 120;
	material2->opacity = 0.8f;
	material2->diffuseMap = textureRenderTarget->textures[0];

	scene.addComponent(material1);

	// Create mesh
	MeshComponent2* boxMesh = new MeshComponent2;
	boxMesh->mesh = new Mesh("");
	CHECK(boxMesh->mesh->create(ChamferBoxBuilder(0.5f, 5, true), Mesh::Static));

	EntityPtr boxes = new Entity("Boxes");
	boxes->asChildOf(&scene);

	{	Entity* e = new Entity("Chamfer box1");
		e->asChildOf(boxes.get());
		e->addComponent(boxMesh);
		e->localTransform.translateBy(Vec3f(2, 0, -2));
	}

	{	Entity* e = new Entity("Chamfer box2");
		e->asChildOf(boxes.get());
		e->addComponent(boxMesh->clone());
		e->localTransform.translateBy(Vec3f(-2, 0, -2));
	}

	MeshComponent2* sphereMesh = new MeshComponent2;
	sphereMesh->mesh = new Mesh("");
	CHECK(sphereMesh->mesh->create(ChamferBoxBuilder(1, 5, true), Mesh::Static));

	EntityPtr spheres = new Entity("Spheres");
	spheres->addComponent(material2);
	spheres->asChildOf(&scene);

	{	Entity* e = new Entity("Sphere1");
		e->asChildOf(spheres.get());
		e->addComponent(sphereMesh);
		e->localTransform.translateBy(Vec3f(2, 0, 2));
	}

	{	Entity* e = new Entity("Sphere2");
		e->asChildOf(spheres.get());
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
