#ifndef __MCD_RENDER_GLWINDOW__
#define __MCD_RENDER_GLWINDOW__

#include "ShareLib.h"
#include "../Core/System/Window.h"

namespace MCD {

/*!	An application window with OpenGl.
	Example:
	\code
	GlWindow window;
	window.create(L"title='Hello world!';width=400;height=300");
	window.setOptions(L"showCursor=0");
	window.makeActive();

	while(true) {
		Event e;
		// We keep render even without any window event, therefore we
		// use non-blocking mode
		bool hasEvent = window.popEvent(e, false);

		if(hasEvent && e.Type == Event::Closed) {
			window.destroy();
			break;
		}

		// Do you drawing here
		// ...

		// Call swapBuffers() to present your drawing to the front buffer
		swapBuffers();
	}
	\endcode
 */
class MCD_RENDER_API GlWindow : public Window
{
public:
	GlWindow();
	sal_override ~GlWindow();

	/*!	Create a new window.
		Additional supported options:

		FSAA = '0|2|4|16' // Full-screen anti-aliasing\n
	 */
	sal_override void create(sal_in_z_opt const wchar_t* options=nullptr) throw(std::exception);

	/*!	Create a window from an existing control.
	 */
	sal_override void create(Handle existingControl, sal_in_z_opt const wchar_t* options=nullptr) throw(std::exception);

	/*!	Destroy the window and perform necessary cleanup.
	 */
	sal_override void destroy();

	//! Making this opengl window be the current render context.
	bool makeActive();

	/*!	Swap the front and back buffers.
		Most likely you need to invoke this function after all your drawing commands
		to end a frame.
	 */
	bool swapBuffers();

	//! Enable/disable vertical sync.
	bool setVerticalSync(bool flag);

protected:
	sal_override void onEvent(const Event& eventReceived);

	class Impl;
};	// GlWindow

}	// namespace MCD

#endif	// __MCD_RENDER_GLWINDOW__
