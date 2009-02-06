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
		: mFieldOfView(60.0f), mResourceManager(*createDefaultFileSystem())
	{

		{	// Setup entity 1
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"ChamferBox 1";
			e->link(&mRootNode);
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
	}

	void update()
	{
		mResourceManager.processLoadingEvents();

		glClearColor(0.5f, 0.5f, 0.5f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		mCamera.applyTransform();

		glTranslatef(0.0f, 0.0f, -10.0f);
		RenderableComponent::traverseEntities(&mRootNode);

		glFlush();
		swapBuffers();
	}

	void setFieldOfView(float angle, size_t width, size_t height)
	{
		mFieldOfView = angle;

		// Reset coordinate system
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		// Define the "viewing volume"
		// Produce the perspective projection
		gluPerspective(
			mFieldOfView,			// The camera angle ... field of view in y direction
			(GLfloat)width/height,	// The width-to-height ratio
			1.0f,					// The near z clipping coordinate
			500.0f);				// The far z clipping coordinate

		// Restore back to the model view matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	void resize(size_t width, size_t height)
	{
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
		setFieldOfView(mFieldOfView, width, height);
	}

	float mFieldOfView;
	Entity mRootNode;
	Camera mCamera;
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

System::Void RenderPanelControl::RenderPanelControl_Load(System::Object^ sender, System::EventArgs^ e)
{
	MCD_ASSERT(mImpl == nullptr);
	mImpl = new RenderPanelControlImpl;
	mImpl->create(Handle.ToPointer(), nullptr);
	mImpl->makeActive();
}

System::Void RenderPanelControl::RenderPanelControl_Resize(System::Object^ sender, System::EventArgs^ e)
{
}

System::Void RenderPanelControl::timer1_Tick(System::Object^ sender, System::EventArgs^ e)
{
	if(!mImpl)
		return;

	String^ s = this->Name;
	mImpl->update();
}

System::Void RenderPanelControl::RenderPanelControl_Enter(System::Object^ sender, System::EventArgs^ e)
{
	timer1->Start();
	mImpl->makeActive();
}

System::Void RenderPanelControl::RenderPanelControl_Leave(System::Object^ sender, System::EventArgs^ e)
{
	timer1->Stop();
}

System::Void RenderPanelControl::RenderPanelControl_SizeChanged(System::Object^ sender, System::EventArgs^ e)
{
	mImpl->resize(this->Width, this->Height);
}

}
