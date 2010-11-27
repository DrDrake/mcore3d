#include "Pch.h"
#include "Framework.h"
#include "FpsControllerComponent.h"
#include "ResizeFrustumComponent.h"
#include "BuildinData/BuildinData.h"

#include "../Core/Binding/CoreBindings.h"
#include "../Core/Binding/ScriptComponent.h"
#include "../Core/Binding/VMCore.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Entity/PrefabLoaderComponent.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/System/FileSystemCollection.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryFileSystem.h"
#include "../Core/System/PtrVector.h"
#include "../Core/System/RawFileSystem.h"
#include "../Core/System/RawFileSystemMonitor.h"
#include "../Core/System/Resource.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/TaskPool.h"
#include "../Core/System/Timer.h"
#include "../Core/System/Window.h"
#include "../Core/System/ZipFileSystem.h"

#ifdef MCD_IPHONE
#	include "../Core/Entity/iPhoneInputComponent.h"
#else
#	include "../Core/Entity/WinMessageInputComponent.h"
#endif

#include "../Render/Animation.h"
#include "../Render/Camera.h"
#include "../Render/Font.h"
#include "../Render/Light.h"
#include "../Render/Material.h"
#include "../Render/RenderBindings.h"
#include "../Render/Renderer.h"
#include "../Render/RenderTarget.h"
#include "../Render/RenderWindow.h"
#include "../Render/Skeleton.h"

#include "../Loader/BitmapLoader.h"
#include "../Loader/FntLoader.h"
#include "../Loader/JpegLoader.h"
#include "../Loader/PngLoader.h"
#include "../Loader/PvrLoader.h"
#include "../Loader/TgaLoader.h"

#include "../Audio/AudioDevice.h"
#include "../Audio/AudioBindings.h"
#include "../Audio/AudioSourceComponent.h"

namespace MCD {

namespace Binding {

extern void registerFrameworkBinding(VMCore& vm, Framework& framework);

}	// namespace Binding

class Framework::Impl
{
public:
	Impl();
	~Impl();

	bool initSystem();
	bool initWindow(const char* args);
	bool initWindow(RenderWindow& existingWindow, bool takeOwnership);
	bool addFileSystem(const char* path);
	bool removeFileSystem(const char* path);
	void enableDebuggerOnPort(uint16_t tcpPort);
	PrefabLoaderComponent* loadPrefabTo(const char* resourcePath, Entity& location, bool blockingLoad);
	void registerResourceCallback(const char* path, BehaviourComponent& behaviour, bool isRecursive, int minLoadIteration);
	bool update(Event& e);

	EntityPtr mRootEntity, mSystemEntity, mSceneLayer, mGuiLayer;
	std::auto_ptr<FileSystemCollection> mFileSystem;
	std::auto_ptr<ResourceManager> mResourceManager;

	ptr_vector<RawFileSystemMonitor> mFileMonitors;

	bool mTakeWindowOwership;

	Timer mTimer;
	float mDeltaTime, mCurrentTime;
	size_t mFrameCounter;	//! For calculating fps, reset every one second.
	float mOneSecondCountDown;
	float mFramePerSecond;

	RendererComponentPtr mRenderer;
	InputComponentPtr mInput;
	ResourceManagerComponentPtr mResourceManagerComponent;
	std::auto_ptr<RenderWindow> mWindow;
	std::auto_ptr<TaskPool> mTaskPool;
	TextLabelComponentPtr mFpsLabel;

	Binding::VMCore vm;
};	// Impl

Framework::Impl::Impl()
	: mFrameCounter(0), mOneSecondCountDown(0), mFramePerSecond(0)
{
	mDeltaTime = mCurrentTime = 0;
	mTakeWindowOwership = true;

	Log::start(&std::cout);
	Log::setLevel(Log::Level(Log::Error | Log::Warn | Log::Info));

	{	// Setup the default entity tree structure
		mRootEntity = new Entity("Root entity");
		Entity::setCurrentRoot(mRootEntity.getNotNull());

		mSystemEntity = mRootEntity->addLastChild("System entities");
		mSceneLayer = mRootEntity->addLastChild("Scene layer");
		mGuiLayer = mRootEntity->addLastChild("2D Gui layer");
	}

	{	// Task pool
		mTaskPool.reset(new TaskPool);
#ifdef MCD_IPHONE
		mTaskPool->setThreadCount(0);
#else
		mTaskPool->setThreadCount(3);
#endif
		Entity* e = mSystemEntity->addFirstChild("Task pool");
		e->addComponent(new TaskPoolComponent(*mTaskPool));
	}

	{	// File system
		mFileSystem.reset(new FileSystemCollection);
		Entity* e = mSystemEntity->addFirstChild("File system");
		e->addComponent(new FileSystemComponent(*mFileSystem));
	}

	{	// Setup the memory file system
		MemoryFileSystem* fs = new MemoryFileSystem("");
		mFileSystem->addFileSystem(*fs);
		addDataToMemoryFileSystem(*fs);
	}

	{	// Resource manager
		mResourceManager.reset(new ResourceManager(*mFileSystem, *mTaskPool, false));
		mResourceManagerComponent = new ResourceManagerComponent(*mResourceManager);
		Entity* e = mSystemEntity->addFirstChild("Resource manager");
		e->addComponent(mResourceManagerComponent.get());
	}

	{	// Register default resource loaders
		mResourceManager->addFactory(new BitmapLoaderFactory);
		mResourceManager->addFactory(new FntLoaderFactory);
		mResourceManager->addFactory(new JpegLoaderFactory);
		mResourceManager->addFactory(new PngLoaderFactory);
		mResourceManager->addFactory(new PvrLoaderFactory);
		mResourceManager->addFactory(new TgaLoaderFactory);
	}

	{	// Default light
		Entity* e = mSceneLayer->addFirstChild("Default light");

		{	Entity* e1 = e->addFirstChild("Light 1");
			e1->localTransform.setTranslation(Vec3f(10, 10, 0));

			LightComponent* light = e1->addComponent(new LightComponent);
			light->color = ColorRGBf(1, 0.8f, 0.7f);
		}

		{	Entity* e1 = e->addFirstChild("Light 2");
			e1->localTransform.setTranslation(Vec3f(0, 10, 10));

			LightComponent* light = e1->addComponent(new LightComponent);
			light->color = ColorRGBf(0.7f, 0.8f, 1.0f);
		}
	}

	{	// Fps label
		Entity* e = mGuiLayer->addFirstChild("Fps label");
		mFpsLabel = e->addComponent(new TextLabelComponent);
		mFpsLabel->font = "buildin/Arial-20.fnt";
		mFpsLabel->anchor = Vec2f(0, 0);
	}

	{	// Behaviour updater
		Entity* e = mSystemEntity->addFirstChild("Behaviour updater");
		BehaviourUpdaterComponent* c = new BehaviourUpdaterComponent;
		e->addComponent(c);
	}

	{	// Script manager
		using namespace Binding;
		Entity* e = mSystemEntity->addFirstChild("Script manager");
		ScriptManagerComponent* c = new ScriptManagerComponent(&vm);
		e->addComponent(c);
	}

	{	// Animation updater
		Entity* e = mSystemEntity->addFirstChild("Animation updater");
		AnimationUpdaterComponent* c = new AnimationUpdaterComponent;
		e->addComponent(c);
	}

	// Audio
	MCD_VERIFY(initAudioDevice());

	{	// Audio manager
		Entity* e = mSystemEntity->addFirstChild("Audio manager");
		AudioManagerComponent* c = new AudioManagerComponent;
		e->addComponent(c);
	}
}

Framework::Impl::~Impl()
{
	mTaskPool->stop();

	if(mWindow.get())
		mWindow->makeActive();

	delete mRootEntity.get();
	MCD_ASSERT(!mSystemEntity);
	MCD_ASSERT(!mSceneLayer);
	MCD_ASSERT(!mGuiLayer);
	MCD_ASSERT(!mRenderer);
	MCD_ASSERT(!mInput);

	mResourceManager.reset();
	mFileSystem.reset();

	if(mTakeWindowOwership)
		mWindow.reset();
	else
		mWindow.release();

	mTaskPool.reset();

	closeAudioDevice();

	Log::stop(false);
}

bool Framework::Impl::initWindow(RenderWindow& existingWindow, bool takeOwnership)
{
	mTakeWindowOwership = takeOwnership;
	mWindow.reset(&existingWindow);
	existingWindow.makeActive();

	// NOTE: There are components depends on the Window's handle need to be
	// initialized here

	{	// Renderer
		mRenderer = new RendererComponent;
		Entity* e = mSystemEntity->addFirstChild("Renderer");
		e->addComponent(mRenderer);
	}

	{	// Default scene camera
		Entity* e = mSceneLayer->addFirstChild("Scene camera");
		CameraComponent* c = e->addComponent(new CameraComponent(mRenderer));
		c->frustum.projectionType = Frustum::Perspective;
		c->frustum.create(45.f, 4.0f / 3.0f, 1.0f, 500.0f);
		e->localTransform.setTranslation(Vec3f(0, 0, 10));
	}

	{	// Default scene material
		mSceneLayer->addComponent(new MaterialComponent);
	}

	{	// Default Gui camera
		Entity* e = mGuiLayer->addFirstChild("Gui camera");
		CameraComponent* c = e->addComponent(new CameraComponent(mRenderer));
		c->frustum.projectionType = Frustum::YDown2D;
	}

	{	// Setup scene render target
		Entity* e = mSceneLayer->addFirstChild("Scene layer render target");
		RenderTargetComponent* c = e->addComponent(new RenderTargetComponent);
		c->window = dynamic_cast<RenderWindow*>(&existingWindow);
		c->entityToRender = mSceneLayer;
		c->cameraComponent = mSceneLayer->findComponentInChildrenExactType<CameraComponent>();
	}

	{	// Setup Gui render target
		Entity* e = mGuiLayer->addFirstChild("Gui layer render target");
		RenderTargetComponent* c = e->addComponent(new RenderTargetComponent);
		c->shouldClearColor = false;
		c->clearColor = ColorRGBAf(0, 0);
		c->window = dynamic_cast<RenderWindow*>(&existingWindow);
		c->entityToRender = mGuiLayer;
		c->cameraComponent = mGuiLayer->findComponentInChildrenExactType<CameraComponent>();
	}

	// Setup resize frustum component for scene layer
	if(CameraComponent* camera = mSceneLayer->findComponentInChildrenExactType<CameraComponent>())
	{
		if(Entity* e = camera->entity()) {
			ResizeFrustumComponent* c = e->addComponent(new ResizeFrustumComponent);
			c->camera = mSceneLayer->findComponentInChildrenExactType<CameraComponent>();
			c->renderTarget = mSceneLayer->findComponentInChildrenExactType<RenderTargetComponent>();
		}
	}

	// Setup resize frustum component for Gui layer
	if(CameraComponent* camera = mGuiLayer->findComponentInChildrenExactType<CameraComponent>())
	{
		if(Entity* e = camera->entity()) {
			ResizeFrustumComponent* c = e->addComponent(new ResizeFrustumComponent);
			c->camera = mGuiLayer->findComponentInChildrenExactType<CameraComponent>();
			c->renderTarget = mGuiLayer->findComponentInChildrenExactType<RenderTargetComponent>();
		}
	}

	{	// Input component
		Entity* e = new Entity("Input");
		e->insertAfter(mGuiLayer);
#ifdef MCD_IPHONE
		mInput = new iPhoneInputComponent;
#else
		WinMessageInputComponent* c = new WinMessageInputComponent;
		mInput = c;
		c->attachTo(*mWindow);
#endif
		e->addComponent(mInput);
	}

	{	// Default FPS controller
		Entity* e = mSystemEntity->addFirstChild("Fps controller");
		FpsControllerComponent* c = e->addComponent(new FpsControllerComponent);
		c->target = mSceneLayer->findEntityByPath("Scene camera");
		MCD_ASSERT(c->target);
		if(Entity* e1 = mRootEntity->findEntityByPath("Input"))
			c->inputComponent = dynamic_cast<InputComponent*>(e1->findComponent<BehaviourComponent>());
	}

	{	// Skeleton visualizer
		Entity* e = mSceneLayer->addFirstChild("Skeleton visualizer");
		MaterialComponent* m = e->addComponent(new MaterialComponent);
		m->lighting = false;
		m->cullFace = false;
		m->useVertexColor = true;

		SkeletonPoseVisualizer* c = e->addFirstChild("")->addComponent(new SkeletonPoseVisualizer);
		c->jointSize = 1;
		c->entityTree = mSceneLayer;
	}

	return true;
}

bool Framework::Impl::initWindow(const char* args)
{
	std::auto_ptr<RenderWindow> w(new RenderWindow);
	w->create(args);
	return initWindow(*w.release(), true);
}

bool Framework::Impl::addFileSystem(const char* path)
{
	if(strCaseCmp(Path(path).getExtension().c_str(), "zip") == 0) {
		RawFileSystem rawFs("");
		std::auto_ptr<IFileSystem> zipFs(new ZipFileSystem(rawFs.toAbsolutePath(path)));
		mFileSystem->addFileSystem(*zipFs.release());
		// NOTE: For simplicity we won't monitor changes of a zip file system
	}
	else {
		try {
			std::auto_ptr<IFileSystem> rawFs(new RawFileSystem(path));
			std::auto_ptr<RawFileSystemMonitor> monitor(new RawFileSystemMonitor(rawFs->getRoot().getString().c_str(), true));
			mFileSystem->addFileSystem(*rawFs.release());
			mFileMonitors.push_back(monitor.release());
		}
		catch(std::runtime_error& err) {
			Log::write(Log::Error, err.what());
			return false;
		}
	}

	return true;
}

bool Framework::Impl::removeFileSystem(const char* path)
{
	Path absolutePath(path);
	if(!absolutePath.hasRootDirectory())
		absolutePath = Path::getCurrentPath() / absolutePath;

	if(strCaseCmp(absolutePath.getString().c_str(), "zip") == 0) {
		return mFileSystem->removeFileSystem(absolutePath);
	}
	else {
		if(!mFileSystem->removeFileSystem(absolutePath))
			return false;

		for(size_t i=0; i<mFileMonitors.size(); ++i) {
			if(strCaseCmp(mFileMonitors[i].monitringPath().c_str(), absolutePath.getString().c_str()) == 0) {
				mFileMonitors.erase(mFileMonitors.begin()+i);
				return true;
			}
			//std::auto_ptr<RawFileSystemMonitor> monitor(new RawFileSystemMonitor(path, true));
			//mFileMonitors.push_back(monitor.release());
		}
	}

	return false;
}

PrefabLoaderComponent* Framework::Impl::loadPrefabTo(const char* resourcePath, Entity& location, bool blockingLoad)
{
	PrefabLoaderComponent* c(new PrefabLoaderComponent);
	const int blockingIteration = blockingLoad ? 1 : 0;
	c->prefab = dynamic_cast<Prefab*>(mResourceManager->load(resourcePath, blockingIteration, 0).get());

	if(!c->prefab) {
		c->destroyThis();
		return nullptr;
	}

	location.addComponent(c);

	// Make sure the prefab is committed in blocking load
	if(blockingIteration > 0)
		c->update(0);

	return c;
}

void Framework::Impl::registerResourceCallback(const char* path, BehaviourComponent& callback, bool isRecursive, int minLoadIteration)
{
	mResourceManagerComponent->registerCallback(path, callback, isRecursive, minLoadIteration);
}

bool Framework::Impl::update(Event& e)
{
	Entity::setCurrentRoot(mRootEntity.getNotNull());

	const float frameTime = 1.0f / 30;
	float newTime = float(mTimer.get().asSecond());
	mDeltaTime = newTime - mCurrentTime;

	const float overBuget = newTime - mCurrentTime - frameTime;
	float buget = frameTime - overBuget;
	buget = buget < frameTime/3 ? frameTime/3 : buget;
	const float timeOut = newTime + buget;
	mCurrentTime = newTime;

	// Check for window events
	bool hasWindowEvent = false;
	if(mWindow.get())
		hasWindowEvent = mWindow->popEvent(e, false);

	{	// Reload any changed files in the RawFileSystem
		std::string path;
		MCD_FOREACH(const RawFileSystemMonitor& monitor, mFileMonitors) {
			while(!(path = monitor.getChangedFile()).empty())
				mResourceManager->reload(Path(path).normalize());
		}
	}

	if(mResourceManagerComponent)
		mResourceManagerComponent->update(&mTimer, timeOut);

	{	// Frame rate calculation
		mOneSecondCountDown -= mDeltaTime;
		++mFrameCounter;

		if(mOneSecondCountDown < 0) {
			mFramePerSecond = float(mFrameCounter) / (1-mOneSecondCountDown);
			mOneSecondCountDown = 1 + mOneSecondCountDown;
			mFrameCounter = 0;
		}

		mFpsLabel->text = FixString(float2Str(mFramePerSecond).c_str());
	}

	{	// Entity and Component traversal
		ComponentUpdater::traverseBegin(*mSystemEntity);

		// Gather components into the corresponding updater
		for(EntityPreorderIterator i(mRootEntity.get()); !i.ended(); )
		{
			if(!i->enabled) {
				i.skipChildren();
				continue;
			}

			for(Component* c = i->components.begin(); c != i->components.end(); c = c->next())
				c->gather();

			i.next();
		}

		// Preform the updater's update(dt) function
		ComponentUpdater::traverseEnd(*mSystemEntity, mDeltaTime);
	}

	return hasWindowEvent;
}

static Framework* gCurrentUpdatingFramework = nullptr;

Framework::Framework()
	: mImpl(*new Impl)
{
	{	// Initialize script vm
		Binding::registerCoreBinding(mImpl.vm);
		Binding::registerAudioBinding(mImpl.vm);
		Binding::registerRenderBinding(mImpl.vm);
		Binding::registerFrameworkBinding(mImpl.vm, *this);
	}
}

Framework::~Framework()
{
	delete &mImpl;
}

bool Framework::initWindow(RenderWindow& existingWindow, bool takeOwership) {
	return mImpl.initWindow(existingWindow, takeOwership);
}

bool Framework::initWindow(const char* args) {
	return mImpl.initWindow(args);
}

bool Framework::addFileSystem(const char* path) {
	return mImpl.addFileSystem(path);
}

bool Framework::removeFileSystem(sal_in_z const char* path) {
	return mImpl.removeFileSystem(path);
}

void Framework::addLoaderFactory(ResourceManager::IFactory* factory) {
	resourceManager().addFactory(factory);
}

PrefabLoaderComponent* Framework::loadPrefabTo(const char* resourcePath, Entity& location, bool blockingLoad) {
	return mImpl.loadPrefabTo(resourcePath, location, blockingLoad);
}

void Framework::mainLoop()
{
	Event e;
	do {
		(void)update(e);
		rendererComponent()->render(rootEntity());
	} while(e.Type != Event::Closed);
}

bool Framework::update(Event& e)
{
	gCurrentUpdatingFramework = this;
	bool ret = mImpl.update(e);
	gCurrentUpdatingFramework = nullptr;
	return ret;
}

FileSystemCollection& Framework::fileSystemCollection() {
	return *mImpl.mFileSystem;
}

ResourceManager& Framework::resourceManager() {
	return *mImpl.mResourceManager;
}

TaskPool& Framework::taskPool() {
	return *mImpl.mTaskPool;
}

Entity& Framework::rootEntity() {
	return *mImpl.mRootEntity;
}

Entity& Framework::systemEntity() {
	return *mImpl.mSystemEntity;
}

Entity& Framework::sceneLayer() {
	return *mImpl.mSceneLayer;
}

Entity& Framework::guiLayer() {
	return *mImpl.mGuiLayer;
}

Binding::VMCore& Framework::vm() {
	return mImpl.vm;
}

RenderWindow* Framework::window() {
	return mImpl.mWindow.get();
}

RendererComponentPtr Framework::rendererComponent() {
	return mImpl.mRenderer;
}

InputComponentPtr Framework::inputComponent() {
	return mImpl.mInput;
}

float Framework::dt() const {
	return mImpl.mDeltaTime;
}

float Framework::fps() const {
	return mImpl.mFramePerSecond;
}

Framework* currentUpdatingFramework() {
	return gCurrentUpdatingFramework;
}

}	// namespace MCD
