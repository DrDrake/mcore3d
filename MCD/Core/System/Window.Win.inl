// User must include "Window.h" before Window.Win.inl

#include "PlatformInclude.h"
#include "WindowEvent.h"

namespace MCD {

static const wchar_t* cWindowClass = L"MCD_Window";
extern size_t gWindowCount;

//! Check the state of the shift keys on a key event, and return the corresponding key code
extern Key::Code GetShiftState(bool keyDown);

//! Convert a Win32 virtual key code to our key code
extern Key::Code ConvertKeyCode(WPARAM virtualKey, LPARAM flags);

class MCD_CORE_API Window::Impl : public ImplBase
{
public:
	Impl(Window& w);

	void createNewWindow();

	void destroy();

	//////////////////////////////////////////////////////////////////////////
	// Begin of setting options

	// "curosrPosition = 'x=123; y=456'"
	void setCursorPosition(const wchar_t* value);

	// "height = 600"
	void setHeight(const wchar_t* value);

	// "mode = 'fullscreen=0|1; width=800; height=600;'
	void setWindowMode(const wchar_t*);

	// "show = 0|1"
	void setShowWindow(const wchar_t* value);

	// "showCursor = 0|1"
	void setShowCursor(const wchar_t* value);

	// "title = 'Simple game engine'"
	void setTitle(const wchar_t* value);

	// "width = 800"
	void setWidth(const wchar_t* value);

	sal_override void setOption(const wchar_t* name, const wchar_t* value);

	// End of setting options
	//////////////////////////////////////////////////////////////////////////

	sal_override void processEvent(bool blocking);

	static LRESULT CALLBACK wndProc(sal_in HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT Proc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND mWnd;
	Window& mWindow;
};	// Impl

}	// namespace MCD
