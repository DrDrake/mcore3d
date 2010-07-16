#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../../MCD/Core/Entity/WinMessageInputComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/Window.h"
//#include "../../../MCD/Core/System/WindowEvent.h"

using namespace MCD;

namespace {


}	// namespace

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow("title=InputComponentTest;width=800;height=600;fullscreen=0;FSAA=4")
	{
		{	// Setup the input entity
			std::auto_ptr<Entity> e(new Entity);
			e->name = "Input entity";
			e->asChildOf(&mRootNode);

			WinMessageInputComponent* c = new WinMessageInputComponent();
			c->attachTo(*this);
			e->addComponent(c);
			mInputComponent = c;

			mInputEntity = e.release();
		}
	}

	sal_override void update(float deltaTime)
	{
		BehaviourComponent::traverseEntities(&mRootNode, 0.0f);
/*
		// Show the picking result
		for(size_t i=0; i<mPicker->hitCount(); ++i) {
			EntityPtr e = mPicker->hitAtIndex(i);
			if(!e)
				continue;
			std::wcout << e->name << std::endl;
		}
		mPicker->clearResult();
		mPickerEntity->enabled = false;*/
	}

	Entity mRootNode;
	EntityPtr mInputEntity;
	InputComponent* mInputComponent;
};	// TestWindow

TEST(InputComponentTest)
{
	TestWindow window;
	window.mainLoop();
}
