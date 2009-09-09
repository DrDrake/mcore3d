#include "Pch.h"
#include "../MCD/Binding/Launcher.h"
#include "../MCD/Component/Input/WinMessageInputComponent.h"
#include "../MCD/Core/System/MemoryProfiler.h"
#include "../MCD/Core/System/Path.h"
#include "../Test/RenderTest/BasicGlWindow.h"

using namespace MCD;

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=Launcher;width=800;height=600;fullscreen=0;FSAA=4")
	{
		MemoryProfiler::singleton().setEnable(true);
		mMemoryProfilerServer.listern(5001);

		WinMessageInputComponent* c = new WinMessageInputComponent();
		c->attachTo(*this);
		(void)mLauncher.init(*c);

		// TODO: Let user supply a command line argument to choose the startup script
		mLauncher.scriptComponentManager.doFile(L"init.nut", true);
	}

	sal_override ~TestWindow()
	{
	}

	sal_override void update(float deltaTime)
	{
		if(mTimer.get().asSecond() > 0.5) {
			mTimer.reset();
			mMemoryProfilerServer.accept();
			mMemoryProfilerServer.update();
		}

		mLauncher.update();
	}

protected:
	Launcher mLauncher;
	MemoryProfilerServer mMemoryProfilerServer;
	Timer mTimer;
};	// TestWindow

int main()
{
#ifdef MCD_VC
	// Tell the c-run time to do memory check at program shut down
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);
#endif

	TestWindow window;
	window.mainLoop();

	return 0;
}
