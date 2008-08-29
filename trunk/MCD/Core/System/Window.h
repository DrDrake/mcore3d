#ifndef __MCD_CORE_SYSTEM_WINDOW__
#define __MCD_CORE_SYSTEM_WINDOW__

#include "NonCopyable.h"
#include "../ShareLib.h"
#include <stdexcept>

namespace MCD {

class Event;
/*!	Base class for classes that want to receive events.
	For internal use only.
 */
class MCD_NOVTABLE IWindowListener
{
public:
	virtual ~IWindowListener() {}

	//! Called each time an event is received from attached window.
	virtual void onEvent(const Event& eventReceived) = 0;
};	// IWindowListener

/*!	An application window.
	Example:
	\code
	Window window;
	window.create(L"title='Hello world!';width=400;height=300");
	window.setOptions(L"showCursor=0");

	while(true) {
		Event e;
		window.popEvent(e, true);

		if(e.Type == Event::Closed) {
			window.destroy();
			break;
		}
	}
	\endcode
 */
class MCD_CORE_API Window : protected IWindowListener, Noncopyable
{
public:
	Window();
	virtual ~Window();

#ifdef _WINDOWS
	// Windows defines a void* handle (HWND)
	typedef void* Handle;
#else
    // Unix - X11 defines an unsigned integer handle (Window)
    typedef unsigned long Handle;
#endif

	//! Get the platform dependent handle to this window
	Handle handle();

	/*!	Create a new window.
		\param options A name/value pair string that describe the properties of the window
		\sa NvpParser

		Supported options on all platforms:

		curosrPosition = 'x=positive number; y=positive number' \n
		height = positive number \n
		show = 0|1 \n
		showCursor = 0|1 \n
		title = 'Your window title' \n
		width = positive number \n

		Supported options on Microsoft Windows:
		colorBits = 4|8|16|32 \n
		fullscreen = 0|1 \n
	 */
	void create(sal_in_z_opt const wchar_t* options=nullptr) throw(std::exception);

	/*!	Create a window from an existing control.
	 */
	void create(Handle existingControl, sal_in_z_opt const wchar_t* options=nullptr) throw(std::exception);

	/*!	Change some of the properties of the window.
		\note Not all options that can be set in create() can be applied here,
			for instance we cannot change the fullscreen mode once the window is created.

		Supported options on all platforms:

		curosrPosition = 'x=positive number; y=positive number' \n
		show = 0|1 \n
		showCursor = 0|1 \n
	 */
	void setOptions(sal_in_z sal_notnull const wchar_t* options);

	/*!	Destroy the window and perform necessary cleanup.
		\note Will be invoked in ~Window()
	 */
	void destroy();

	/*!	Extract events from the OS API.
		This function extract and translates the platform dependent messages into our Event class,
		then invoke onEvent() to inform the user.
	 */
	void processEvent(bool blocking);

	/*!	Pop an event out of the event queue.
		Implicitly call processEvent() if the event queue is empty.
		\param event Output parameter to get the event.
		\param blocking If it's true, the function will block until an event happen.
		\return false if the event queue is empty, meaningful for non-blocking mode only.
	 */
	bool popEvent(Event& event, bool blocking);

	//! Width of the window client area in pixel.
	uint width() const;

	//! Height of the window client area in pixel.
	uint height() const;

protected:
	/*!	Invoked by processEvent() to tell the user about the event happened.
		The implementation of Window::onEvent() will put the event into it's
		event queue so that user can query about it using popEvent().
		User can override this function and perform different actions base on
		the event rather than put it into the event queue.
	 */
	sal_override void onEvent(const Event& eventReceived);

protected:
	class Impl;
	Impl* mImpl;
};	// Window

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_WINDOW__
