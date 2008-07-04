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

	//! Get the handle
	Handle handle();

	/*!	Create a new window
	 */
	void create(sal_in_z_opt const wchar_t* options=nullptr) throw(std::runtime_error);

	/*!	Create a window from an existing control
	 */
	void create(Handle existingControl, sal_in_z_opt const wchar_t* options=nullptr) throw(std::runtime_error);

	void setOptions(sal_in_z sal_notnull const wchar_t* options);

	/*!	Destroy the window and perform necessary cleanup.
	 */
	void destroy();

	void processEvent(bool blocking);

	bool getEvent(Event& event, bool blocking);

protected:
	sal_override void onEvent(const Event& eventReceived);

protected:
	class Impl;
	Impl* mImpl;
};	// Window

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_WINDOW__
