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

	void createWindow(Window::Handle existingWindowHandle=0);

	void destroy();

	/*! Invoked for Win32 "WM_DESTROY" event, which may need to do some cleanup.
		Derived class can overrided this function to perform specific cleanup.
	 */
	virtual void onDestroy();

	//////////////////////////////////////////////////////////////////////////
	// Begin of setting options

	// "colorBits = 32"
	void setColorBits(const char* value);

	// "curosrPosition = 'x=123; y=456'"
	void setCursorPosition(const char* value);

	// "fullscreen = 0|1"
	void setFullscreen(const char* value);
	void setFullscreen(bool flag);

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

	// End of setting options
	//////////////////////////////////////////////////////////////////////////

	sal_override void processEvent(bool blocking);

	static LRESULT CALLBACK wndProc(sal_in HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT Proc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND mWnd;
	Window& mWindow;
	DEVMODE mDisplaySettingBackup;	//! Backup of the current display setting before we go into fullscreen mode
	uint8_t mColorBits;		//! Number of bits per pixel
	bool mFullScreen;
	int mShowWindow;
};	// Impl

}	// namespace MCD
