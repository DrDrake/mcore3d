#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/WindowEvent.h"
#include "../../MCD/Core/System/ResourceLoader.h"
#include "../../MCD/Core/System/RawFileSystem.h"

using namespace MCD;

TEST(RendererTest)
{
	DefaultResourceManager resourceManager(*new RawFileSystem("Media"));

	RenderWindow window;
	window.create("title=RendererTest;width=800;height=600;fullscreen=0;FSAA=4");

	while(true) {
		Event e;
		// check for window events
		if(window.popEvent(e, false) && e.Type == Event::Closed)
			break;

		resourceManager.processLoadingEvents();
		window.preUpdate();
		window.postUpdate();
	}

	CHECK(true);
}
