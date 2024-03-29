// User must include "Window.h" before Window.X11.inl

#include "PlatformInclude.h"
#include "WindowEvent.h"

#include <X11/Xlib.h>	// XOpenDisplay
#include <X11/keysym.h>
#include <X11/XKBlib.h>	// XkbSetDetectableAutoRepeat

namespace MCD {

extern Display* gDisplay;
extern int gDisplayCount;
extern int gScreen;
extern XIM gInputMethod;
extern Cursor gHiddenCursor;	//!< As X11 doesn't provide cursor hidding, we must create a transparent one

//! Open the display (if not already done)
extern void openDisplay();

//! Close the display, in pair with openDisplay()
extern void closeDisplay();

//! Convert a X11 keysym to our key code
extern Key::Code ConvertKeyCode(KeySym sym);

class Window::Impl : public ImplBase
{
public:
	Impl(Window& w);

	void createWindow(Window::Handle existingWindowHandle=0);

	void destroy();

	/*! Invoked for X11 "DestroyNotify" event, which may need to do some cleanup.
		Derived class can overrided this function to perform specific cleanup.
	 */
	virtual void onDestroy() {}

	//////////////////////////////////////////////////////////////////////////
	// Begin of setting options

	// "curosrPosition = 'x=123; y=456'"
	void setCursorPosition(const char* value);

	// "height = 600"
	void setHeight(const char* value);

	// "show = 0|1"
	void setShowWindow(const char* value);

	// "showCursor = 0|1"
	void setShowCursor(const char* value);

	// "title = 'Simple game engine'"
	void setTitle(const char* value);

	// "width = 800"
	void setWidth(const char* value);

	sal_override void setOption(const char* name, const char* value);

	void processEvent(bool blocking);

	void processEvent(XEvent xEvent);

	//! Create a transparent mouse cursor (tricks to hide the cursor)
	void createHiddenCursor();

	Window& mWindow;
	::Window mWnd;
	Atom mAtomClose;		//!< Atom used to identify the close event
	XIC mInputContext;		//!< Input context used to get unicode input in our window
};	// Impl

}	// namespace MCD
