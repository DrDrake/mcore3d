#ifndef __MCD_RENDERTEST_BASICGLWINDOW__
#define __MCD_RENDERTEST_BASICGLWINDOW__

#include "../../MCD/Render/GlWindow.h"
#include "../../MCD/Core/System/Timer.h"
#include "../../MCD/Core/System/WindowEvent.h"
#include "../../3Party/glew/glew.h"

#if defined(MCD_VC)
#	pragma comment(lib, "OpenGL32")
#	pragma comment(lib, "GLU32")
#	pragma comment(lib, "GlAux")
#endif

class BasicGlWindow : public MCD::GlWindow
{
public:
	BasicGlWindow(const wchar_t* options)
		:
		mIsClosing(false),
		mFieldOfView(60.0f),
		mIteration(0)
	{
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
	}

	void mainLoop()
	{
		using namespace MCD;

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

	void preUpdate() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	//! To be overriden by derived class to do the actual update
	virtual void update(float deltaTime) {
		(void)deltaTime;
	}

	sal_override void onClose() {
		mIsClosing = true;
	}

	void postUpdate()
	{
		glFlush();
		swapBuffers();
	}

protected:
	void setFieldOfView(float angle)
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

	void update()
	{
		preUpdate();

		float deltaTime = float(mTimer.getDelta().asSecond());
		if(++mIteration % 1000  == 0)
			printf("FPS: %f\n", 1.0 / deltaTime);

		update(deltaTime);
		postUpdate();
	}

protected:
	uint mWidth, mHeight;
	float mFieldOfView;
	bool mIsClosing;
	size_t mIteration;
	MCD::DeltaTimer mTimer;
};	// BasicGlWindow

#endif	// __MCD_RENDERTEST_BASICGLWINDOW__
