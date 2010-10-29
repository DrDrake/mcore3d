#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Audio/AudioSourceComponent.h"
#include "../../MCD/Audio/OggLoader.h"
#include "../../MCD/Render/Font.h"
#include "../../MCD/Render/Renderer.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Entity/InputComponent.h"
#include "../../MCD/Core/System/Timer.h"

using namespace MCD;

class ControllerComponent : public BehaviourComponent
{
public:
	ControllerComponent() : currentFileIdx(0) {}

	sal_override void update(float dt)
	{
		static const char* cFiles[] = {
			"mono.ogg",
			"stereo.ogg",
			"__not_exist_.ogg",
		};

		// Toogle looping
		if(input->getButtonDown("l"))
			audio->loop = !audio->loop;

		// Toggle play-pause
		if(input->getButtonDown("p"))
			audio->play = !audio->play;

		// Change volume
		if(input->getButton("Up")) {
			audio->volume += 1.0f * dt;
			audio->volume = audio->volume > 1 ? 1 : audio->volume;
		}
		if(input->getButton("Down")) {
			audio->volume -= 1.0f * dt;
			audio->volume = audio->volume < 0 ? 0 : audio->volume;
		}

		// Change file
		if(input->getButtonDown("n")) {
			++currentFileIdx;
			if(currentFileIdx >= MCD_COUNTOF(cFiles))
				currentFileIdx = 0;
		}

		audio->filePath = cFiles[currentFileIdx];

		char buf[512];
		sprintf(buf, "File: %s (n)\n%s (p)\nLoop (l): %s\nCurrent time: %4.2f\nvolume (up | down): %4.2f\n",
			audio->filePath.c_str(),
			audio->play ? "Playing..." : "Paused",
			audio->loop ? "yes" : "no",
			audio->time,
			audio->volume
		);
		text->text = buf;
	}

	size_t currentFileIdx;
	InputComponentPtr input;
	TextLabelComponentPtr text;
	AudioSourceComponentPtr audio;
};	// ControllerComponent

TEST(AudioSourceComponentTest)
{
	Framework framework;
	CHECK(framework.addFileSystem("./"));
	CHECK(framework.initWindow("title=AudioSourceComponentTest;width=400;height=300;fullscreen=0;FSAA=4"));

	Entity& root = framework.rootEntity();
	Entity& scene = framework.sceneLayer();
	RendererComponent* renderer = root.findComponentInChildrenExactType<RendererComponent>();

	// Find out the input component
	InputComponentPtr input;
	if(Entity* e = root.findEntityByPath("Input"))
		input = dynamic_cast<InputComponent*>(e->findComponent<BehaviourComponent>());

	{	// Register the ogg loader
		framework.addLoaderFactory(new OggLoaderFactory);
	}

	AudioSourceComponentPtr audio;
	{	// Setup for AudioSourceComponent
		Entity* e = scene.addFirstChild("Audio");
		audio = e->addComponent(new AudioSourceComponent);
	}

	TextLabelComponentPtr text;
	{	// Setup GUI layer
		Entity* e = framework.guiLayer().addFirstChild("Text");
		e->localTransform.setTranslation(Vec3f(0, 300, 0));
		text = e->addComponent(new TextLabelComponent);
	}

	{	// Setup the controller
		Entity* e = scene.addFirstChild("Controller");
		ControllerComponent* c = e->addComponent(new ControllerComponent);
		c->input = input;
		c->text = text;
		c->audio = audio;
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

