// This inc file is to be included by GlWindow.cpp only

#include "../Core/System/StrUtility.h"
#include "../Core/System/Window.X11.inl"
#include "../../3Party/glew/wglew.h"
#include <GL/glx.h>

namespace MCD {

class GlWindow::Impl : public Window::Impl
{
public:
	typedef Window::Impl Super;

	Impl(Window& w)
		: Super(w)
	{
	}

	~Impl()
	{
		destroy();
	}

	sal_override void createNewWindow()
	{
		MCD_ASSERT(!mWnd && "A window is already created, call destroy() first");

		// Open the display at first call
		openDisplay();

		static int attributes[] = {
			GLX_RGBA,
			GLX_DOUBLEBUFFER,
			GLX_DEPTH_SIZE, 16,
			GLX_RED_SIZE, 8,
			GLX_BLUE_SIZE, 8,
			GLX_GREEN_SIZE, 8,
			0
		};

		XVisualInfo* visualInfo = ::glXChooseVisual(gDisplay, DefaultScreen(gDisplay), attributes);

		XSetWindowAttributes winAttribs;
		winAttribs.event_mask =
			ExposureMask | VisibilityChangeMask |
			KeyPressMask | PointerMotionMask |
			StructureNotifyMask ;

		winAttribs.border_pixel = 0;
		winAttribs.bit_gravity = StaticGravity;
		winAttribs.colormap = ::XCreateColormap(gDisplay, RootWindow(gDisplay, visualInfo->screen), visualInfo->visual, AllocNone);
		unsigned long winmask = CWBorderPixel | CWBitGravity | CWEventMask| CWColormap;

		Super::mWnd = ::XCreateWindow(
			gDisplay, DefaultRootWindow(gDisplay), 20, 20,
			mWidth, mHeight, 0,
			visualInfo->depth, InputOutput,
			visualInfo->visual, winmask, &winAttribs
		);

		if(!mWnd)
			throw std::runtime_error("X11 API call 'XCreateWindow' failed");

		// Set the window title
		::XStoreName(gDisplay, mWnd, wStr2Str(mTitle).c_str());

		::XMapWindow(gDisplay, mWnd);

		// Get the atom defining the close event
		mAtomClose = ::XInternAtom(gDisplay, "WM_DELETE_WINDOW", false);
		::XSetWMProtocols(gDisplay, mWnd, &mAtomClose, 1);

		// Create the input context
		if(gInputMethod) {
			mInputContext = ::XCreateIC(gInputMethod,
				XNClientWindow,	mWnd,
				XNFocusWindow,	mWnd,
				XNInputStyle,	XIMPreeditNothing | XIMStatusNothing,
				//XNResourceName,  "MCD",
				//XNResourceClass, "MCD",
				nullptr
			);

			if(!mInputContext) {
				// TODO: Log error message
				//std::cerr << "Failed to create input context for window -- TextEntered event won't be able to return unicode" << std::endl;
			}
		}

		// Create the hiden cursor
		createHiddenCursor();

		// Create the OpenGl context
		mGlxContext = ::glXCreateContext(gDisplay, visualInfo, 0, True);
	}

	sal_override void destroy()
	{
		// Destroy the OpenGL context
		if(mGlxContext)
			::glXDestroyContext(gDisplay, mGlxContext);

		Super::destroy();
	}

	bool makeActive()
	{
		return ::glXMakeCurrent(gDisplay, mWnd, mGlxContext);
	}

	bool swapBuffers()
	{
		::glXSwapBuffers(gDisplay, mWnd);
		return true;
	}

	bool setVerticalSync(bool flag)
	{
		if(glewIsSupported("WGLEW_swap_control")) {
//			wglSwapIntervalEXT(flag ? 1 : 0);
			return true;
		}
		return false;
	}

	GLXContext mGlxContext;
};	// Impl

}	// namespace MCD
