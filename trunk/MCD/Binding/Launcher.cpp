#include "Pch.h"
#include "Launcher.h"
#include "Entity.h"
#include "InputComponent.h"
#include "PhysicsComponent.h"
#include "System.h"
#include "../Component/Physics/RigidBodyComponent.h"
#include "../Component/Render/EntityPrototypeLoader.h"
#include "../Component/Render/RenderableComponent.h"
#include "../Core/System/FileSystemCollection.h"
#include "../Core/System/Log.h"
#include "../Core/System/RawFileSystem.h"
#include "../Core/System/Resource.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/ZipFileSystem.h"
#include "../Render/ResourceLoaderFactory.h"
#include "../../3Party/jkbind/Declarator.h"

using namespace MCD;

namespace {

IFileSystem* createDefaultFileSystem()
{
	std::auto_ptr<FileSystemCollection> fileSystem(new FileSystemCollection);

	Path actualRoot;

	try {
		std::auto_ptr<IFileSystem> rawFs(new RawFileSystem(L"Media"));
		actualRoot = rawFs->getRoot();
		fileSystem->addFileSystem(*rawFs.release());
	} catch(...) {}

	try {
		std::auto_ptr<IFileSystem> zipFs(new ZipFileSystem(actualRoot.getBranchPath() / L"Media.zip"));
		fileSystem->addFileSystem(*zipFs.release());
	} catch(...) {}

	return fileSystem.release();
}

class DefaultResourceManager : public MCD::ResourceManager
{
public:
	explicit DefaultResourceManager(MCD::IFileSystem& fileSystem)
		: ResourceManager(fileSystem)
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

	~DefaultResourceManager()
	{
	}

	/*!
		\return
			>0 - A resource is sucessfully loaded (partial or full).
			=0 - There are no more event to process at this moment.
			<0 - The resource failed to load.
	 */
	int processLoadingEvents()
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

protected:
	void setupFactories();

	class Impl;
	Impl* mImpl;
};	// DefaultResourceManager

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
	.declareClass<ResourceLoadCallback, IResourceManagerCallback>(L"ResourceLoadCallback")
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

Launcher::Launcher()
	:
	fileSystem(*createDefaultFileSystem()),
	mResourceManager(new DefaultResourceManager(fileSystem)),	// The ownership of fileSystem will pass to the resource manager
	mScriptComponentManager(fileSystem)
{
	FileSystemCollection& fs = dynamic_cast<FileSystemCollection&>(fileSystem);
	fs.addFileSystem(*(new RawFileSystem(L"")));

	// Start the physics thread
	mPhysicsThread.start(mDynamicsWorld, false);

	mSingleton = this;
}

Launcher::~Launcher()
{
//	mScriptComponentManager.shutdown();

	// Make sure the RigidBodyComponent is freed BEFORE the dynamics world...
	while(mRootNode->firstChild())
		delete mRootNode->firstChild();

	// Stop the physics thread
	mPhysicsThread.postQuit();
	mPhysicsThread.wait();

	// Give the script engine a chance to do cleanups
//	mScriptComponentManager.updateScriptComponents();

	// The Entity tree must be destroyed before the script VM.
	delete mRootNode;

	delete mResourceManager;
}

void Launcher::init(InputComponent& inputComponent, Entity* rootNode)
{
	if(rootNode)
		mRootNode = rootNode;
	else {
		mRootNode = new Entity();
		mRootNode->name = L"root";
	}

	mInputComponent = &inputComponent;

	mScriptComponentManager.registerRootEntity(*mRootNode);

	{	// Setup input component
		std::auto_ptr<Entity> e(new Entity);
		e->name = L"Input";
		e->asChildOf(mRootNode);
		e->addComponent(mInputComponent);
		e.release();
	}

	using namespace script;

	VMCore* v = (VMCore*)sq_getforeignptr(HSQUIRRELVM(mScriptComponentManager.vm.getImplementationHandle()));

	script::RootDeclarator root(v);
	root.declareFunction<objNoCare>(L"_getlauncher", &Launcher::sinleton);

	script::ClassTraits<FrameTimer>::bind(v);
	script::ClassTraits<ResourceLoadCallback>::bind(v);
	script::ClassTraits<Launcher>::bind(v);

	// Setup some global variable for easy access in script.
	mScriptComponentManager.vm.runScript(
		L"gLauncher <- _getlauncher();\n"

		L"function loadEntity(filePath, loadOptions={}) {\n"
		L"	if(\"createStaticRigidBody\" in loadOptions && loadOptions[\"createStaticRigidBody\"])\n"
		L"		return gLauncher.loadEntity(filePath, true);\n"
		L"	return gLauncher.loadEntity(filePath, false);\n"
		L"}\n"

		L"gInput <- gLauncher.inputComponent;\n"
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
	);

	// Patch the original RigidBodyComponent constructor to pass our dynamics world automatically
	mScriptComponentManager.vm.runScript(L"\
		local backup = RigidBodyComponent.constructor;\n\
		RigidBodyComponent.constructor <- function(mass, collisionShape) : (backup) {\n\
			backup.call(this, gLauncher.dynamicsWorld, mass, collisionShape);	// Call the original constructor\n\
		}\n"
	);

	// TODO: Let user supply a command line argument to choose the startup script
	mScriptComponentManager.doFile(L"init.nut", true);
}

Entity* Launcher::loadEntity(const wchar_t* filePath, bool createCollisionMesh)
{
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

	Entity* e = new Entity();
	e->name = filePath;
	EntityPrototypeLoader::addEntityAfterLoad(e, *mResourceManager, filePath,
		createCollisionMesh ? new EntityLoadCreatePhysicsCallback(mDynamicsWorld) : nullptr,
		0, L"loadAsEntity=true"
	);

	return e;
}

void Launcher::update(float deltaTime)
{
	static_cast<DefaultResourceManager*>(mResourceManager)->processLoadingEvents();

	mScriptComponentManager.updateScriptComponents();

	BehaviourComponent::traverseEntities(mRootNode);
	RenderableComponent::traverseEntities(mRootNode);

	mFrameTimer.nextFrame();
}
