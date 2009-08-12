#include "Pch.h"
#include "../MCD/Binding/Binding.h"
#include "../MCD/Binding/Entity.h"
#include "../MCD/Binding/InputComponent.h"
#include "../MCD/Binding/PhysicsComponent.h"
#include "../MCD/Binding/ScriptComponentManager.h"
#include "../MCD/Binding/System.h"
#include "../MCD/Core/Entity/Entity.h"
#include "../MCD/Core/System/FileSystemCollection.h"
#include "../MCD/Core/System/RawFileSystem.h"
#include "../MCD/Render/ChamferBox.h"
#include "../MCD/Render/Effect.h"
#include "../MCD/Render/Mesh.h"
#include "../MCD/Component/Physics/RigidBodyComponent.h"
#include "../MCD/Component/Physics/ThreadedDynamicWorld.h"
#include "../MCD/Component/Render/EntityPrototypeLoader.h"
#include "../MCD/Component/Render/CameraComponent.h"
#include "../MCD/Component/Render/MeshComponent.h"
#include "../MCD/Component/Input/WinMessageInputComponent.h"
#include "../Test/RenderTest/BasicGlWindow.h"
#include "../Test/RenderTest/DefaultResourceManager.h"

#ifdef MCD_VC
#	ifndef NDEBUG
#		pragma comment(lib, "jkbindd")
#		pragma comment(lib, "squirreld")
#	else
#		pragma comment(lib, "jkbind")
#		pragma comment(lib, "squirrel")
#	endif
#endif

using namespace MCD;

//! Call a script function when a list of resources were loaded.
class ResourceLoadCallback : public ResourceManagerCallback
{
public:
	sal_override void doCallback()
	{
		if(onLoaded.isAssigned())
			onLoaded.call();
	}

	script::Event<void> onLoaded;
};	// Callback

//! A callback that create static physics collision mesh upon loads complete.
class EntityLoadCreatePhysicsCallback : public EntityPrototypeLoader::LoadCallback
{
public:
	EntityLoadCreatePhysicsCallback(DynamicsWorld& dynamicsWorld) : mDynamicsWorld(dynamicsWorld) {}

	sal_override void doCallback()
	{
		// Call the parent's doCallback() first
		LoadCallback::doCallback();

		// Create physics components
		if(entityAdded)
			createStaticRigidBody(mDynamicsWorld, *entityAdded);
	}

protected:
	DynamicsWorld& mDynamicsWorld;
};	// EntityLoadCreatePhysicsCallback

//! A timer to measure the fps
class FrameTimer : protected DeltaTimer
{
public:
	FrameTimer() : DeltaTimer(TimeInterval(1.0/60)) {}

	float frameTime() const { return float(mFrameTime.asSecond()); }
	float accumulateTime() const { return float(mAccumulateTime.asSecond()); }
	float fps() const { return 1.0f / frameTime(); }
	void nextFrame() { mFrameTime = getDelta(); mAccumulateTime = mTimer.get(); }

protected:
	TimeInterval mFrameTime;
	TimeInterval mAccumulateTime;
};	// FrameTimer

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=Launcher;width=800;height=600;fullscreen=0;FSAA=4"),
		fileSystem(*createDefaultFileSystem()),
		mResourceManager(fileSystem),
		mScriptComponentManager(fileSystem)
	{
		MCD_ASSERT(TestWindow::singleton == nullptr);
		TestWindow::singleton = this;

		FileSystemCollection& fs = dynamic_cast<FileSystemCollection&>(fileSystem);
		fs.addFileSystem(*(new RawFileSystem(L"")));

		// Override the default loader of *.3ds file
		mResourceManager.addFactory(new EntityPrototypeLoaderFactory(mResourceManager));

		// Start the physics thread
		mPhysicsThread.start(mDynamicsWorld, false);

		mRootNode = new Entity();
		mRootNode->name = L"root";

		{	// Setup input component
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"Input";
			e->asChildOf(mRootNode);

			WinMessageInputComponent* c = new WinMessageInputComponent();
			mInputComponent = c;
			c->attachTo(*this);
			e->addComponent(c);

			e.release();
		}
	}

	sal_override ~TestWindow()
	{
//		mScriptComponentManager.shutdown();

		// Make sure the RigidBodyComponent is freed BEFORE the dynamics world...
		while(mRootNode->firstChild())
			delete mRootNode->firstChild();

		// Stop the physics thread
		mPhysicsThread.postQuit();
		mPhysicsThread.wait();

		// Give the script engine a chance to do cleanups
//		mScriptComponentManager.updateScriptComponents();

		// The Entity tree must be destroyed before the script VM.
		delete mRootNode;
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		mScriptComponentManager.updateScriptComponents();

		BehaviourComponent::traverseEntities(mRootNode);
		RenderableComponent::traverseEntities(mRootNode);

		mFrameTimer.nextFrame();
	}

// Script usage:
	void scriptBindingSetup();

	sal_notnull Entity* rootNode() {
		return mRootNode;
	}

	sal_notnull Entity* loadEntity(const wchar_t* filePath, bool createCollisionMesh)
	{
		Entity* e = new Entity();
		EntityPrototypeLoader::addEntityAfterLoad(e, mResourceManager, filePath,
			createCollisionMesh ? new EntityLoadCreatePhysicsCallback(mDynamicsWorld) : nullptr
		);
		return e;
	}

	sal_notnull InputComponent* inputComponent() {
		return mInputComponent;
	}

	sal_notnull IResourceManager* resourceManager() {
		return &mResourceManager;
	}

	sal_notnull ThreadedDynamicsWorld* dynamicsWorld() {
		return &mDynamicsWorld;
	}

	sal_notnull FrameTimer* frameTimer() {
		return &mFrameTimer;
	}

	static TestWindow* getSinleton() {
		return TestWindow::singleton;
	}

	static TestWindow* singleton;

	//! Perform callback when the supplied list of resource get loaded
	static int addCallback(HSQUIRRELVM vm)
	{
		script::detail::StackHandler sa(vm);
		int nparams = sa.getParamCount();

		static const wchar_t errorMsg[] = L"addCallback() expecting a string or an array of string as parameter";

		if(nparams < 1)
			return sa.throwError(errorMsg);

		return 1;
	}

// Attributes:
	Entity* mRootNode;
	InputComponent* mInputComponent;

	FrameTimer mFrameTimer;
	IFileSystem& fileSystem;
	DefaultResourceManager mResourceManager;
	Thread mPhysicsThread;
	ThreadedDynamicsWorld mDynamicsWorld;
	ScriptComponentManager mScriptComponentManager;
};	// TestWindow

TestWindow* TestWindow::singleton = nullptr;

namespace script {

SCRIPT_CLASS_DECLAR(FrameTimer);
SCRIPT_CLASS_DECLAR(ResourceLoadCallback);
SCRIPT_CLASS_DECLAR(TestWindow);

SCRIPT_CLASS_REGISTER_NAME(FrameTimer, "FrameTimer")
	.enableGetset(L"FrameTimer")
	.method(L"_getframeTime", &FrameTimer::frameTime)
	.method(L"_getaccumulateTime", &FrameTimer::accumulateTime)
	.method(L"_getfps", &FrameTimer::fps)
;}

SCRIPT_CLASS_REGISTER(ResourceLoadCallback)
	.declareClass<ResourceLoadCallback, IResourceManagerCallback>(L"ResourceLoadCallback")
	.constructor()
	.scriptEvent(L"onLoaded", &ResourceLoadCallback::onLoaded)
;}

SCRIPT_CLASS_REGISTER_NAME(TestWindow, "MainWindow")
	.enableGetset(L"MainWindow")
	.method<objNoCare>(L"_getrootEntity", &TestWindow::rootNode)
	.method(L"loadEntity", &TestWindow::loadEntity)
	.method<objNoCare>(L"_getinputComponent", &TestWindow::inputComponent)
	.method<objNoCare>(L"_getresourceManager", &TestWindow::resourceManager)
	.method<objNoCare>(L"_getdynamicsWorld", &TestWindow::dynamicsWorld)
	.method<objNoCare>(L"_getframeTimer", &TestWindow::frameTimer)
	.rawMethod(L"addCallback", &TestWindow::addCallback)
;}

}	// namespace script

void TestWindow::scriptBindingSetup()
{
	mScriptComponentManager.registerRootEntity(*mRootNode);

	using namespace script;

	VMCore* v = (VMCore*)sq_getforeignptr(HSQUIRRELVM(mScriptComponentManager.vm.getImplementationHandle()));

	script::RootDeclarator root(v);
	root.declareFunction<objNoCare>(L"_getMainWindow", &TestWindow::getSinleton);

	script::ClassTraits<FrameTimer>::bind(v);
	script::ClassTraits<ResourceLoadCallback>::bind(v);
	script::ClassTraits<TestWindow>::bind(v);

	// Setup some global variable for easy access in script.
	mScriptComponentManager.vm.runScript(
		L"gMainWindow <- _getMainWindow();\n"

		L"function loadEntity(filePath, loadOptions={}) {\n"
		L"	if(\"createStaticRigidBody\" in loadOptions && loadOptions[\"createStaticRigidBody\"])\n"
		L"		return gMainWindow.loadEntity(filePath, true);\n"
		L"	return gMainWindow.loadEntity(filePath, false);\n"
		L"}\n"

		L"gInput <- gMainWindow.inputComponent;\n"
		L"resourceManager <- gMainWindow.resourceManager;\n"
		L"gFrameTimer <- gMainWindow.frameTimer;\n"

		// TODO: Use named parameter to pass blocking and priority options
		L"function loadResource(filePath) {\n"
		L"	return resourceManager.load(filePath, false, 0);\n"
		L"}\n"

		L"function addResourceCallback(filePaths, functor) {\n"
		L"	local callback = ResourceLoadCallback();\n"
		L"	if(typeof filePaths == \"string\")\n"
		L"		callback.addDependency(filePaths);\n"
		L"	else foreach(path in filePaths)\n"
		L"		callback.addDependency(path);\n"
		L"	callback.onLoaded().setHandler(functor);\n"
		L"	resourceManager.addCallback(callback);\n"
		L"}\n"
	);

	// Patch the original RigidBodyComponent constructor to pass our dynamics world automatically
	mScriptComponentManager.vm.runScript(L"\
		local backup = RigidBodyComponent.constructor;\n\
		RigidBodyComponent.constructor <- function(mass, collisionShape) : (backup) {\n\
			backup.call(this, gMainWindow.dynamicsWorld, mass, collisionShape);	// Call the original constructor\n\
		}\n"
	);

	// TODO: Let user supply a command line argument to choose the startup script
	mScriptComponentManager.doFile(L"init.nut", true);
}

int main()
{
#ifdef MCD_VC
	// Tell the c-run time to do memory check at program shut down
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);
#endif

	TestWindow window;
	window.scriptBindingSetup();
	window.mainLoop();

	return 0;
}
