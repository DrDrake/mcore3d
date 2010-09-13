#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Audio/AudioSourceComponent.h"
#include "../../MCD/Audio/OggLoader.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/Timer.h"

using namespace MCD;

TEST(AudioSourceComponentTest)
{
	DeltaTimer timer;
	Framework framework;
	CHECK(framework.addFileSystem("./"));

	Entity& root = framework.rootEntity();
	Entity& scene = framework.sceneLayer();

	{	// Register the ogg loader
		framework.addLoaderFactory(new OggLoaderFactory);
	}

	{	// Setup for AudioSourceComponent
		Entity* e = scene.addChild(new Entity("Audio"));
		AudioSourceComponent* c = e->addComponent(new AudioSourceComponent);
		c->filePath = "stereo.ogg";
		c->loop = false;
	}

	while(true)
	{
		Event e;
		framework.update(e);

		if(e.Type == Event::Closed)
			break;

		const float dt = (float)timer.getDelta().asSecond();
		BehaviourComponent::traverseEntities(&root, dt);
		AudioComponent::traverseEntities(&scene);
	}

	CHECK(true);
}

