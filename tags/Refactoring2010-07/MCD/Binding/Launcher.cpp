#include "Pch.h"
#include "Launcher.h"
#include "Entity.h"
#include "InputComponent.h"
#include "PhysicsComponent.h"
#include "RenderComponent.h"
#include "System.h"
#include "../Audio/ResourceLoaderFactory.h"
#include "../Component/PrefabLoaderComponent.h"
#include "../Component/Audio/AudioSourceComponent.h"
#include "../Component/Physics/RigidBodyComponent.h"
#include "../Component/Render/AnimationComponent.h"
#include "../Component/Render/EntityPrototypeLoader.h"
#include "../Component/Render/RenderableComponent.h"
#include "../Component/Render/SkeletonAnimationComponent.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/Resource.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/ThreadedCpuProfiler.h"
#include "../Loader/PodLoader.h"
#include "../Loader/PvrLoader.h"
#include "../Loader/ResourceLoaderFactory.h"
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

SCRIPT_CLASS_DECLAR(ResourceLoadCallback);
SCRIPT_CLASS_DECLAR(Launcher);

SCRIPT_CLASS_REGISTER(ResourceLoadCallback)
	.declareClass<ResourceLoadCallback, ResourceManagerCallback>("ResourceLoadCallback")
	.constructor()
	.scriptEvent("onLoaded", &ResourceLoadCallback::onLoaded)
;}

SCRIPT_CLASS_REGISTER_NAME(Launcher, "MainWindow")
	.enableGetset()
	.method<objNoCare>("_getrootEntity", &Launcher::rootNode)
	.method("loadEntity", &Launcher::loadEntity)
	.method("loadEntity2", &Launcher::loadEntity2)
	.method<objNoCare>("_getinputComponent", &Launcher::inputComponent)
	.method<objNoCare>("_getresourceManager", &Launcher::resourceManager)
	.method<objNoCare>("_getdynamicsWorld", &Launcher::dynamicsWorld)
	.method<objNoCare>("_getanimationUpdater", &Launcher::animationUpdater)
	.method<objNoCare>("_getskeletonAnimationUpdater", &Launcher::skeletonAnimationUpdater)
//	.rawMethod("addCallback", &Launcher::addCallback)
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
	taskPool.setThreadCount(1);

	mSingleton = this;
}

Launcher::~Launcher()
{
	// NOTE: All components should be destroyed before script VM release those handles,
	// because these components use co-routine which have it's own VM. Otherwise the
	// ScriptOwnershipHandle would refernece an already destroyed VM.
	// Also make sure the RigidBodyComponent is freed BEFORE the dynamics world...
	if(mRootNode) {
		while(Entity* c = mRootNode->firstChild())
			c->destroyThis();
		mRootNode->destroyThis();
	}

	// Give the script engine a chance to do cleanups
//	scriptComponentManager.updateScriptComponents();

	scriptComponentManager.shutdown();

	// Stop the physics thread
	mPhysicsThread.postQuit();
	mPhysicsThread.wait();

	if(mTakeResourceManagerOwnership)
		delete mResourceManager;

	if(mDbgContext)
		sq_rdbg_shutdown(HSQREMOTEDBG(mDbgContext));
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

	if(!scriptComponentManager.init(vm, mFileSystem))
		return false;

	VMCore* v = (VMCore*)sq_getforeignptr(HSQUIRRELVM(vm.getImplementationHandle()));

	// TODO: Remove the need of a singleton
	script::RootDeclarator root(v);
	root.declareFunction<objNoCare>("_getlauncher", &Launcher::sinleton);

	script::ClassTraits<ResourceLoadCallback>::bind(v);
	script::ClassTraits<Launcher>::bind(v);

	// Setup some global variable for easy access in script.
	if(!vm.runScript(
		"gLauncher <- _getlauncher();\n"

		"function loadEntity(filePath, loadOptions={}) {\n"
		"	local args = \"\";\n"
		"	if(\"createCollisionMesh\" in loadOptions && loadOptions[\"createCollisionMesh\"])\n"
		"		args += \";createCollisionMesh=true\";\n"
		"	if(\"anim\" in loadOptions)\n"
		"		foreach(a in loadOptions[\"anim\"])\n"
		"			args += \";anim=\"+a;\n"
		"	return gLauncher.loadEntity(filePath, args);\n"
		"}\n"

		"function loadEntity2(filePath, loadOptions={}) {\n"
		"	local args = \"\";\n"
		"	if(\"createCollisionMesh\" in loadOptions && loadOptions[\"createCollisionMesh\"])\n"
		"		args += \";createCollisionMesh=true\";\n"
		"	return gLauncher.loadEntity2(filePath, args);\n"
		"}\n"

		"resourceManager <- gLauncher.resourceManager;\n"

		// TODO: Use named parameter to pass blocking and priority options
		"function loadResource(filePath) {\n"
		"	return resourceManager.load(filePath, false, 0, \"\");\n"
		"}\n"

		"function addResourceCallback(filePaths, functor) {\n"
		"	local callback = ResourceLoadCallback();\n"
		"	if(typeof filePaths == \"string\")\n"
		"		callback.addDependency(filePaths);\n"
		"	else foreach(path in filePaths)\n"
		"		callback.addDependency(path);\n"
		"	callback.onLoaded().setHandler(functor);\n"
		"	resourceManager.addCallback(callback);\n"
		"}\n"
	))
		return false;

	// Patch the original AnimationComponent constructor to pass our AnimationUpdaterComponent automatically
	if(!vm.runScript("\
		local backup = AnimationComponent.constructor;\n\
		AnimationComponent.constructor <- function() : (backup) {\n\
			backup.call(this, gLauncher.animationUpdater);	// Call the original constructor\n\
		}\n"
	))
		return false;

	// Patch the original RigidBodyComponent constructor to pass our dynamics world automatically
	if(!vm.runScript("\
		local backup = RigidBodyComponent.constructor;\n\
		RigidBodyComponent.constructor <- function(mass, collisionShape) : (backup) {\n\
			backup.call(this, gLauncher.dynamicsWorld, mass, collisionShape);	// Call the original constructor\n\
		}\n"
	))
		return false;

	// Patch the original AudioSourceComponent constructor to pass our resource manager automatically
	if(!vm.runScript("\
		local backup = AudioSourceComponent.constructor;\n\
		AudioSourceComponent.constructor <- function(fileId, args=\"\") : (backup) {\n\
			backup.call(this);	// Call the original constructor\n\
			this.load(resourceManager, fileId, args);\n\
			this.classString=format(\"AudioSourceComponent(\\\"%s\\\", \\\"%s\\\")\", fileId, args);\n\
		}\n"
	))
		return false;

	if(rootNode)
		setRootNode(rootNode);
	else
		setRootNode(new Entity());

	{	// Setup AnimationUpdaterComponent
		if(mAnimationUpdater && mAnimationUpdater->entity())
			delete mAnimationUpdater->entity();

		EntityPtr e = new Entity;
		e->name = "Animation updater";
		mAnimationUpdater = new AnimationUpdaterComponent(&taskPool);
		e->addComponent(mAnimationUpdater.get());
		e->asChildOf(mRootNode);
	}

	{	// Setup SkeletonAnimationUpdaterComponent
		if(mSkeletonAnimationUpdater && mSkeletonAnimationUpdater->entity())
			delete mSkeletonAnimationUpdater->entity();

		EntityPtr e = new Entity;
		e->name = "Skeleton animation updater";
		mSkeletonAnimationUpdater = new SkeletonAnimationUpdaterComponent(&taskPool);
		e->addComponent(mSkeletonAnimationUpdater.get());
		e->asChildOf(mRootNode);
	}

	setInputComponent(&inputComponent);

	// Add any other high level loaders.
	dynamic_cast<ResourceManager*>(mResourceManager)->addFactory(new PodLoaderFactory(*mResourceManager, mAnimationUpdater.get(), mSkeletonAnimationUpdater.get()));

	return true;
}

Entity* Launcher::loadEntity(const char* filePath, const char* args)
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
			if(addToHere)
				createStaticRigidBody(mDynamicsWorld, *addToHere);
		}

	protected:
		DynamicsWorld& mDynamicsWorld;
	};	// EntityLoadCreatePhysicsCallback

	bool createCollisionMesh = false;
	if(nullptr != args) {
		NvpParser parser(args);
		const char* name, *value;
		while(parser.next(name, value)) {
			if(strCaseCmp(name, "createCollisionMesh") == 0 && strCaseCmp(value, "true") == 0)
				createCollisionMesh = true;
		}
	}

	std::string argString = args;
	argString += ";loadAsEntity=true";

	std::auto_ptr<Entity> e(new Entity());
	e->name = Path(filePath).getLeaf();	// Only use the file name, the branch path is ignored.
	EntityPrototypeLoader::addEntityAfterLoad(e.get(), *mResourceManager, filePath,
		createCollisionMesh ? new EntityLoadCreatePhysicsCallback(mDynamicsWorld) : nullptr,
		0, argString.c_str()
	);

	return e.release();
}

Entity* Launcher::loadEntity2(const char* filePath, const char* args)
{
	bool createCollisionMesh = false;
	if(nullptr != args) {
		NvpParser parser(args);
		const char* name, *value;
		while(parser.next(name, value)) {
			if(strCaseCmp(name, "createCollisionMesh") == 0 && strCaseCmp(value, "true") == 0)
				createCollisionMesh = true;
		}
	}

	Entity* ret = PrefabLoaderComponent::loadEntity(
		*mResourceManager, filePath, args, createCollisionMesh ? &mDynamicsWorld : nullptr
	);
	return ret ? ret : new Entity();
}

void Launcher::update()
{
	MemoryProfiler::Scope memProfiler("Launcher::update");
	ThreadedCpuProfiler::Scope cpuProfiler("Launcher::update");

	HSQREMOTEDBG rdbg = reinterpret_cast<HSQREMOTEDBG>(mDbgContext);
	if(rdbg)
		sq_rdbg_update(rdbg);
	mResourceManager->update();

	scriptComponentManager.updateScriptComponents();

	BehaviourComponent::traverseEntities(mRootNode, mDeltaTimer.getDelta().asSecond());
	RenderableComponent::traverseEntities(mRootNode);
	AudioComponent::traverseEntities(mRootNode);
}

void Launcher::setRootNode(Entity* e)
{
	mRootNode = e;

	if(!e)
		return;

	mRootNode->name = "Launcher root node";
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
		e->name = "Input";
		e->asChildOf(mRootNode);
		e->addComponent(inputComponent);
		e.release();
	}

	MCD_VERIFY(vm.runScript("gInput <- gLauncher.inputComponent;\n"));
}

LauncherDefaultResourceManager::LauncherDefaultResourceManager(IFileSystem& fileSystem, bool takeFileSystemOwnership)
	: ResourceManager(fileSystem, takeFileSystemOwnership)
{
	addFactory(new BitmapLoaderFactory);
	addFactory(new DdsLoaderFactory);
	addFactory(new EffectLoaderFactory(*this));
	addFactory(new JpegLoaderFactory);
	addFactory(new Max3dsLoaderFactory(*this));
	addFactory(new OggLoaderFactory);
	addFactory(new PixelShaderLoaderFactory);
	addFactory(new PngLoaderFactory);
	addFactory(new PvrLoaderFactory);
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
		const IResourceLoader::LoadingState loadingState = e.loader->getLoadingState();
		const bool hasError = loadingState == IResourceLoader::Aborted;

		if(hasError)
			Log::format(Log::Warn, "Resource: %s %s", e.resource->fileId().getString().c_str(), "failed to load");
		else if(loadingState != IResourceLoader::Loading)
			e.loader->commit(*e.resource);	// Allow one resource to commit for each frame

		// Note that commit() is invoked before doCallbacks()
		doCallbacks(e);

		return hasError ? -1 : 1;
	}
	else {
		// Remember to call doCallbacks() even for empty Event such that
		// invoking addCallback() after a resource is loaded will also trigger the callback.
		doCallbacks(e);
	}

	return 0;
}