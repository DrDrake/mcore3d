#ifndef __MCD_RENDER_OGLCONTEXT__
#define __MCD_RENDER_OGLCONTEXT__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include <stdexcept>

namespace MCD {

/*!	Opengl context.
 */
class MCD_RENDER_API OglContext : Noncopyable
{
public:
#ifdef _WINDOWS
	// Windows defines a void* handle (HWND)
	typedef void* Handle;
#else
    // Unix - X11 defines an unsigned integer handle (Window)
    typedef unsigned long Handle;
#endif

	// TODO: Add options like enable v-sync
	//! Create an Opengl context using a window handle.
	OglContext(Handle windowHandle) throw(std::runtime_error);

	~OglContext();

	//! Making this OglContext be the current one.
	bool makeActive();

	/*!	Swap the front and back buffers.
		Most likey you need to invoke this function after all your drawing commands
		to end a frame.
	 */
	bool swapBuffers();

protected:
	class Impl;
	Impl* mImpl;
};	// OglContext

}	// namespace MCD

#endif	// __MCD_RENDER_OGLCONTEXT__
