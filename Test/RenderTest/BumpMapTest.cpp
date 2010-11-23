#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Render/Chamferbox.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Renderer.h"
#include "../../MCD/Core/Entity/Entity.h"

using namespace MCD;

TEST(BumpMapTest)
{
	Framework framework;
	CHECK(framework.addFileSystem("Media"));
	CHECK(framework.initWindow("title=BumpMapTest;width=800;height=600;fullscreen=0;FSAA=4"));

	ResourceManager& resourceManager = framework.resourceManager();

	Entity& root = framework.rootEntity();
	Entity& scene = framework.sceneLayer();
	RendererComponent* renderer = root.findComponentInChildrenExactType<RendererComponent>();

	// Create material
	MaterialComponent* material1 = new MaterialComponent;
	scene.addComponent(material1);
	material1->diffuseMap = dynamic_cast<Texture*>(resourceManager.load("Fieldstone.jpg").get());
	material1->bumpMap = dynamic_cast<Texture*>(resourceManager.load("FieldstoneBump.jpg").get());

	// Create mesh
	MeshComponent* boxMesh = new MeshComponent;
	boxMesh->mesh = new Mesh("");
	CHECK(boxMesh->mesh->create(ChamferBoxBuilder(0.2f, 5, true), Mesh::Static));

	Entity* boxes = scene.addFirstChild("Boxes");
	boxes->localTransform.scaleBy(Vec3f(4, 4, 4));

	{	Entity* e = boxes->addFirstChild("Chamfer box1");
		e->addComponent(boxMesh);
		e->localTransform.translateBy(Vec3f(2, 0, -2));
	}

	{	Entity* e = boxes->addFirstChild("Chamfer box2");
		e->addComponent(boxMesh->clone());
		e->localTransform.translateBy(Vec3f(-2, 0, -2));
	}

	while(true)
	{
		Event e;
		framework.update(e);

		if(e.Type == Event::Closed)
			break;

		renderer->render(root);
	}

	CHECK(true);
}
