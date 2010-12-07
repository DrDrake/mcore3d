#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Render/Chamferbox.h"
#include "../../MCD/Render/Light.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/Entity/Entity.h"

using namespace MCD;

TEST(BumpMapTest)
{
	Framework framework;
	CHECK(framework.addFileSystem("Media"));
	CHECK(framework.initWindow("title=BumpMapTest;width=800;height=600;fullscreen=0;FSAA=4"));

	ResourceManager& resourceManager = framework.resourceManager();

	Entity& scene = framework.sceneLayer();

	// Create material
	MaterialComponent* material1 = new MaterialComponent;
	scene.addComponent(material1);
	material1->diffuseMap = dynamic_cast<Texture*>(resourceManager.load("Fieldstone.jpg").get());
	material1->bumpMap = dynamic_cast<Texture*>(resourceManager.load("FieldstoneBump.jpg").get());
	material1->bumpFactor = 0.2f;

	// Create mesh
	MeshComponent* boxMesh = new MeshComponent;
	boxMesh->mesh = new Mesh("");
	CHECK(boxMesh->mesh->create(ChamferBoxBuilder(0.2f, 5, true), Mesh::Static));

	Entity* boxes = scene.addFirstChild("Boxes");

	{	Entity* e = boxes->addFirstChild("Chamfer box1");
		e->addComponent(boxMesh);
		e->localTransform.translateBy(Vec3f(8, 0, 0));
		e->localTransform.scaleBy(Vec3f(4));
	}

	{	Entity* e = boxes->addFirstChild("Chamfer box2");
		e->addComponent(boxMesh->clone());
		e->localTransform.translateBy(Vec3f(-8, 0, 0));
		e->localTransform.scaleBy(Vec3f(4));
	}

	// Locate the camera
	CameraComponent* camera = scene.findComponentInChildrenExactType<CameraComponent>();
	camera->entity()->localTransform.translateBy(Vec3f(0, 0, 10));

	// Locate the default lights
	LightComponent* l1 = nullptr, *l2 = nullptr;
	for(EntityPreorderIterator itr(&scene); !itr.ended(); itr.next()) {
		if(LightComponent* l = itr->findComponentExactType<LightComponent>()) {
			if(!l1) l1 = l;
			else l2 = l;
		}
	}

	Timer timer;
	while(true)
	{
		// Move the lights
		float s = (float)sin(timer.get().asSecond());
		float c = (float)cos(timer.get().asSecond());

		l1->entity()->localTransform.setTranslation(10 * Vec3f(0, s, c));
		l2->entity()->localTransform.setTranslation(10 * Vec3f(s, 1, c));

		Event e;
		framework.update(e);
		if(e.Type == Event::Closed)
			break;
	}

	CHECK(true);
}
