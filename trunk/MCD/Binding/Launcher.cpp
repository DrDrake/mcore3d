#include "Pch.h"
#include "Launcher.h"
#include "Entity.h"
#include "InputComponent.h"
#include "PhysicsComponent.h"
#include "System.h"
#include "../Component/Physics/RigidBodyComponent.h"
#include "../Component/Render/EntityPrototypeLoader.h"
#include "../Component/Render/RenderableComponent.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/Resource.h"
#include "../Core/System/ResourceLoader.h"
#include "../Render/ResourceLoaderFactory.h"
#include "../../3Party/jkbind/Declarator.h"

// For sqdbg
#include "../../3Party/squirrel/sqdbg/sqrdbg.h"
#include "../../3Party/squirrel/sqdbg/sqdbgserver.h"
#include "../../3Party/squirrel/sqstdaux.h"

using namespace MCD;

namespace {

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

}	// namespace

namespace script {

SCRIPT_CLASS_DECLAR(Launcher::FrameTimer);
SCRIPT_CLASS_DECLAR(ResourceLoadCallback);
SCRIPT_CLASS_DECLAR(Launcher);

SCRIPT_CLASS_REGISTER_NAME(Launcher::FrameTimer, "FrameTimer")
	.enableGetset()
	.method(L"_getframeTime", &Launcher::FrameTimer::frameTime)
	.method(L"_getaccumulateTime", &Launcher::FrameTimer::accumulateTime)
	.method(L"_getfps", &Launcher::FrameTimer::fps)
;}

SCRIPT_CLASS_REGISTER(ResourceLoadCallback)
	.declareClass<ResourceLoadCallback, ResourceManagerCallback>(L"ResourceLoadCallback")
	.constructor()
	.scriptEvent(L"onLoaded", &ResourceLoadCallback::onLoaded)
;}

SCRIPT_CLASS_REGISTER_NAME(Launcher, "MainWindow")
	.enableGetset()
	.method<objNoCare>(L"_getrootEntity", &Launcher::rootNode)
	.method(L"loadEntity", &Launcher::loadEntity)
	.method<objNoCare>(L"_getinputComponent", &Launcher::inputComponent)
	.method<objNoCare>(L"_getresourceManager", &Launcher::resourceManager)
	.method<objNoCare>(L"_getdynamicsWorld", &Launcher::dynamicsWorld)
	.method<objNoCare>(L"_getframeTimer", &Launcher::frameTimer)
//	.rawMethod(L"addCallback", &Launcher::addCallback)
;}

}	// namespace script

Launcher* Launcher::mSingleton = nullptr;

Launcher::Launcher(IFileSystem& fileSystem, IResourceManager& resourceManager, bool takeResourceManagerOwnership)
	:
	mFileSystem(fileSystem),
	mRootNode(nullptr),
	mInputComponent(nullptr),
	mDbgContext(nullptr),
	mResourceManager(&resourceManager),
	mTakeResourceManagerOwnership(takeResourceManagerOwnership)
{
	// Start the physics thread
	mPhysicsThread.start(mDynamicsWorld, false);

	mSingleton = this;
}

Launcher::~Launcher()
{
	// NOTE: All components should be destroyed before script VM release those handles,
	// because these components use co-routine which have it's own VM. Otherwise the
	// ScriptOwnershipHandle would refernece an already destroyed VM.
	// Also make sure the RigidBodyComponent is freed BEFORE the dynamics world...
	if(mRootNode) while(mRootNode->firstChild())
		delete mRootNode->firstChild();
	delete mRootNode;

	// Give the script engine a chance to do cleanups
//	scriptComponentManager.updateScriptComponents();

	scriptComponentManager.shutdown();

	// Stop the physics thread
	mPhysicsThread.postQuit();
	mPhysicsThread.wait();

	if(mTakeResourceManagerOwnership)
		delete mResourceManager;
}

// TODO: Separate the wait connection from this function, such that the client will
// able to connect to this debugging server multiple times.
bool Launcher::enableDebugger(size_t port, bool blockUntilConnected)
{
	HSQUIRRELVM v = HSQUIRRELVM(vm.getImplementationHandle());
	sqstd_seterrorhandlers(v);

	// Initializes the debugger on the specific port 4321, enables autoupdate
	HSQREMOTEDBG rdbg = sq_rdbg_init(v, port, SQFalse);
	mDbgContext = rdbg;

	if(!rdbg)
		return false;

	// Enables debug info generation (for the compiler)
	sq_enabledebuginfo(v, SQTrue);

	if(blockUntilConnected) {
		// Suspends current thread until the debugger client connects
		return SQ_SUCCEEDED(sq_rdbg_waitforconnections(rdbg));
	}
	else
		return true;
}

bool Launcher::init(InputComponent& inputComponent, Entity* rootNode)
{
	using namespace script;

	scriptComponentManager.init(vm, mFileSystem);

	VMCore* v = (VMCore*)sq_getforeignptr(HSQUIRRELVM(vm.getImplementationHandle()));

	// TODO: Remove the need of a singleton
	script::RootDeclarator root(v);
	root.declareFunction<objNoCare>(L"_getlauncher", &Launcher::sinleton);

	script::ClassTraits<FrameTimer>::bind(v);
	script::ClassTraits<ResourceLoadCallback>::bind(v);
	script::ClassTraits<Launcher>::bind(v);

	// Setup some global variable for easy access in script.
	if(!vm.runScript(
		L"gLauncher <- _getlauncher();\n"

		L"function loadEntity(filePath, loadOptions={}) {\n"
		L"	if(\"createStaticRigidBody\" in loadOptions && loadOptions[\"createStaticRigidBody\"])\n"
		L"		return gLauncher.loadEntity(filePath, true);\n"
		L"	return gLauncher.loadEntity(filePath, false);\n"
		L"}\n"

		L"resourceManager <- gLauncher.resourceManager;\n"
		L"gFrameTimer <- gLauncher.frameTimer;\n"

		// TODO: Use named parameter to pass blocking and priority options
		L"function loadResource(filePath) {\n"
		L"	return resourceManager.load(filePath, false, 0, \"\");\n"
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
	))
		return false;

	// Patch the original RigidBodyComponent constructor to pass our dynamics world automatically
	if(!vm.runScript(L"\
		local backup = RigidBodyComponent.constructor;\n\
		RigidBodyComponent.constructor <- function(mass, collisionShape) : (backup) {\n\
			backup.call(this, gLauncher.dynamicsWorld, mass, collisionShape);	// Call the original constructor\n\
		}\n"
	))
		return false;

	if(rootNode)
		setRootNode(rootNode);
	else
		setRootNode(new Entity());

	setInputComponent(&inputComponent);

	return true;
}

Entity* Launcher::loadEntity(const wchar_t* filePath, bool createCollisionMesh)
{
	MemoryProfiler::Scope profiler("Launcher::loadEntity");

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

	std::auto_ptr<Entity> e(new Entity());
	e->name = filePath;
	EntityPrototypeLoader::addEntityAfterLoad(e.get(), *mResourceManager, filePath,
		createCollisionMesh ? new EntityLoadCreatePhysicsCallback(mDynamicsWorld) : nullptr,
		0, L"loadAsEntity=true"
	);

	return e.release();
}

void Launcher::update()
{
	MemoryProfiler::Scope profiler("Launcher::update");

	HSQREMOTEDBG rdbg = reinterpret_cast<HSQREMOTEDBG>(mDbgContext);
	sq_rdbg_update(rdbg);
	mResourceManager->update();

	scriptComponentManager.updateScriptComponents();

	BehaviourComponent::traverseEntities(mRootNode);
	RenderableComponent::traverseEntities(mRootNode);

	mFrameTimer.nextFrame();
}

void Launcher::setRootNode(Entity* e)
{
	mRootNode = e;

	if(!e)
		return;

	mRootNode->name = L"Launcher root node";
	scriptComponentManager.registerRootEntity(*e);
}

InputComponent* Launcher::inputComponent() {
	return mInputComponent.get();
}

void Launcher::setInputComponent(InputComponent* inputComponent)
{
	mInputComponent = inputComponent;
	if(!mRootNode || !inputComponent)
		return;

	if(inputComponent->entity() == nullptr) {
		std::auto_ptr<Entity> e(new Entity);
		e->name = L"Input";
		e->asChildOf(mRootNode);
		e->addComponent(inputComponent);
		e.release();
	}

	MCD_VERIFY(vm.runScript(L"gInput <- gLauncher.inputComponent;\n"));
}

LauncherDefaultResourceManager::LauncherDefaultResourceManager(IFileSystem& fileSystem, bool takeFileSystemOwnership)
	: ResourceManager(fileSystem, takeFileSystemOwnership)
{
	addFactory(new BitmapLoaderFactory);
	addFactory(new DdsLoaderFactory);
	addFactory(new EffectLoaderFactory(*this));
	addFactory(new JpegLoaderFactory);
	addFactory(new Max3dsLoaderFactory(*this));
	addFactory(new PodLoaderFactory(*this));
	addFactory(new PixelShaderLoaderFactory);
	addFactory(new PngLoaderFactory);
	addFactory(new TgaLoaderFactory);
	addFactory(new VertexShaderLoaderFactory);
	addFactory(new Max3dsLoaderFactory(*this));
	addFactory(new EntityPrototypeLoaderFactory(*this));

	// CubemapLoader must be added last (have a higher priority than other image loader factory)
	addFactory(new CubemapLoaderFactory);
}

int LauncherDefaultResourceManager::update()
{
	ResourceManager::Event e = popEvent();
	if(e.loader) {
		bool hasError = e.loader->getLoadingState() == IResourceLoader::Aborted;

		if(hasError)
			Log::format(Log::Warn, L"Resource: %s %s", e.resource->fileId().getString().c_str(), L"failed to load");
		else	// Allow at most one resource to commit at each time
			e.loader->commit(*e.resource);

		// Note that commit() is invoked before doCallbacks()
		doCallbacks(e);

		return hasError ? -1 : 1;
	}

	return 0;
}
