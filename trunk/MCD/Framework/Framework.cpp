#include "Pch.h"
#include "Framework.h"
#include "FpsControllerComponent.h"
#include "ResizeFrustumComponent.h"

#include "../Core/Entity/Entity.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/System/FileSystemCollection.h"
#include "../Core/System/Log.h"
#include "../Core/System/PtrVector.h"
#include "../Core/System/RawFileSystem.h"
#include "../Core/System/RawFileSystemMonitor.h"
#include "../Core/System/Resource.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/TaskPool.h"
#include "../Core/System/Window.h"
#include "../Core/System/ZipFileSystem.h"

#ifdef MCD_IPHONE
#	include "../Core/Entity/iPhoneInputComponent.h"
#else
#	include "../Core/Entity/WinMessageInputComponent.h"
#endif

#include "../Render/Light.h"
#include "../Render/Renderer.h"
#include "../Render/RenderTargetComponent.h"
#include "../Render/RenderWindow.h"
#include "../Component/Render/AnimationComponent.h"
#include "../Component/Render/CameraComponent.h"
#include "../Component/Prefab.h"
#include "../Component/PrefabLoaderComponent.h"
#include "../Component/Render/SkeletonAnimationComponent.h"

namespace MCD {

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
	PrefabLoaderComponent* loadPrefabTo(const Path& resourcePath, Entity* location, IResourceManager::BlockingMode blockingMode, const char* args);
	void processLoadingEvents();
	bool update(Event& e);

	EntityPtr mRootEntity, mSystemEntity, mSceneLayer, mGuiLayer;
	std::auto_ptr<FileSystemCollection> mFileSystem;
	std::auto_ptr<ResourceManager> mResourceManager;

	ptr_vector<RawFileSystemMonitor> mFileMonitors;

	bool mTakeWindowOwership;

	RendererComponentPtr mRenderer;
	std::auto_ptr<RenderWindow> mWindow;
	std::auto_ptr<TaskPool> mTaskPool;
};	// Impl

Framework::Impl::Impl()
{
	mTakeWindowOwership = true;

	Log::start(&std::cout);
	Log::setLevel(Log::Level(Log::Error | Log::Warn | Log::Info));

	{	// Setup the default entity tree structure
		mRootEntity = new Entity("Root entity");

		mSystemEntity = new Entity("System entities");
		mSystemEntity->asChildOf(mRootEntity.getNotNull());

		mSceneLayer = new Entity("Scene layer");
		mSceneLayer->insertAfter(mSystemEntity.getNotNull());

		mGuiLayer = new Entity("Gui layer");
		mGuiLayer->insertAfter(mSceneLayer.getNotNull());	// Gui will draw after the scene layer
	}

	{	// Task pool
		mTaskPool.reset(new TaskPool);
#ifdef MCD_IPHONE
		mTaskPool->setThreadCount(0);
#else
		mTaskPool->setThreadCount(1);
#endif
	}

	{	// File system
		mFileSystem.reset(new FileSystemCollection);
		Entity* e = new Entity("File system");
		mSystemEntity->addChild(e);
		e->addComponent(new FileSystemComponent(*mFileSystem));
	}

	{	// Resource manager
		mResourceManager.reset(new ResourceManager(*mFileSystem, *mTaskPool, false));
		Entity* e = new Entity("Resource manager");
		mSystemEntity->addChild(e);
		e->addComponent(new ResourceManagerComponent(*mResourceManager));
	}

	{	// Default light
		Entity* e = new Entity("Default light");
		mSceneLayer->addChild(e);

		{	Entity* e1 = new Entity("Light 1");
			e->addChild(e1);
			e1->localTransform.setTranslation(Vec3f(10, 10, 0));

			LightComponent* light = new LightComponent;
			e1->addComponent(light);
			light->color = ColorRGBf(1, 0.8f, 0.8f);
		}

		{	Entity* e1 = new Entity("Light 2");
			e->addChild(e1);
			e1->localTransform.setTranslation(Vec3f(0, 10, 10));

			LightComponent* light = new LightComponent;
			e1->addComponent(light);
			light->color = ColorRGBf(0.8f, 1, 0.8f);
		}
	}

	{	// Animation updater
		Entity* e = new Entity("Animation updater");
		mSystemEntity->addChild(e);
#ifdef MCD_IPHONE
		AnimationUpdaterComponent* c = new AnimationUpdaterComponent(nullptr);
#else
		AnimationUpdaterComponent* c = new AnimationUpdaterComponent(mTaskPool.get());
#endif
		e->addComponent(c);
	}

	{	// Skeleton animation updater
		Entity* e = new Entity("Skeleton animation updater");
		mSystemEntity->addChild(e);
#ifdef MCD_IPHONE
		SkeletonAnimationUpdaterComponent* c = new SkeletonAnimationUpdaterComponent(nullptr);
#else
		SkeletonAnimationUpdaterComponent* c = new SkeletonAnimationUpdaterComponent(mTaskPool.get());
#endif
		e->addComponent(c);
	}
}

Framework::Impl::~Impl()
{
	mTaskPool->stop();

	mWindow->makeActive();
	delete mRootEntity.get();
	MCD_ASSERT(!mSystemEntity);
	MCD_ASSERT(!mSceneLayer);
	MCD_ASSERT(!mGuiLayer);
	MCD_ASSERT(!mRenderer);

	mResourceManager.reset();
	mFileSystem.reset();

	if(mTakeWindowOwership)
		mWindow.reset();
	else
		mWindow.release();

	mTaskPool.reset();

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
		Entity* e = new Entity("Renderer");
		mSystemEntity->addChild(e);
		e->addComponent(mRenderer.get());
	}

	{	// Default scene camera
		Entity* e = new Entity("Scene camera");
		mSceneLayer->addChild(e);
		CameraComponent2* c = new CameraComponent2(mRenderer);
		e->addComponent(c);
		c->frustum.projectionType = Frustum::Perspective;
		c->frustum.create(45.f, 4.0f / 3.0f, 1.0f, 500.0f);
		e->localTransform.setTranslation(Vec3f(0, 0, 10));
	}

	{	// Default Gui camera
		Entity* e = new Entity("Gui camera");
		mGuiLayer->addChild(e);
		CameraComponent2* c = new CameraComponent2(mRenderer);
		e->addComponent(c);
		c->frustum.projectionType = Frustum::Ortho;
	}

	{	// Setup scene render target
		Entity* e = new Entity("Scene layer render target");
		mSceneLayer->addChild(e);

		RenderTargetComponent* c = new RenderTargetComponent;
		c->window = dynamic_cast<RenderWindow*>(&existingWindow);
		c->entityToRender = mSceneLayer;
		c->cameraComponent = mSceneLayer->findComponentInChildrenExactType<CameraComponent2>();
		c->rendererComponent = mRenderer;
		e->addComponent(c);
	}

	{	// Setup Gui render target
		Entity* e = new Entity("Gui layer render target");
		mGuiLayer->addChild(e);

		RenderTargetComponent* c = new RenderTargetComponent;
		e->addComponent(c);
		c->shouldClearColor = false;
		c->clearColor = ColorRGBAf(0, 0);
		c->window = dynamic_cast<RenderWindow*>(&existingWindow);
		c->entityToRender = mGuiLayer;
		c->cameraComponent = mGuiLayer->findComponentInChildrenExactType<CameraComponent2>();
		c->rendererComponent = mRenderer;
	}

	// Setup resize frustum component for scene layer
	if(CameraComponent2* camera = mSceneLayer->findComponentInChildrenExactType<CameraComponent2>())
	{
		if(Entity* e = camera->entity()) {
			ResizeFrustumComponent* c = new ResizeFrustumComponent;
			e->addComponent(c);
			c->camera = mSceneLayer->findComponentInChildrenExactType<CameraComponent2>();
			c->renderTarget = mSceneLayer->findComponentInChildrenExactType<RenderTargetComponent>();
		}
	}

	// Setup resize frustum component for Gui layer
	if(CameraComponent2* camera = mGuiLayer->findComponentInChildrenExactType<CameraComponent2>())
	{
		if(Entity* e = camera->entity()) {
			ResizeFrustumComponent* c = new ResizeFrustumComponent;
			e->addComponent(c);
			c->camera = mGuiLayer->findComponentInChildrenExactType<CameraComponent2>();
			c->renderTarget = mGuiLayer->findComponentInChildrenExactType<RenderTargetComponent>();
		}
	}

	{	// Input component
		Entity* e = new Entity("Input");
		mSystemEntity->addChild(e);
#ifdef MCD_IPHONE
		iPhoneInputComponent* c = new iPhoneInputComponent;
#else
		WinMessageInputComponent* c = new WinMessageInputComponent;
#endif
		e->addComponent(c);
		c->attachTo(*mWindow);
	}

	{	// Default FPS controller
		Entity* e = new Entity("Fps controller");
		mSystemEntity->addChild(e);
		FpsControllerComponent* c = new FpsControllerComponent;
		e->addComponent(c);
		c->target = mSceneLayer->findEntityByPath("Scene camera");
		MCD_ASSERT(c->target);
		if(Entity* e1 = mSystemEntity->findEntityByPath("Input"))
			c->inputComponent = dynamic_cast<InputComponent*>(e1->findComponent<BehaviourComponent>());
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

PrefabLoaderComponent* Framework::Impl::loadPrefabTo(const Path& resourcePath, Entity* location, IResourceManager::BlockingMode blockingMode, const char* args)
{
	if(!location)
		return nullptr;

	std::auto_ptr<PrefabLoaderComponent> c(new PrefabLoaderComponent);
	c->prefab = dynamic_cast<Prefab*>(mResourceManager->load(resourcePath, blockingMode, 0, args).get());

	if(nullptr == c->prefab)
		return nullptr;

	location->addComponent(c.get());

	// Make sure the prefab is committed in blocking load
	if(blockingMode != IResourceManager::NonBlock) {
		while(c->prefab->commitCount == 0)
			processLoadingEvents();
		c->update(0);
	}

	return c.release();
}

void Framework::Impl::processLoadingEvents()
{
	ResourceManager::Event e = mResourceManager->popEvent();
	if(e.loader) {
		const IResourceLoader::LoadingState loadingState = e.loader->getLoadingState();
		const bool hasError = loadingState == IResourceLoader::Aborted;

		if(hasError)
			Log::format(Log::Warn, "Resource: %s %s", e.resource->fileId().getString().c_str(), "failed to load");
		else if(loadingState != IResourceLoader::Loading)
			e.loader->commit(*e.resource);	// Allow one resource to commit for each frame

		// Note that commit() is invoked before doCallbacks()
		mResourceManager->doCallbacks(e);
	}
}

bool Framework::Impl::update(Event& e)
{
	// Check for window events
	bool hasWindowEvent = false;
	if(mWindow.get())
		hasWindowEvent = mWindow->popEvent(e, false);

	{	// Reload any changed files in the RawFileSystem
		std::string path;
		MCD_FOREACH(const RawFileSystemMonitor& monitor, mFileMonitors) {
			while(!(path = monitor.getChangedFile()).empty())
				mResourceManager->reload(Path(path).normalize(), IResourceManager::NonBlock);
		}
	}

	processLoadingEvents();

	return hasWindowEvent;
}

Framework::Framework()
	: mImpl(*new Impl)
{}

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

PrefabLoaderComponent* Framework::loadPrefabTo(const Path& resourcePath, Entity* location, IResourceManager::BlockingMode blockingMode, const char* args) {
	return mImpl.loadPrefabTo(resourcePath, location, blockingMode, args);
}

bool Framework::update(Event& e) {
	return mImpl.update(e);
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

RenderWindow* Framework::window() {
	return mImpl.mWindow.get();
}

RendererComponentPtr Framework::rendererComponent() {
	return mImpl.mRenderer;
}

void Framework::processLoadingEvents() {
	mImpl.processLoadingEvents();
}

}	// namespace MCD
