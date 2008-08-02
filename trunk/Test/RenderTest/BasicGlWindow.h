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
	BasicGlWindow(const wchar_t* options) : mIsClosing(false), mIteration(0)
	{
		create(options);

		// A lot of opengl options to be enabled by default
		glShadeModel(GL_SMOOTH);
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	void mainLoop()
	{
		while(true) {
			Event e;
			popEvent(e, false);

			if(e.Type == Event::Closed)
				break;
			if(e.Type == Event::Resized) {
				uint w = e.Size.Width;
				uint h = e.Size.Height;
				mW = w;
				mH = h;

				// Prevents division by zero
				h = (h == 0) ? 1 : h;
				glViewport(0, 0, w, h);

				// Reset coordinate system
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				// Define the "viewing volume"

				// Produce the perspective projection
				gluPerspective(60.0f, (GLfloat)w/(GLfloat)h, 1.0f, 1000.0f);

				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
			}

			preUpdate();

			float deltaTime = float(mTimer.getDelta().asSecond());
			if(++mIteration % 1000  == 0)
				printf("FPS: %f\n", 1.0 / deltaTime);

			update(deltaTime);
			postUpdate();
		}
	}

	void preUpdate() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

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
	uint mW, mH;
	bool mIsClosing;
	size_t mIteration;
	MCD::DeltaTimer mTimer;
};	// BasicGlWindow
