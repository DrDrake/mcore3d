#include "stdafx.h"
#include "RenderPanelControl.h"
#include "CsInputComponent.h"
#include "FileSystemCollection.h"
#include "ResourceManager.h"
#include "Utility.h"
#include "../Common/Gizmo/Gizmo.h"
#include "../Common/GroundPlaneComponent.h"

#define _WINDOWS
#include "../../MCD/Audio/AudioDevice.h"
#include "../../MCD/Audio/AudioEffect.h"
#include "../../MCD/Binding/Launcher.h"
#include "../../MCD/Component/Render/AnimationComponent.h"
#include "../../MCD/Component/Render/CameraComponent.h"
#include "../../MCD/Component/Render/PickComponent.h"
#include "../../MCD/Component/Render/SkeletonAnimationComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/System/MemoryProfiler.h"
#include "../../MCD/Core/System/ResourceManager.h"
#include "../../MCD/Core/System/ThreadedCpuProfiler.h"
#include "../../MCD/Core/System/WindowEvent.h"
#include "../../MCD/Render/Camera.h"
#include "../../MCD/Render/RenderWindow.h"
#include "../../3Party/glew/glew.h"
#include "../../3Party/glew/wglew.h"
#include "../../3Party/squirrel/squirrel.h"
#undef nullptr
#include <gcroot.h>

#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32")
#pragma comment(lib, "glew")

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace Binding {

using namespace MCD;

class RenderPanelControlImpl : public RenderWindow
{
public:
	RenderPanelControlImpl(RenderPanelControl^ c, ResourceManager^ mgr)
		:
		mBackRef(c),
		mWidth(0), mHeight(0), mFieldOfView(60.0f),
		mGizmo(nullptr), mEntityPicker(nullptr),
		mPredefinedSubTree(nullptr), mUserSubTree(nullptr),
		mResourceManager(*mgr->getRawPtr()),
		mResourceManageRef(mgr),
		mPropertyGridNeedRefresh(false),
		mPlaying(false),
		mLauncher(*mgr->fileSystemCollection->getRawPtr(), *mgr->getRawPtr(), false)
	{
		mRootNode.name = "Ultimate root node";
		static_cast<MCD::ResourceManager&>(mResourceManager).taskPool().setThreadCount(1);
	}

	~RenderPanelControlImpl()
	{
		// Maually remove all entity, before the Launcher get destroyed.
		while(MCD::Entity* e = mRootNode.firstChild())
			e->destroyThis();

		// Remember to clear the old user sub-tree also
		if(mOldUserSubTree) {
			mOldUserSubTree->destroyThis();
			mOldUserSubTree = NULL;
		}

		mLauncher.setRootNode(nullptr);
	}

	void createScene()
	{
		{	// Setup user defined sub-tree
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = "Studio user defined sub-tree";
			e->asChildOf(&mRootNode);
			mUserSubTree = e.release();
		}

		{	// Setup pre-defined sub-tree
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = "Studio pre-defined sub-tree";
			e->asChildOf(&mRootNode);
			mPredefinedSubTree = e.release();
		}

		{	// Add the ground plane
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = "Ground plane";
			e->asChildOf(mPredefinedSubTree);

			// Add component
			GroundPlaneComponent* c = new GroundPlaneComponent;
			e->addComponent(c);

			e.release();
		}

		{	// Add picking detector
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = "Entity picker";
			e->asChildOf(mPredefinedSubTree);
			mEntityPicker = new PickComponent;
			mEntityPicker->entityToPick = mUserSubTree;
			e->addComponent(mEntityPicker);

			e.release();
		}

		{	// Add a C# contorl event input component
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = "Scene view input";
			e->asChildOf(mPredefinedSubTree);
			mCsInputComponent = new CsInputComponent();
			mCsInputComponent->attachTo(mBackRef);
			e->addComponent(mCsInputComponent.get());
			mLauncher.init(*mCsInputComponent, mUserSubTree.get());

			e.release();
		}

		{	// Add a default camera
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = "Default camera";
			e->asChildOf(mPredefinedSubTree);

			// Add component
			mCamera = static_cast<CameraComponent*>(mLauncher.scriptComponentManager.runScripAsComponent(
				"return loadComponent(\"StudioFpsCamera.nut\");"
			));
			mCamera->clearColor = ColorRGBf(0.5f);	// Set the background color as 50% gray
			e->addComponent(mCamera.get());

			e.release();
		}

		{	// Add a Gizmo
			// TODO: Move the Gizmo entity to mPredefinedSubTree, make sure all
			// related stuffs (eg axis picking) are working
			std::auto_ptr<Gizmo> e(new Gizmo(mResourceManager, mLauncher.inputComponent()));
			e->setCamrea(mCamera->entity());
			e->name = "Gizmo";
			e->enabled = false;	// The gizmo is initially disable, until an object is selected
			e->asChildOf(&mRootNode);
			e->setActiveGizmo(e->translationGizmo.get());

			mGizmo = e.release();
		}

		{	// Move launcher's animation updater into mPredefinedSubTree
			MCD::EntityPtr e = mLauncher.animationUpdater()->entity();
			e->asChildOf(mPredefinedSubTree);
		}

		{	// Move launcher's skeleton animation updater into mPredefinedSubTree
			MCD::EntityPtr e = mLauncher.skeletonAnimationUpdater()->entity();
			e->asChildOf(mPredefinedSubTree);
		}

		// Initialize the serialization system
		MCD_VERIFY(executeScriptFile("EntitySerialization.nut"));
	}

	void update()
	{
		MCD::ThreadedCpuProfiler::Scope scope("RenderPanelControl::update");

		const float dt = float(mDeltaTimer.getDelta().asSecond());
		makeActive();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_LIGHTING);
		preUpdate();

		mLauncher.update();
		mResourceManageRef->pollForUpdatedFiles();

		// Update pre-defined tree
		if(!mPlaying) {
			BehaviourComponent::traverseEntities(mPredefinedSubTree, dt);
			RenderableComponent::traverseEntities(mPredefinedSubTree);

			// Draw the Gizmo
			if(mGizmo && mGizmo->enabled) {
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_LIGHTING);
				glClear(GL_DEPTH_BUFFER_BIT);
				BehaviourComponent::traverseEntities(mGizmo, dt);
				RenderableComponent::traverseEntities(mGizmo);
			}

			// Handle picking result
			for(size_t i=0; i<mEntityPicker->hitCount(); ++i) {
				EntityPtr e = mEntityPicker->hitAtIndex(i);
				if(!e)
					continue;
				mBackRef->entitySelectionChanged(mBackRef, Entity::getEntityFromRawPtr(e.get()));
				break;	// We only pick the Entity that nearest to the camera
			}
			mEntityPicker->clearResult();
			mEntityPicker->entity()->enabled = false;
		}
		else {
			mLauncher.animationUpdater()->update(dt);
			mLauncher.skeletonAnimationUpdater()->update(dt);
			// Update the C# input component the last, such that the getButtonDown() run correctly.
			mCsInputComponent->update(dt);
		}

		postUpdate();
	}

	void resize(size_t width, size_t height)
	{
		// Prevents problem when the application is minmized
		if(width == 0 || height == 0)
			return;

		// Make this resize function vert lazy
		if(width == mWidth && height == mHeight)
			return;

		mWidth = float(width);
		mHeight = float(height);

		makeActive();

		// A lot of opengl options to be enabled by default
		glShadeModel(GL_SMOOTH);
		glFrontFace(GL_CCW);			// OpenGl use counterclockwise as the default winding
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		// Automatic normalization (useful when we have uniform scaled the model)
		// Reference: http://www.opengl.org/resources/features/KilgardTechniques/oglpitfall/
		glEnable(GL_RESCALE_NORMAL);
//		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		// Set up and enable light 0
		glDisable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		GLfloat ambientLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
		GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

		GLfloat lightPos[] = { 200, 200, 200, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		// Prevents division by zero
		height = (height == 0) ? 1 : height;
		glViewport(0, 0, width, height);
		if(mCamera)
			mCamera->camera.frustum.setAcpectRatio(float(width)/height);

		update();
	}

	bool executeScriptFile(const char* scriptFilePath)
	{
		return mLauncher.scriptComponentManager.doFile(scriptFilePath, true);
	}

	void play(const char* scriptFilePath)
	{
		// Backup the old sub-tree and input component
		mOldUserSubTree = mUserSubTree->clone();
		std::swap(mUserSubTree, mOldUserSubTree);

		// Use the new sub-tree
		mOldUserSubTree->unlink();
		mLauncher.setRootNode(mUserSubTree.get());
		mUserSubTree->asChildOf(&mRootNode);

		mLauncher.scriptComponentManager.doFile(scriptFilePath, true);
		mCamera->entity()->enabled = false;
		mPlaying = true;
	}

	void stop()
	{
		if(!mOldUserSubTree.get())	// Not playing
			return;

		// Restore the old backup sub-tree and input component
		std::swap(mUserSubTree, mOldUserSubTree);
		mUserSubTree->asChildOf(&mRootNode);
		mLauncher.setRootNode(mUserSubTree.get());

		if(mOldUserSubTree) {
			mOldUserSubTree->destroyThis();
			mOldUserSubTree = NULL;
		}

		// Clear the class cache so that all component scripts will be reloaded from the file system on next play()
		(void)mLauncher.vm.runScript("clearClassCache();");

		mCamera->entity()->enabled = true;
		mPlaying = false;
	}

	void onKeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
	{
	}

	void onKeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
	{
	}

	void onMouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		if(e->Button == MouseButtons::Left)
		{
			mGizmo->mouseDown(e->X, e->Y);

			// Gizmo has a higher priority to do picking
			if(!mGizmo->isDragging())
			{
				mEntityPicker->entity()->enabled = true;
				mEntityPicker->setPickRegion(e->X, e->Y);
			}
		}

		mLastMousePos = Point(e->X, e->Y);
	}

	void onMouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		mGizmo->mouseUp(e->X, e->Y);
	}

	void onMouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		mGizmo->mouseMove(e->X, e->Y);

		if(mGizmo->isDragging() && mLastMousePos != Point(e->X, e->Y))
			mPropertyGridNeedRefresh = true;

		mLastMousePos = Point(e->X, e->Y);
	}

	//! A helper class object to ensure proper init/close invocation order.
	struct AudioInitiator {
		AudioInitiator() { initAudioDevice(); initAudioEffect(); }
		~AudioInitiator() { closeAudioDevice(); }
	};	// AudioInitiator

	AudioInitiator mAudioInitiator;
	gcroot<RenderPanelControl^> mBackRef;
	float mWidth, mHeight;
	float mFieldOfView;
	MCD::Entity mRootNode, *mPredefinedSubTree;
	MCD::EntityPtr mUserSubTree;
	Gizmo* mGizmo;
	MCD::PickComponent* mEntityPicker;
	MCD::IntrusiveWeakPtr<CameraComponent> mCamera;
	IResourceManager& mResourceManager;
	gcroot<ResourceManager^> mResourceManageRef;
	bool mPropertyGridNeedRefresh;
	Point mLastMousePos;
	bool mPlaying;
	DeltaTimer mDeltaTimer;

	Launcher mLauncher;
	MCD::EntityPtr mOldUserSubTree;
	MCD::IntrusiveWeakPtr<CsInputComponent> mCsInputComponent;
};	// RenderPanelControlImpl

RenderPanelControl::RenderPanelControl(ResourceManager^ resourceManager, IntPtr sharedGlContext)
{
	InitializeComponent();
	mSharedGlContext = sharedGlContext;
	mImpl = nullptr;
	mResourceManager = resourceManager;
}

RenderPanelControl::~RenderPanelControl()
{
	if(components)
		delete components;

	this->!RenderPanelControl();
}

RenderPanelControl::!RenderPanelControl()
{
	destroy();
}

IntPtr RenderPanelControl::renderContext::get()
{
	if(!mImpl)
		return IntPtr(nullptr);
	return IntPtr(mImpl->renderContext());
}

void RenderPanelControl::destroy()
{
	enableAutoUpdate(false);
	delete mImpl;
	mImpl = nullptr;
}

void RenderPanelControl::update()
{
	if(!mImpl)
		return;
	mImpl->resize(this->Width, this->Height);
	mImpl->update();

	if(mImpl->mPropertyGridNeedRefresh) {
		propertyGrid->Refresh();
		mImpl->mPropertyGridNeedRefresh = false;
	}
}

void RenderPanelControl::enableAutoUpdate(bool flag)
{
	if(flag)
		timer->Start();
	else
		timer->Stop();
}

::Binding::Entity^ RenderPanelControl::rootEntity::get()
{
	if(mRootEntity == nullptr)
		mRootEntity = gcnew ::Binding::Entity(IntPtr(&(mImpl->mRootNode)));
	return mRootEntity;
}

::Binding::Entity^ RenderPanelControl::userRootEntity::get()
{
	if(mUserRootEntity == nullptr || mUserRootEntity->getRawEntityPtr() != mImpl->mLauncher.rootNode())
		mUserRootEntity = gcnew ::Binding::Entity(IntPtr(mImpl->mLauncher.rootNode()));
	return mUserRootEntity;
}

::Binding::Entity^ RenderPanelControl::selectedEntity::get()
{
	MCD::Entity* e = mImpl->mGizmo->selectedEntity();
	return Entity::getEntityFromRawPtr(e);
}

void RenderPanelControl::selectedEntity::set(::Binding::Entity^ entity)
{
	if(selectedEntity == entity)
		return;

	MCD::Entity* e = nullptr;
	if(entity != nullptr)
		e = entity->getRawEntityPtr();

	mImpl->mGizmo->setSelectedEntity(e);
	entitySelectionChanged(this, entity);
}

RenderPanelControl::GizmoMode RenderPanelControl::gizmoMode::get()
{
	Gizmo* g = mImpl->mGizmo;
	if(g) {
		if(g->activeGizmo() == g->translationGizmo)
			return GizmoMode::Translate;
		if(g->activeGizmo() == g->rotationGizmo)
			return GizmoMode::Rotate;
		if(g->activeGizmo() == g->scaleGizmo)
			return GizmoMode::Scale;
	}

	return GizmoMode::None;
}

void RenderPanelControl::gizmoMode::set(GizmoMode mode)
{
	Gizmo* g = mImpl->mGizmo;
	if(!g)
		return;

	if(mode == GizmoMode::Translate)
		g->setActiveGizmo(g->translationGizmo);
	else if(mode == GizmoMode::Rotate)
		g->setActiveGizmo(g->rotationGizmo);
	else if(mode == GizmoMode::Scale)
		g->setActiveGizmo(g->scaleGizmo);
	else
		g->setActiveGizmo(NULL);
}

bool RenderPanelControl::executeScriptFile(System::String^ scriptFilePath)
{
	return mImpl->executeScriptFile(Utility::toUtf8String(scriptFilePath).c_str());
}

void RenderPanelControl::play(String^ scriptFilePath)
{
	MemoryProfiler::Scope profiler("RenderPanelControl::play");

	mImpl->play(Utility::toUtf8String(scriptFilePath).c_str());

	// The entity tree is altered, reset it.
	selectedEntity = nullptr;
	mRootEntity = nullptr;
	mRootEntity = rootEntity;
}

void RenderPanelControl::stop()
{
	MemoryProfiler::Scope profiler("RenderPanelControl::stop");

	mImpl->stop();

	// The entity tree is altered, reset it.
	selectedEntity = nullptr;
	mRootEntity = nullptr;
	mRootEntity = rootEntity;
}

bool RenderPanelControl::playing::get()
{
	return mImpl->mPlaying;
}

System::String^ RenderPanelControl::serailizeScene()
{
	std::string str = mImpl->mLauncher.vm.runScriptAsString(
		"gSerializationState.output=\"\";gSerializationState.patch.clear();"
		"rootEntity.serialize(gSerializationState);"
		"local ret = gSerializationState.output + gSerializationState.getPatchString();"
		"gSerializationState.reset();"
		"return ret;"
	);
	return gcnew System::String(str.c_str());
}

System::Void RenderPanelControl::timer_Tick(System::Object^ sender, System::EventArgs^ e)
{
	this->update();

	MCD::MemoryProfiler::singleton().nextFrame();
	MCD::ThreadedCpuProfiler::singleton().nextFrame();
}

System::Void RenderPanelControl::RenderPanelControl_Load(System::Object^ sender, System::EventArgs^ e)
{
	MCD_ASSERT(mImpl == nullptr);
	mImpl = new RenderPanelControlImpl(this, mResourceManager);
	mImpl->create(Handle.ToPointer(), nullptr);

	void* sharedGlContext = mSharedGlContext.ToPointer();
	if(sharedGlContext) {
		void* thisGlContext = mImpl->renderContext();
		::wglShareLists((HGLRC)sharedGlContext, (HGLRC)thisGlContext);
	}

	mImpl->makeActive();
	mImpl->createScene();
}

System::Void RenderPanelControl::RenderPanelControl_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e)
{
	this->update();
}

System::Void RenderPanelControl::RenderPanelControl_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
{
	mImpl->onKeyDown(sender, e);
}

System::Void RenderPanelControl::RenderPanelControl_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
{
	mImpl->onKeyUp(sender, e);
}

System::Void RenderPanelControl::RenderPanelControl_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
{
	mImpl->onMouseDown(sender, e);
}

System::Void RenderPanelControl::RenderPanelControl_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
{
	mImpl->onMouseUp(sender, e);
}

System::Void RenderPanelControl::RenderPanelControl_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
{
	mImpl->onMouseMove(sender, e);
}

}
