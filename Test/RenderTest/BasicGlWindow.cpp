#include "Pch.h"
#include "BasicGlWindow.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Core/System/WindowEvent.h"
#include "../../3Party/glew/glew.h"
#include <iostream>

#if defined(MCD_VC)
#	pragma comment(lib, "OpenGL32")
#	pragma comment(lib, "GLU32")
#	pragma comment(lib, "GlAux")
#endif

using namespace MCD;

BasicGlWindow::BasicGlWindow(const wchar_t* options)
	:
	mIsClosing(false),
	mFieldOfView(60.0f),
	mIteration(0)
{
	Log::start(&std::wcout);
	Log::setLevel(Log::Level(Log::Error | Log::Warn | Log::Info));

	create(options);

	// A lot of opengl options to be enabled by default
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);			// OpenGl use counterclockwise as the default winding
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	// Automatic normalization (usefull when we have uniform scaled the model)
	// Reference: http://www.opengl.org/resources/features/KilgardTechniques/oglpitfall/
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Set up and enable light 0
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	GLfloat lightPos[] = { -50.f, 0.0f, 40.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
}

BasicGlWindow::~BasicGlWindow()
{
	Log::stop(false);
}

void BasicGlWindow::mainLoop()
{
	while(true) {
		Event e;

		if(popEvent(e, false))
		{
			if(e.Type == Event::Closed)
				break;

			switch(e.Type)
			{
			case Event::MouseWheelMoved:
				mFieldOfView -= float(e.MouseWheel.Delta);
				setFieldOfView(mFieldOfView);
				break;

			case Event::Resized:
				mWidth = e.Size.Width;
				mHeight = e.Size.Height;

				// Prevents division by zero
				mHeight = (mHeight == 0) ? 1 : mHeight;
				glViewport(0, 0, mWidth, mHeight);
				setFieldOfView(mFieldOfView);
				break;

			default:
				// Just ignore the remaining events
				break;
			}
		}

		update();
	}
}

void BasicGlWindow::preUpdate()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void BasicGlWindow::update(float deltaTime)
{
	(void)deltaTime;
}

void BasicGlWindow::onClose()
{
	mIsClosing = true;
}

void BasicGlWindow::postUpdate()
{
	glFlush();
	swapBuffers();
}

void BasicGlWindow::setFieldOfView(float angle)
{
	mFieldOfView = angle;

	// Reset coordinate system
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Define the "viewing volume"
	// Produce the perspective projection
	gluPerspective(mFieldOfView, (GLfloat)mWidth/(GLfloat)mHeight, 1.0f, 1000.0f);

	// Restore back to the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void BasicGlWindow::update()
{
	preUpdate();

	float deltaTime = float(mTimer.getDelta().asSecond());
	if(++mIteration % 1000  == 0)
		printf("FPS: %f\n", 1.0 / deltaTime);

	update(deltaTime);
	postUpdate();
}
