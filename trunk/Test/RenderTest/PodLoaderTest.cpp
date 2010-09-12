#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Loader/PodLoader.h"
#include "../../MCD/Render/Chamferbox.h"
#include "../../MCD/Render/Font.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Renderer.h"
#include "../../MCD/Render/RenderTargetComponent.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/WindowEvent.h"

using namespace MCD;

TEST(PodLoaderTest)
{
	DeltaTimer timer;
	Framework framework;
	CHECK(framework.addFileSystem("Media"));
	CHECK(framework.initWindow("title=RendererTest;width=800;height=600;fullscreen=0;FSAA=4"));

	ResourceManager& resourceManager = framework.resourceManager();

	Entity& root = framework.rootEntity();
	Entity& scene = framework.sceneLayer();
	RendererComponent* renderer = root.findComponentInChildrenExactType<RendererComponent>();
//	CameraComponent* sceneCamera = scene.findComponentInChildrenExactType<CameraComponent>();

	{	// Register various resource loaders
		framework.addLoaderFactory(new PodLoaderFactory);
//		framework.addLoaderFactory(new PvrLoaderFactory);
//		framework.addLoaderFactory(new TgaLoaderFactory);
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

	{	// Load the model using prefab
		Entity* e = new Entity("Model prefab");
		scene.addChild(e);
//		e->localTransform.setScale(Vec3f(0.2f));
		CHECK(framework.loadPrefabTo("Scene/scene.pod", e));
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

	// Create mesh
	MeshComponent* boxMesh = new MeshComponent;
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

	MeshComponent* sphereMesh = new MeshComponent;
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

		const float dt = (float)timer.getDelta().asSecond();
		BehaviourComponent::traverseEntities(&root, dt);
		renderer->render(root);
	}

	CHECK(true);
}
