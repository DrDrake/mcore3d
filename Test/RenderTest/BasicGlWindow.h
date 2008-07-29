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
		glFrontFace(GL_CW);
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

			preUpdate();

			float deltaTime = float(mTimer.getDelta().asSecond());
			if(++mIteration % 1000  == 0)
				printf("FPS: %f\n", 1.0 / deltaTime);

			update(deltaTime);
			postUpdate();
		}
	}

	void preUpdate()
	{
		glClear(GL_COLOR_BUFFER_BIT);

		// Clear matrix stack
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	virtual void update(float deltaTime) {
		(void)deltaTime;
	}

	sal_override void onClose() {
		mIsClosing = true;
	}

	void postUpdate() {
		swapBuffers();
	}

protected:
	bool mIsClosing;
	size_t mIteration;
	MCD::DeltaTimer mTimer;
};	// BasicGlWindow
