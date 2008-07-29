// This inc file is to be included by Window.cpp only

#include "Window.Win.inl"

namespace MCD {

size_t gWindowCount = 0;

//! Check the state of the shift keys on a key event, and return the corresponding key code
Key::Code GetShiftState(bool keyDown)
{
	static bool LShiftPrevDown = false;
	static bool RShiftPrevDown = false;

	bool LShiftDown = (HIWORD(::GetAsyncKeyState(VK_LSHIFT)) != 0);
	bool RShiftDown = (HIWORD(::GetAsyncKeyState(VK_RSHIFT)) != 0);

	Key::Code Code = Key::Code(0);
	if(keyDown)
	{
		if		(!LShiftPrevDown && LShiftDown) Code = Key::LShift;
		else if	(!RShiftPrevDown && RShiftDown) Code = Key::RShift;
	}
	else
	{
		if		(LShiftPrevDown && !LShiftDown) Code = Key::LShift;
		else if	(RShiftPrevDown && !RShiftDown) Code = Key::RShift;
	}

	LShiftPrevDown = LShiftDown;
	RShiftPrevDown = RShiftDown;

	return Code;
}

//! Convert a Win32 virtual key code to our key code
Key::Code ConvertKeyCode(WPARAM virtualKey, LPARAM flags)
{
	switch(virtualKey)
	{
		// VK_SHIFT is handled by the GetShiftState function
		case VK_MENU :			return (flags & (1 << 24)) ? Key::RAlt : Key::LAlt;
		case VK_CONTROL :		return (flags & (1 << 24)) ? Key::RControl : Key::LControl;
		case VK_LWIN :			return Key::LSystem;
		case VK_RWIN :			return Key::RSystem;
		case VK_APPS :			return Key::Menu;
		case VK_OEM_1 :			return Key::SemiColon;
		case VK_OEM_2 :			return Key::Slash;
		case VK_OEM_PLUS :		return Key::Equal;
		case VK_OEM_MINUS :		return Key::Dash;
		case VK_OEM_4 :			return Key::LBracket;
		case VK_OEM_6 :			return Key::RBracket;
		case VK_OEM_COMMA :		return Key::Comma;
		case VK_OEM_PERIOD :	return Key::Period;
		case VK_OEM_7 :			return Key::Quote;
		case VK_OEM_5 :			return Key::BackSlash;
		case VK_OEM_3 :			return Key::Tilde;
		case VK_ESCAPE :		return Key::Escape;
		case VK_SPACE :			return Key::Space;
		case VK_RETURN :		return Key::Return;
		case VK_BACK :			return Key::Back;
		case VK_TAB :			return Key::Tab;
		case VK_PRIOR :			return Key::PageUp;
		case VK_NEXT :			return Key::PageDown;
		case VK_END :			return Key::End;
		case VK_HOME :			return Key::Home;
		case VK_INSERT :		return Key::Insert;
		case VK_DELETE :		return Key::Delete;
		case VK_ADD :			return Key::Add;
		case VK_SUBTRACT :		return Key::Subtract;
		case VK_MULTIPLY :		return Key::Multiply;
		case VK_DIVIDE :		return Key::Divide;
		case VK_PAUSE :			return Key::Pause;
		case VK_F1 :			return Key::F1;
		case VK_F2 :			return Key::F2;
		case VK_F3 :			return Key::F3;
		case VK_F4 :			return Key::F4;
		case VK_F5 :			return Key::F5;
		case VK_F6 :			return Key::F6;
		case VK_F7 :			return Key::F7;
		case VK_F8 :			return Key::F8;
		case VK_F9 :			return Key::F9;
		case VK_F10 :			return Key::F10;
		case VK_F11 :			return Key::F11;
		case VK_F12 :			return Key::F12;
		case VK_F13 :			return Key::F13;
		case VK_F14 :			return Key::F14;
		case VK_F15 :			return Key::F15;
		case VK_LEFT :			return Key::Left;
		case VK_RIGHT :			return Key::Right;
		case VK_UP :			return Key::Up;
		case VK_DOWN :			return Key::Down;
		case VK_NUMPAD0 :		return Key::Numpad0;
		case VK_NUMPAD1 :		return Key::Numpad1;
		case VK_NUMPAD2 :		return Key::Numpad2;
		case VK_NUMPAD3 :		return Key::Numpad3;
		case VK_NUMPAD4 :		return Key::Numpad4;
		case VK_NUMPAD5 :		return Key::Numpad5;
		case VK_NUMPAD6 :		return Key::Numpad6;
		case VK_NUMPAD7 :		return Key::Numpad7;
		case VK_NUMPAD8 :		return Key::Numpad8;
		case VK_NUMPAD9 :		return Key::Numpad9;
		case 'A' :				return Key::A;
		case 'Z' :				return Key::Z;
		case 'E' :				return Key::E;
		case 'R' :				return Key::R;
		case 'T' :				return Key::T;
		case 'Y' :				return Key::Y;
		case 'U' :				return Key::U;
		case 'I' :				return Key::I;
		case 'O' :				return Key::O;
		case 'P' :				return Key::P;
		case 'Q' :				return Key::Q;
		case 'S' :				return Key::S;
		case 'D' :				return Key::D;
		case 'F' :				return Key::F;
		case 'G' :				return Key::G;
		case 'H' :				return Key::H;
		case 'J' :				return Key::J;
		case 'K' :				return Key::K;
		case 'L' :				return Key::L;
		case 'M' :				return Key::M;
		case 'W' :				return Key::W;
		case 'X' :				return Key::X;
		case 'C' :				return Key::C;
		case 'V' :				return Key::V;
		case 'B' :				return Key::B;
		case 'N' :				return Key::N;
		case '0' :				return Key::Num0;
		case '1' :				return Key::Num1;
		case '2' :				return Key::Num2;
		case '3' :				return Key::Num3;
		case '4' :				return Key::Num4;
		case '5' :				return Key::Num5;
		case '6' :				return Key::Num6;
		case '7' :				return Key::Num7;
		case '8' :				return Key::Num8;
		case '9' :				return Key::Num9;
	}

	return Key::Code(0);
}

Window::Impl::Impl(Window& w)
	:
	mWnd(nullptr), mWindow(w),
	mColorBits(32),
	mFullScreen(false)
{
	mListeners.insert(&w);
	mWidth = mHeight = uint(CW_USEDEFAULT);
}

void Window::Impl::createNewWindow()
{
	HMODULE hModule = ::GetModuleHandle(nullptr);

	if(gWindowCount == 0)
	{
		// Register window class
		WNDCLASS wc;
		::ZeroMemory(&wc, sizeof(wc));
		// CS_OWNDC is an optimization, see page 659 of OpenGL SuperBible 4th edition
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.hInstance = hModule;
		wc.lpfnWndProc = &wndProc;
		wc.lpszClassName = cWindowClass;
		wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = nullptr;	// (HBRUSH)(COLOR_WINDOW);

		if(::RegisterClass(&wc) == 0)
			throw std::runtime_error("Win32 API call 'RegisterClass' failed");
		++gWindowCount;
	}

	{	// Create window
		MCD_ASSERT(!mWnd && "A window is already created, call destroy() first");

		// OpenGL requires WS_CLIPCHILDREN and WS_CLIPSIBLINGS, see page 657 of OpenGL SuperBible 4th edition
		DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

		// In windowed mode, adjust width and height so that window will have the requested client area
		int windowWidth = mWidth, windowHeight = mHeight;
		if(!mFullScreen) {
			RECT rect = { 0, 0, mWidth, mHeight };
			::AdjustWindowRect(&rect, style, /*hasMenu*/false);
			windowWidth = (mWidth == CW_USEDEFAULT ? mWidth : rect.right - rect.left);
			windowHeight = (mHeight == CW_USEDEFAULT ? mHeight :  rect.bottom - rect.top);
		}

		mWnd = ::CreateWindow(
			cWindowClass, mTitle.c_str(),
			style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			windowWidth, windowHeight,
			nullptr, nullptr,
			hModule,
			nullptr
		);

		if(!mWnd)
			throw std::runtime_error("Win32 API call 'CreateWindow' failed");

		// Get the client area size from Win32 if user didn't supply one
		if(mWidth == CW_USEDEFAULT || mHeight == CW_USEDEFAULT) {
			RECT rect;
			::GetClientRect(mWnd, &rect);
			mWidth = rect.right - rect.left;
			mHeight = rect.bottom - rect.top;
		}

		if(mFullScreen)
			setFullscreen(true);
	}

	// Associate the this pointer as the user-data of the window handle
	::SetWindowLongPtr(mWnd, GWL_USERDATA, reinterpret_cast<LONG>(this));

	::ShowWindow(mWnd, SW_SHOW);
}

void Window::Impl::destroy()
{
	if(mWnd) {
		::DestroyWindow(mWnd);

		--gWindowCount;
		if(gWindowCount == 0) {
			HMODULE hModule = ::GetModuleHandle(nullptr);
			if(hModule)
				::UnregisterClassW(cWindowClass, hModule);
		}

		mWnd = nullptr;
	}
}

void Window::Impl::onDestroy()
{
	// Back to windowed mode once we receive the WM_DESTROY message
	setFullscreen(false);
}

//////////////////////////////////////////////////////////////////////////
// Begin of setting options

void Window::Impl::setColorBits(const wchar_t* value)
{
	// Cannot change the color bits once the window is created
	if(mWnd)
		return;

	uint colorBits = toInt(value, 32);
	if(colorBits != 4 && colorBits != 8 && colorBits != 16 && colorBits != 32)
		return;

	mColorBits = uint8_t(colorBits);
}

// "curosrPosition = 'x=123; y=456'"
void Window::Impl::setCursorPosition(const wchar_t* value)
{
	int x, y;
	parseXy(value, x, y);
	POINT p = {x, y};
	::ClientToScreen(mWnd, &p);
	::SetCursorPos(p.x, p.y);
}

// "fullscreen = 0|1"
void Window::Impl::setFullscreen(const wchar_t* value)
{
	// Cannot change the fullscreen mode once the window is created
	if(mWnd)
		return;
	mFullScreen = (toInt(value, false) == 1);
}

void Window::Impl::setFullscreen(bool flag)
{
	if(!mFullScreen)
		return;

	if(flag) {
		// Backup the non-fullscreen settings
		::EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &mDisplaySettingBackup);

		DEVMODE devMode;
		ZeroMemory(&devMode, sizeof(devMode));
		devMode.dmSize = sizeof(devMode);
		devMode.dmPelsWidth = mWidth;
		devMode.dmPelsHeight = mHeight;
		devMode.dmBitsPerPel = mColorBits;
		devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

		// Apply fullscreen mode
		if(::ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			mFullScreen = false;
			Log::write(Log::Error, L"Fail to enter full screen mode");
			return;
		}

		// Change window style (no border, no titlebar, ...)
		::SetWindowLong(mWnd, GWL_STYLE, WS_POPUP);
		::SetWindowLong(mWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

		// And resize it so that it fits the entire screen
		::SetWindowPos(mWnd, HWND_TOP, 0, 0, mWidth, mHeight, SWP_FRAMECHANGED);
		::ShowWindow(mWnd, SW_SHOW);

		// SetPixelFormat can fail if window style doesn't contain these flags
		long Style = ::GetWindowLong(mWnd, GWL_STYLE);
		::SetWindowLong(mWnd, GWL_STYLE, Style | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	}
	else {
		// Restore the backup settings
		if(::ChangeDisplaySettings(&mDisplaySettingBackup, CDS_RESET) != DISP_CHANGE_SUCCESSFUL) {
			Log::write(Log::Error, L"Fail to restore orginal screen setting from full screen mode");
		}

		::SetWindowLong(mWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		::SetWindowLong(mWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
		mFullScreen = false;
	}
}

// "height = 600"
void Window::Impl::setHeight(const wchar_t* value)
{
	mHeight = toInt(value, 600);
}

// "show = 0|1"
void Window::Impl::setShowWindow(const wchar_t* value)
{
	::ShowWindow(mWnd, toInt(value, 1) == 1 ? SW_SHOW : SW_HIDE);
}

// "showCursor = 0|1"
void Window::Impl::setShowCursor(const wchar_t* value)
{
	::ShowCursor(toInt(value, 1) == 1);
}

// "title = 'Simple game engine'"
void Window::Impl::setTitle(const wchar_t* value)
{
	mTitle = value;
}

// "width = 800"
void Window::Impl::setWidth(const wchar_t* value)
{
	mWidth = toInt(value, 800);
}

sal_override void Window::Impl::setOption(const wchar_t* name, const wchar_t* value)
{
	struct S {	const wchar_t* name; void (Impl::*fun)(const wchar_t*); };
	static const S table[] = {
		{ L"colorBits",			&Impl::setColorBits },
		{ L"curosrPosition",	&Impl::setCursorPosition },
		{ L"fullscreen",		&Impl::setFullscreen },
		{ L"height",			&Impl::setHeight },
		{ L"show",				&Impl::setShowWindow },
		{ L"showCursor",		&Impl::setShowCursor },
		{ L"title",				&Impl::setTitle },
		{ L"width",				&Impl::setWidth },
	};

	for(size_t i=0; i<sizeof(table)/sizeof(S); ++i) {
		if(::wcscmp(name, table[i].name) == 0) {
			(this->*(table[i].fun))(value);
			return;
		}
	}
}

// End of setting options
//////////////////////////////////////////////////////////////////////////

sal_override void Window::Impl::processEvent(bool blocking)
{
    MSG message;

	if(blocking) {
		if(::GetMessage(&message, mWnd, 0, 0) >= 0) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		} else {
			Log::format(Log::Error, L"Win32 API GetMessage failed with message:\"%s\"", str2WStr(getErrorMessage(nullptr)).c_str());
		}
	}
	else while(::PeekMessage(&message, mWnd, 0, 0, PM_REMOVE)) {
	    TranslateMessage(&message);
	    DispatchMessage(&message);
	}
}

LRESULT CALLBACK Window::Impl::wndProc(sal_in HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Impl* impl = reinterpret_cast<Impl*>(::GetWindowLongPtr(hWnd, GWL_USERDATA));
	if(impl)
		return impl->Proc(uMsg, wParam, lParam);
	else
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT Window::Impl::Proc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Event ev;
	ZeroMemory(&ev, sizeof(ev));

	switch(uMsg)
	{
	case WM_PAINT:
		::ValidateRect(mWnd, nullptr);
		break;

	case WM_DESTROY:
		// Here we must cleanup resources!
		onDestroy();
		break;

	case WM_CLOSE:
	{
		ev.Type = Event::Closed;
		SendEvent(ev);
		break;
	}

	case WM_SIZE:
	{
		// The lParam stores client area size, not window size
		mWidth = LOWORD(lParam);
		mHeight = HIWORD(lParam);

		ev.Type = Event::Resized;
		ev.Size.Width = mWidth;
		ev.Size.Height = mHeight;
		SendEvent(ev);
		break;
	}

	case WM_SETFOCUS:
	{
		ev.Type = Event::GainedFocus;
		SendEvent(ev);
		break;
	}

	case WM_KILLFOCUS:
	{
		ev.Type = Event::LostFocus;
		SendEvent(ev);
		break;
	}

	case WM_CHAR :
	{
		ev.Type = Event::TextEntered;
		ev.Text.Unicode = static_cast<uint16_t>(wParam);
		SendEvent(ev);
		break;
	}

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		if(/*myKeyRepeatEnabled || */((lParam & (1 << 30)) == 0))
		{
			ev.Type = Event::KeyPressed;
			ev.Key.Code = (wParam == VK_SHIFT) ? GetShiftState(true) : ConvertKeyCode(wParam, lParam);
			ev.Key.Alt = HIWORD(::GetAsyncKeyState(VK_MENU)) != 0;
			ev.Key.Control = HIWORD(::GetAsyncKeyState(VK_CONTROL)) != 0;
			ev.Key.Shift = HIWORD(::GetAsyncKeyState(VK_SHIFT)) != 0;
			SendEvent(ev);
		}

		// Use DefWindowProc so that Alt+F4 will handled automatically
		return ::DefWindowProc(mWnd, uMsg, wParam, lParam);
	}

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		ev.Type = Event::KeyReleased;
		ev.Key.Code = (wParam == VK_SHIFT) ? GetShiftState(false) : ConvertKeyCode(wParam, lParam);
		ev.Key.Alt = HIWORD(::GetAsyncKeyState(VK_MENU)) != 0;
		ev.Key.Control = HIWORD(::GetAsyncKeyState(VK_CONTROL)) != 0;
		ev.Key.Shift = HIWORD(::GetAsyncKeyState(VK_SHIFT)) != 0;
		SendEvent(ev);
		break;
	}

	case WM_MOUSEWHEEL :
	{
		ev.Type = Event::MouseWheelMoved;
		ev.MouseWheel.Delta = static_cast<int16_t>(HIWORD(wParam)) / 120;
		SendEvent(ev);
		break;
	}

	case WM_LBUTTONDOWN:
	{
		ev.Type = Event::MouseButtonPressed;
		ev.MouseButton.Button = Mouse::Left;
		SendEvent(ev);
		break;
	}

	case WM_LBUTTONUP:
	{
		ev.Type = Event::MouseButtonReleased;
		ev.MouseButton.Button = Mouse::Left;
		SendEvent(ev);
		break;
	}

	case WM_RBUTTONDOWN:
	{
		ev.Type = Event::MouseButtonPressed;
		ev.MouseButton.Button = Mouse::Right;
		SendEvent(ev);
		break;
	}

	case WM_RBUTTONUP:
	{
		ev.Type = Event::MouseButtonReleased;
		ev.MouseButton.Button = Mouse::Right;
		SendEvent(ev);
		break;
	}

	case WM_MBUTTONDOWN:
	{
		ev.Type = Event::MouseButtonPressed;
		ev.MouseButton.Button = Mouse::Middle;
		SendEvent(ev);
		break;
	}

	case WM_MBUTTONUP:
	{
		ev.Type = Event::MouseButtonReleased;
		ev.MouseButton.Button = Mouse::Middle;
		SendEvent(ev);
		break;
	}

	case WM_XBUTTONDOWN:
	{
		ev.Type = Event::MouseButtonPressed;
		ev.MouseButton.Button = HIWORD(wParam) == XBUTTON1 ? Mouse::XButton1 : Mouse::XButton2;
		SendEvent(ev);
		break;
	}

	case WM_XBUTTONUP:
	{
		ev.Type = Event::MouseButtonReleased;
		ev.MouseButton.Button = HIWORD(wParam) == XBUTTON1 ? Mouse::XButton1 : Mouse::XButton2;
		SendEvent(ev);
		break;
	}

	case WM_MOUSEMOVE:
	{
		ev.Type = Event::MouseMoved;
		ev.MouseMove.X = LOWORD(lParam);
		ev.MouseMove.Y = HIWORD(lParam);
		SendEvent(ev);
		break;
	}

	default:
		return ::DefWindowProc(mWnd, uMsg, wParam, lParam);
	}

	return 0;
}

}	// namespace MCD
