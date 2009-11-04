#include "Pch.h"
#include "../MCD/Audio/AudioDevice.h"
#include "../MCD/Binding/Launcher.h"
#include "../MCD/Component/Input/WinMessageInputComponent.h"
#include "../MCD/Core/System/FileSystemCollection.h"
#include "../MCD/Core/System/MemoryProfiler.h"
#include "../MCD/Core/System/Path.h"
#include "../MCD/Core/System/RawFileSystem.h"
#include "../MCD/Core/System/ZipFileSystem.h"
#include "../Test/RenderTest/BasicGlWindow.h"

using namespace MCD;

namespace {

IFileSystem* createDefaultFileSystem()
{
	std::auto_ptr<FileSystemCollection> fileSystem(new FileSystemCollection);

	Path actualRoot;

	std::auto_ptr<IFileSystem> rawFs1(new RawFileSystem(L""));
	actualRoot = rawFs1->getRoot();
	fileSystem->addFileSystem(*rawFs1.release());

	try {
		std::auto_ptr<IFileSystem> rawFs2(new RawFileSystem(L"Media"));
		actualRoot = rawFs2->getRoot();
		fileSystem->addFileSystem(*rawFs2.release());
	} catch(...) {}

	try {
		std::auto_ptr<IFileSystem> zipFs(new ZipFileSystem(actualRoot.getBranchPath() / L"Media.zip"));
		fileSystem->addFileSystem(*zipFs.release());
	} catch(...) {}

	return fileSystem.release();
}

}	// namespace

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=Launcher;width=800;height=600;fullscreen=0;FSAA=4")
	{
		initAudioDevice();

		IFileSystem* fs = createDefaultFileSystem();
		std::auto_ptr<IResourceManager> mgr(new LauncherDefaultResourceManager(*fs, true));
		mLauncher.reset(new Launcher(*fs, *mgr.release(), true));

		MemoryProfiler::singleton().setEnable(true);
		mMemoryProfilerServer.listern(5001);

		WinMessageInputComponent* c = new WinMessageInputComponent();
		c->attachTo(*this);
		(void)mLauncher->enableDebugger(4321, false);
		(void)mLauncher->init(*c);

		// TODO: Let user supply a command line argument to choose the startup script
		(void)mLauncher->scriptComponentManager.doFile(L"scene.nut", true);
		(void)mLauncher->scriptComponentManager.doFile(L"init.nut", true);
	}

	sal_override ~TestWindow()
	{
		closeAudioDevice();
	}

	sal_override void update(float deltaTime)
	{
		if(mTimer.get().asSecond() > 0.5) {
			mTimer.reset();
			mMemoryProfilerServer.accept();
			mMemoryProfilerServer.update();
		}

		mLauncher->update();
	}

protected:
	std::auto_ptr<Launcher> mLauncher;
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
