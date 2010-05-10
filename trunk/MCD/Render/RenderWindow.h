#ifndef __MCD_RENDER_RENDERWINDOW__
#define __MCD_RENDER_RENDERWINDOW__

#include "ShareLib.h"
#include "../Core/System/Window.h"

namespace MCD {

/*!	An application window with 3D rendering.
	Example:
	\code
	RenderWindow window;
	window.create("title='Hello world!';width=400;height=300");
	window.setOptions("showCursor=0");
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
class MCD_RENDER_API RenderWindow : public Window
{
public:
	RenderWindow();
	sal_override ~RenderWindow();

	//! Get the underlaying render context.
	void* renderContext();

	/*!	Create a new window.
		Additional supported options:

		FSAA = '0|2|4|16' // Full-screen anti-aliasing\n
	 */
	sal_override void create(sal_in_z_opt const char* options=nullptr) throw(std::exception);

	/*!	Create a window from an existing control.
	 */
	sal_override void create(Handle existingControl, sal_in_z_opt const char* options=nullptr) throw(std::exception);

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

	static void* getActiveContext();

protected:
	sal_override void onEvent(const Event& eventReceived);

	class Impl;
};	// RenderWindow

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERWINDOW__
