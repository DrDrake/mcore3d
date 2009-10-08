#include "stdafx.h"
#include "RenderPanelControl.h"
#include "CsInputComponent.h"
#include "FileSystemCollection.h"
#include "ResourceManager.h"
#include "Utility.h"
#include "../Common/Gizmo/Gizmo.h"
#include "../Common/GroundPlaneComponent.h"

#define _WINDOWS
#include "../../MCD/Binding/Launcher.h"
#include "../../MCD/Component/Render/CameraComponent.h"
#include "../../MCD/Component/Render/PickComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/System/MemoryProfiler.h"
#include "../../MCD/Core/System/ResourceManager.h"
#include "../../MCD/Core/System/WindowEvent.h"
#include "../../MCD/Render/Camera.h"
#include "../../MCD/Render/GlWindow.h"
#include "../../3Party/glew/glew.h"
#include "../../3Party/glew/wglew.h"
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

class RenderPanelControlImpl : public GlWindow
{
public:
	RenderPanelControlImpl(RenderPanelControl^ c, ResourceManager^ mgr)
		:
		mBackRef(c),
		mWidth(0), mHeight(0), mFieldOfView(60.0f),
		mGizmo(nullptr), mEntityPicker(nullptr),
		mPredefinedSubTree(nullptr), mUserSubTree(nullptr),
		mResourceManager(*mgr->getRawPtr()),
		mPropertyGridNeedRefresh(false),
		mPlaying(false),
		mLauncher(*mgr->fileSystemCollection->getRawPtr(), *mgr->getRawPtr(), false)
	{
		mRootNode.name = L"Ultimate root node";
	}

	~RenderPanelControlImpl()
	{
		// Maually remove all entity, before the Launcher get destroyed.
		while(mRootNode.firstChild())
			delete mRootNode.firstChild();
		mLauncher.setRootNode(nullptr);
	}

	void createScene()
	{
		{	// Setup user defined sub-tree
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = L"Studio user defined sub-tree";
			e->asChildOf(&mRootNode);
			mUserSubTree = e.release();
		}

		{	// Setup pre-defined sub-tree
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = L"Studio pre-defined sub-tree";
			e->asChildOf(&mRootNode);
			mPredefinedSubTree = e.release();
		}

		{	// Add the ground plane
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = L"Ground plane";
			e->asChildOf(mPredefinedSubTree);

			// Add component
			GroundPlaneComponent* c = new GroundPlaneComponent;
			e->addComponent(c);

			e.release();
		}

		{	// Add picking detector
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = L"Entity picker";
			e->asChildOf(mPredefinedSubTree);
			mEntityPicker = new PickComponent;
			mEntityPicker->entityToPick = mUserSubTree;
			e->addComponent(mEntityPicker);

			e.release();
		}

		{	// Add a C# contorl event input component
			mCsInputComponent = new CsInputComponent();
			mCsInputComponent->attachTo(mBackRef);
			mLauncher.init(*mCsInputComponent, mUserSubTree.get());
		}

		{	// Add a default camera
			std::auto_ptr<MCD::Entity> e(new MCD::Entity);
			e->name = L"Default camera";
			e->asChildOf(mPredefinedSubTree);

			// Add component
			mCamera = static_cast<CameraComponent*>(mLauncher.scriptComponentManager.runScripAsComponent(
				L"return loadComponent(\"StudioFpsCamera.nut\");"
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
			e->name = L"Gizmo";
			e->enabled = false;	// The gizmo is initially disable, until an object is selected
			e->asChildOf(&mRootNode);
			e->setActiveGizmo(e->translationGizmo.get());

			mGizmo = e.release();
		}
	}

	void update()
	{
		makeActive();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_LIGHTING);

		mLauncher.update();

		// Update pre-defined tree
		if(!mPlaying) {
			BehaviourComponent::traverseEntities(mPredefinedSubTree);
			RenderableComponent::traverseEntities(mPredefinedSubTree);

			// Draw the Gizmo
			if(mGizmo && mGizmo->enabled) {
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_LIGHTING);
				glClear(GL_DEPTH_BUFFER_BIT);
				BehaviourComponent::traverseEntities(mGizmo);
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

		swapBuffers();
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

	void executeScript(const wchar_t* scriptFilePath)
	{
		mLauncher.scriptComponentManager.doFile(scriptFilePath, true);
	}

	void play(const wchar_t* scriptFilePath)
	{
		// Backup the old sub-tree and input component
		mOldUserSubTree = mUserSubTree->clone();
		mOldCsInputComponent = new CsInputComponent();
		std::swap(mUserSubTree, mOldUserSubTree);
		std::swap(mCsInputComponent, mOldCsInputComponent);

		// Use the new sub-tree
		mOldUserSubTree->unlink();
		mCsInputComponent->attachTo(mBackRef);
		mLauncher.setRootNode(mUserSubTree.get());
		mLauncher.setInputComponent(mCsInputComponent.get());
		mUserSubTree->asChildOf(&mRootNode);

		mLauncher.scriptComponentManager.doFile(scriptFilePath, true);
		mCamera->entity()->enabled = false;
		mPlaying = true;
	}

	void stop()
	{
		// Restore the old backup sub-tree and input component
		std::swap(mUserSubTree, mOldUserSubTree);
		std::swap(mCsInputComponent, mOldCsInputComponent);
		mUserSubTree->asChildOf(&mRootNode);
		mLauncher.scriptComponentManager.shutdown();
		mLauncher.setRootNode(mUserSubTree.get());
		mLauncher.setInputComponent(mCsInputComponent.get());
		delete mOldUserSubTree.get();

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

	gcroot<RenderPanelControl^> mBackRef;
	float mWidth, mHeight;
	float mFieldOfView;
	MCD::Entity mRootNode, *mPredefinedSubTree;
	MCD::EntityPtr mUserSubTree;
	Gizmo* mGizmo;
	MCD::PickComponent* mEntityPicker;
	MCD::WeakPtr<CameraComponent> mCamera;
	IResourceManager& mResourceManager;
	bool mPropertyGridNeedRefresh;
	Point mLastMousePos;
	bool mPlaying;

	Launcher mLauncher;
	MCD::EntityPtr mOldUserSubTree;
	MCD::WeakPtr<CsInputComponent> mCsInputComponent, mOldCsInputComponent;
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

IntPtr RenderPanelControl::glContext::get()
{
	if(!mImpl)
		return IntPtr(nullptr);
	return IntPtr(mImpl->glContext());
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

void RenderPanelControl::executeScript(System::String^ scriptFilePath)
{
	mImpl->executeScript(Utility::toWString(scriptFilePath).c_str());
}

void RenderPanelControl::play(String^ scriptFilePath)
{
	MemoryProfiler::Scope profiler("RenderPanelControl::play");

	mImpl->play(Utility::toWString(scriptFilePath).c_str());

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

System::Void RenderPanelControl::timer_Tick(System::Object^ sender, System::EventArgs^ e)
{
	MCD::MemoryProfiler::singleton().nextFrame();
	this->update();
}

System::Void RenderPanelControl::RenderPanelControl_Load(System::Object^ sender, System::EventArgs^ e)
{
	MCD_ASSERT(mImpl == nullptr);
	mImpl = new RenderPanelControlImpl(this, mResourceManager);
	mImpl->create(Handle.ToPointer(), nullptr);

	void* sharedGlContext = mSharedGlContext.ToPointer();
	if(sharedGlContext) {
		void* thisGlContext = mImpl->glContext();
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
