#ifndef __MCD_RENDER_GLWINDOW__
#define __MCD_RENDER_GLWINDOW__

#include "ShareLib.h"
#include "../Core/System/Window.h"

namespace MCD {

class MCD_RENDER_API GlWindow : public Window
{
public:
	GlWindow();
	sal_override ~GlWindow();

	/*!	Create a new window
	 */
	sal_override void create(sal_in_z_opt const wchar_t* options=nullptr) throw(std::exception);

	/*!	Create a window from an existing control
	 */
	sal_override void create(Handle existingControl, sal_in_z_opt const wchar_t* options=nullptr) throw(std::exception);

	/*!	Destroy the window and perform necessary cleanup.
	 */
	sal_override void destroy();

	//! Making this opengl window be the current render context.
	bool makeActive();

	/*!	Swap the front and back buffers.
		Most likey you need to invoke this function after all your drawing commands
		to end a frame.
	 */
	bool swapBuffers();

	//! Enable/disable vertical sync.
	bool setVerticalSync(bool flag);

protected:
	class Impl;
};	// GlWindow

}	// namespace MCD

#endif	// __MCD_RENDER_GLWINDOW__
