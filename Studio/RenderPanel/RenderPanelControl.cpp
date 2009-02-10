#include "stdafx.h"
#include "RenderPanelControl.h"
#include "../Common/ChamferBox.h"
#include "../Common/DefaultResourceManager.h"

#define _WINDOWS
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/FileSystem.h"
#include "../../MCD/Core/System/WindowEvent.h"
#include "../../MCD/Render/Camera.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/GlWindow.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Components/CameraComponent.h"
#include "../../MCD/Render/Components/MeshComponent.h"
#include "../../3Party/glew/glew.h"

#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32")
#pragma comment(lib, "GlAux")
//#pragma comment(lib, "glew")

namespace RenderPanel {

using namespace MCD;

class RenderPanelControlImpl : public GlWindow
{
public:
	RenderPanelControlImpl()
		:
		mFieldOfView(60.0f), mPredefinedSubTree(nullptr), mUserSubTree(nullptr),
		mResourceManager(*createDefaultFileSystem())
	{
	}

	void createScene()
	{
		{	// Setup user defined sub-tree
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"Studio user defined sub-tree";
			e->link(&mRootNode);
			mPredefinedSubTree = e.release();
		}

		{	// Setup pre-defined sub-tree
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"Studio pre-defined sub-tree";
			e->link(&mRootNode);
			mUserSubTree = e.release();
		}

		{	// Add a default camera
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"Default camera";
			e->link(mPredefinedSubTree);
			e->localTransform.setTranslation(Vec3f(0, 0, 5));

			// Add component
			mCamera = new CameraComponent;
			// We relay on Entity's transform rather than the position of MCD::Camera
			mCamera->camera.position = Vec3f(0, 0, 0);
			mCamera->camera.lookAt = Vec3f(0, 0, -1);
			mCamera->camera.upVector = Vec3f(0, 1, 0);
			mCamera->clearColor = ColorRGBf(0.5f);	// Set the background color as 50% gray
			e->addComponent(mCamera.get());

			e.release();
		}

		{	// Setup entity 1
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"ChamferBox 1";
			e->link(mUserSubTree);
			e->localTransform = Mat44f::rotate(0, Mathf::cPiOver4(), 0);

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);
			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

			// Add component
			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
			e->addComponent(c);

			e.release();
		}

		{	// Setup entity 2
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"Sphere 1";
			e->link(mUserSubTree);
			e->localTransform.setTranslation(Vec3f(1, 0, 0));

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(1.0f, 10);
			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

			// Add component
			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
			e->addComponent(c);

			e.release();
		}
	}

	void update()
	{
		makeActive();
		mResourceManager.processLoadingEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderableComponent::traverseEntities(&mRootNode);

		glFlush();
		swapBuffers();
	}

	void resize(size_t width, size_t height)
	{
		// Prevents problem when the application is minmized
		if(width == 0 || height == 0)
			return;

		makeActive();

		// A lot of opengl options to be enabled by default
		glShadeModel(GL_SMOOTH);
		glFrontFace(GL_CCW);			// OpenGl use counterclockwise as the default winding
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		// Automatic normalization (useful when we have uniform scaled the model)
		// Reference: http://www.opengl.org/resources/features/KilgardTechniques/oglpitfall/
		glEnable(GL_RESCALE_NORMAL);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		// Set up and enable light 0
		glEnable(GL_LIGHTING);
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
		if(mCamera.get())
			mCamera->camera.frustum.setAcpectRatio(float(width)/height);

		update();
	}

	void onKeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
	{
		switch(e->KeyCode) {
		case System::Windows::Forms::Keys::W:
			mCamera->camera.moveForward(0.5f);
			break;
		}
	}

	void onKeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
	{
	}

	float mFieldOfView;
	Entity mRootNode, *mPredefinedSubTree, *mUserSubTree;
	WeakPtr<CameraComponent> mCamera;
	DefaultResourceManager mResourceManager;
};	// RenderPanelControlImpl

RenderPanelControl::RenderPanelControl()
{
	InitializeComponent();
	mImpl = nullptr;
}

RenderPanelControl::~RenderPanelControl()
{
	if(components)
	{
		delete components;
	}

	this->!RenderPanelControl();
}

RenderPanelControl::!RenderPanelControl()
{
	delete mImpl;
	mImpl = nullptr;	// Finializer may call more than once
}

::Binding::Entity^ RenderPanelControl::rootEntity::get()
{
	return gcnew ::Binding::Entity(IntPtr(&(mImpl->mRootNode)));
}

System::Void RenderPanelControl::RenderPanelControl_Load(System::Object^ sender, System::EventArgs^ e)
{
	MCD_ASSERT(mImpl == nullptr);
	mImpl = new RenderPanelControlImpl;
	mImpl->create(Handle.ToPointer(), nullptr);
	mImpl->makeActive();
	mImpl->createScene();
}

System::Void RenderPanelControl::timer1_Tick(System::Object^ sender, System::EventArgs^ e)
{
	if(!mImpl)
		return;
	mImpl->update();
}

System::Void RenderPanelControl::RenderPanelControl_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e)
{
	if(!mImpl)
		return;
	mImpl->update();
}

System::Void RenderPanelControl::RenderPanelControl_Enter(System::Object^ sender, System::EventArgs^ e)
{
	mImpl->resize(this->Width, this->Height);
	timer1->Start();
}

System::Void RenderPanelControl::RenderPanelControl_Leave(System::Object^ sender, System::EventArgs^ e)
{
	timer1->Stop();
}

System::Void RenderPanelControl::RenderPanelControl_SizeChanged(System::Object^ sender, System::EventArgs^ e)
{
	mImpl->resize(this->Width, this->Height);
}

System::Void RenderPanelControl::RenderPanelControl_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
{
	mImpl->onKeyDown(sender, e);
}

System::Void RenderPanelControl::RenderPanelControl_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
{
	mImpl->onKeyUp(sender, e);
}

}
