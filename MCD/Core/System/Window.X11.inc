// This inc file is to be included by Window.cpp only

#include "Window.X11.inl"
#include <GL/glx.h>
//#include <X11/extensions/Xrandr.h>

namespace MCD {

Display* gDisplay = nullptr;
int gDisplayCount = 0;
int gScreen = 0;
XIM gInputMethod = nullptr;
Cursor gHiddenCursor;	//!< As X11 doesn't provide cursor hidding, we must create a transparent one

//! Open the display (if not already done)
void openDisplay()
{
	// If no display has been opened yet, open it
	if(gDisplayCount == 0) {
		MCD_ASSERT(gDisplay == nullptr);

		gDisplay = ::XOpenDisplay("localhost:0");

		if(!gDisplay)
			throw std::runtime_error(std::string("Fail to open a connection with the X server"));

//		gScreen = DefaultRootWindow(gDisplay);

		// Disable repetition of KeyRelease events when a key is held down
		Bool Supported;
		::XkbSetDetectableAutoRepeat(gDisplay, True, &Supported);
		::XFlush(gDisplay);

		// Get the input method (XIM) object
		gInputMethod = ::XOpenIM(gDisplay, nullptr, nullptr, nullptr);
	}

	++gDisplayCount;
}

//! Close the display, in pair with openDisplay()
void closeDisplay()
{
	// If all windows have been destroyed, then we can close the display
	if(gDisplay != nullptr && --gDisplayCount == 0) {
		// Close the input method object
		if(gInputMethod) {
			::XCloseIM(gInputMethod);
			gInputMethod = nullptr;
		}

		// Destroy the hidden cursor
		if(gHiddenCursor) {
			::XFreeCursor(gDisplay, gHiddenCursor);
			gHiddenCursor = 0;
		}

		// Close the display
		::XCloseDisplay(gDisplay);
		gDisplay = nullptr;
	}
}

//! Convert a X11 keysym to our key code
Key::Code ConvertKeyCode(KeySym sym)
{
    // First convert to uppercase (to avoid dealing with two different keysyms for the same key)
    KeySym lower, key;
	::XConvertCase(sym, &lower, &key);

	switch(key)
	{
		case XK_Shift_L :		return Key::LShift;
		case XK_Shift_R :		return Key::RShift;
		case XK_Control_L :		return Key::LControl;
		case XK_Control_R :		return Key::RControl;
		case XK_Alt_L :			return Key::LAlt;
		case XK_Alt_R :			return Key::RAlt;
		case XK_Super_L :		return Key::LSystem;
		case XK_Super_R :		return Key::RSystem;
		case XK_Menu :			return Key::Menu;
		case XK_Escape :		return Key::Escape;
		case XK_semicolon :		return Key::SemiColon;
		case XK_slash :			return Key::Slash;
		case XK_equal :			return Key::Equal;
		case XK_minus :			return Key::Dash;
		case XK_bracketleft :	return Key::LBracket;
		case XK_bracketright :	return Key::RBracket;
		case XK_comma :			return Key::Comma;
		case XK_period :		return Key::Period;
		case XK_dead_acute :	return Key::Quote;
		case XK_backslash :		return Key::BackSlash;
		case XK_dead_grave :	return Key::Tilde;
		case XK_space :			return Key::Space;
		case XK_Return :		return Key::Return;
		case XK_KP_Enter :		return Key::Return;
		case XK_BackSpace :		return Key::Back;
		case XK_Tab :			return Key::Tab;
		case XK_Prior :			return Key::PageUp;
		case XK_Next :			return Key::PageDown;
		case XK_End :			return Key::End;
		case XK_Home :			return Key::Home;
		case XK_Insert :		return Key::Insert;
		case XK_Delete :		return Key::Delete;
		case XK_KP_Add :		return Key::Add;
		case XK_KP_Subtract :	return Key::Subtract;
		case XK_KP_Multiply :	return Key::Multiply;
		case XK_KP_Divide :		return Key::Divide;
		case XK_Pause :			return Key::Pause;
		case XK_F1 :			return Key::F1;
		case XK_F2 :			return Key::F2;
		case XK_F3 :			return Key::F3;
		case XK_F4 :			return Key::F4;
		case XK_F5 :			return Key::F5;
		case XK_F6 :			return Key::F6;
		case XK_F7 :			return Key::F7;
		case XK_F8 :			return Key::F8;
		case XK_F9 :			return Key::F9;
		case XK_F10 :			return Key::F10;
		case XK_F11 :			return Key::F11;
		case XK_F12 :			return Key::F12;
		case XK_F13 :			return Key::F13;
		case XK_F14 :			return Key::F14;
		case XK_F15 :			return Key::F15;
		case XK_Left :			return Key::Left;
		case XK_Right :			return Key::Right;
		case XK_Up :			return Key::Up;
		case XK_Down :			return Key::Down;
		case XK_KP_0 :			return Key::Numpad0;
		case XK_KP_1 :			return Key::Numpad1;
		case XK_KP_2 :			return Key::Numpad2;
		case XK_KP_3 :			return Key::Numpad3;
		case XK_KP_4 :			return Key::Numpad4;
		case XK_KP_5 :			return Key::Numpad5;
		case XK_KP_6 :			return Key::Numpad6;
		case XK_KP_7 :			return Key::Numpad7;
		case XK_KP_8 :			return Key::Numpad8;
		case XK_KP_9 :			return Key::Numpad9;
		case XK_A :				return Key::A;
		case XK_Z :				return Key::Z;
		case XK_E :				return Key::E;
		case XK_R :				return Key::R;
		case XK_T :				return Key::T;
		case XK_Y :				return Key::Y;
		case XK_U :				return Key::U;
		case XK_I :				return Key::I;
		case XK_O : 			return Key::O;
		case XK_P : 			return Key::P;
		case XK_Q : 			return Key::Q;
		case XK_S : 			return Key::S;
		case XK_D : 			return Key::D;
		case XK_F : 			return Key::F;
		case XK_G : 			return Key::G;
		case XK_H : 			return Key::H;
		case XK_J :				return Key::J;
		case XK_K :				return Key::K;
		case XK_L :				return Key::L;
		case XK_M :				return Key::M;
		case XK_W :				return Key::W;
		case XK_X :				return Key::X;
		case XK_C :				return Key::C;
		case XK_V : 			return Key::V;
		case XK_B :				return Key::B;
		case XK_N :				return Key::N;
		case XK_0 :				return Key::Num0;
		case XK_1 :				return Key::Num1;
		case XK_2 :				return Key::Num2;
		case XK_3 :				return Key::Num3;
		case XK_4 :				return Key::Num4;
		case XK_5 :				return Key::Num5;
		case XK_6 :				return Key::Num6;
		case XK_7 :				return Key::Num7;
		case XK_8 :				return Key::Num8;
		case XK_9 :				return Key::Num9;
	}

	return Key::Code(0);
}

Window::Impl::Impl(Window& w)
	:
	mWindow(w),
	mWnd(0),
	mAtomClose(0),
	mInputContext(nullptr)
{
	mListeners.insert(&w);
}

void Window::Impl::createNewWindow()
{
	MCD_ASSERT(!mWnd && "A window is already created, call destroy() first");

	// Open the display at first call
	openDisplay();

	int screen = DefaultScreen(gDisplay);
	Visual* visual = DefaultVisual(gDisplay, screen);
	int depth = DefaultDepth(gDisplay, screen);

	XSetWindowAttributes winAttribs;
	winAttribs.event_mask =
		ExposureMask | VisibilityChangeMask |
		KeyPressMask | PointerMotionMask |
		StructureNotifyMask ;

	winAttribs.border_pixel = 0;
	winAttribs.bit_gravity = StaticGravity;
	winAttribs.colormap = ::XCreateColormap(gDisplay, RootWindow(gDisplay, screen), visual, AllocNone);
	unsigned long winmask = CWBorderPixel | CWBitGravity | CWEventMask| CWColormap;

	mWnd = ::XCreateWindow(
		gDisplay, DefaultRootWindow(gDisplay), 20, 20,
		mWidth, mHeight, 0,
		depth, InputOutput,
		visual, winmask, &winAttribs
	);

	if(!mWnd)
		throw std::runtime_error("X11 API call 'XCreateWindow' failed");

	// Set the window title
	::XStoreName(gDisplay, mWnd, wStr2Str(mTitle).c_str());

	::XMapWindow(gDisplay, mWnd);

	// Get the atom defining the close event
	mAtomClose = ::XInternAtom(gDisplay, "WM_DELETE_WINDOW", false);
	::XSetWMProtocols(gDisplay, mWnd, &mAtomClose, 1);

	// Create the input context
	if(gInputMethod) {
		mInputContext = ::XCreateIC(gInputMethod,
			XNClientWindow,	mWnd,
			XNFocusWindow,	mWnd,
			XNInputStyle,	XIMPreeditNothing | XIMStatusNothing,
			//XNResourceName,  "MCD",
			//XNResourceClass, "MCD",
			nullptr
		);

		if(!mInputContext) {
			// TODO: Log error message
			//std::cerr << "Failed to create input context for window -- TextEntered event won't be able to return unicode" << std::endl;
		}
	}

	// Create the hidden cursor
	createHiddenCursor();
}

void Window::Impl::destroy()
{
	// Destroy the input context
	if(mInputContext) {
		::XDestroyIC(mInputContext);
		mInputContext = nullptr;
	}

	// Destroy the window
	if(mWnd/* && !myIsExternal*/) {
		::XDestroyWindow(gDisplay, mWnd);
		::XFlush(gDisplay);
		mWnd = 0;
	}

	closeDisplay();
}

//////////////////////////////////////////////////////////////////////////
// Begin of setting options

// "curosrPosition = 'x=123; y=456'"
void Window::Impl::setCursorPosition(const wchar_t* value)
{
	int x, y;
	parseXy(value, x, y);
	::XWarpPointer(gDisplay, None, mWnd, 0, 0, 0, 0, x, y);
	::XFlush(gDisplay);
}

// "height = 600"
void Window::Impl::setHeight(const wchar_t* value)
{
	mHeight = wStr2IntWithDefault(value, 600);
}

// "show = 0|1"
void Window::Impl::setShowWindow(const wchar_t* value)
{
	if(wStr2IntWithDefault(value, 1) == 1)
		::XMapWindow(gDisplay, mWnd);
	else
		::XUnmapWindow(gDisplay, mWnd);

	::XFlush(gDisplay);
}

// "showCursor = 0|1"
void Window::Impl::setShowCursor(const wchar_t* value)
{
	::XDefineCursor(gDisplay, mWnd, wStr2IntWithDefault(value, 1) == 1 ? None : gHiddenCursor);
	::XFlush(gDisplay);
}

// "title = 'Simple game engine'"
void Window::Impl::setTitle(const wchar_t* value)
{
	mTitle = value;
}

// "width = 800"
void Window::Impl::setWidth(const wchar_t* value)
{
	mWidth = wStr2IntWithDefault(value, 800);
}

sal_override void Window::Impl::setOption(const wchar_t* name, const wchar_t* value)
{
	struct S {	const wchar_t* name; void (Impl::*fun)(const wchar_t*); };
	static const S table[] = {
		{ L"curosrPosition",	&Impl::setCursorPosition },
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

void Window::Impl::processEvent(bool blocking)
{
	// Filter the received events (only allow those matching a specific window)
	struct Dummy {
		static Bool checkEvent(::Display*, XEvent* event, XPointer userData) {
			return event->xany.window == reinterpret_cast< ::Window>(userData);
		}
	};	// Dummy

	{	// Process any event in the queue matching our window
		XEvent event;

		MCD_ASSERT(mWnd);
		if(blocking) {
			::XIfEvent(gDisplay, &event, &Dummy::checkEvent, reinterpret_cast<XPointer>(mWnd));
			processEvent(event);
		}
		else {
			while(::XCheckIfEvent(gDisplay, &event, &Dummy::checkEvent, reinterpret_cast<XPointer>(mWnd)))
				processEvent(event);
		}
	}
}

void Window::Impl::processEvent(XEvent xEvent)
{
	Event ev;
//	ZeroMemory(&ev, sizeof(ev));

	switch(xEvent.type)
	{
    case DestroyNotify:
    {
        // The window is about to be destroyed : we must cleanup resources
		onDestroy();
        break;
    }

	case FocusIn:
	{
		// Update the input context
		if(mInputContext)
			::XSetICFocus(mInputContext);

		ev.Type = Event::GainedFocus;
		SendEvent(ev);
		break;
	}

	case FocusOut:
	{
		// Update the input context
		if(mInputContext)
			::XUnsetICFocus(mInputContext);

		ev.Type = Event::LostFocus;
		SendEvent(ev);
		break;
	}

	case ConfigureNotify:
	{
		if((xEvent.xconfigure.width != static_cast<int>(mWidth)) || (xEvent.xconfigure.height != static_cast<int>(mHeight)))
		{
			mWidth = xEvent.xconfigure.width;
			mHeight = xEvent.xconfigure.height;

			ev.Type = Event::Resized;
			ev.Size.Width = mWidth;
			ev.Size.Height = mHeight;
			SendEvent(ev);
		}
		break;
	}

	case ClientMessage:
	{
		if((xEvent.xclient.format == 32) && (xEvent.xclient.data.l[0]) == static_cast<long>(mAtomClose))
		{
			ev.Type = Event::Closed;
			SendEvent(ev);
		}
		break;
	}

	case ButtonPress:
	{
		uint Button = xEvent.xbutton.button;
		if((Button == Button1) || (Button == Button2) || (Button == Button3) || (Button == 8) || (Button == 9))
		{
			ev.Type = Event::MouseButtonPressed;
			switch(Button)
			{
				case Button1 :	ev.MouseButton.Button = Mouse::Left;		break;
				case Button2 :	ev.MouseButton.Button = Mouse::Middle;		break;
				case Button3 :	ev.MouseButton.Button = Mouse::Right;		break;
				case 8 :		ev.MouseButton.Button = Mouse::XButton1;	break;
				case 9 :		ev.MouseButton.Button = Mouse::XButton2;	break;
			}
			SendEvent(ev);
		}
		break;
	}

	case ButtonRelease:
	{
		uint Button = xEvent.xbutton.button;
		if((Button == Button1) || (Button == Button2) || (Button == Button3) || (Button == 8) || (Button == 9))
		{
			Event ev;
			ev.Type = Event::MouseButtonReleased;
			switch(Button)
			{
				case Button1 :	ev.MouseButton.Button = Mouse::Left;		break;
				case Button2 :	ev.MouseButton.Button = Mouse::Middle;		break;
				case Button3 :	ev.MouseButton.Button = Mouse::Right;		break;
				case 8 :		ev.MouseButton.Button = Mouse::XButton1;	break;
				case 9 :		ev.MouseButton.Button = Mouse::XButton2;	break;
			}
			SendEvent(ev);
		}
		else if((Button == Button4) || (Button == Button5))
		{
			ev.Type = Event::MouseWheelMoved;
			ev.MouseWheel.Delta = xEvent.xbutton.button == Button4 ? 1 : -1;
			SendEvent(ev);
		}
		break;
	}

	case MotionNotify:
	{
		ev.Type = Event::MouseMoved;
		ev.MouseMove.X = xEvent.xmotion.x;
		ev.MouseMove.Y = xEvent.xmotion.y;
		SendEvent(ev);
		break;
	}

	default:
		// TODO: Something missed.
		break;
	}
}

//! Create a transparent mouse cursor (tricks to hide the cursor)
void Window::Impl::createHiddenCursor()
{
	if(gHiddenCursor)
		return;

	// Create the cursor's pixmap (1x1 pixels)
	Pixmap cursorPixmap = ::XCreatePixmap(gDisplay, mWnd, 1, 1, 1);
	GC graphicsContext = ::XCreateGC(gDisplay, cursorPixmap, 0, NULL);
	::XDrawPoint(gDisplay, cursorPixmap, graphicsContext, 0, 0);
	::XFreeGC(gDisplay, graphicsContext);

	// Create the cursor, using the pixmap as both the shape and the mask of the cursor
	XColor color;
	color.flags = DoRed | DoGreen | DoBlue;
	color.red = color.blue = color.green = 0;
	gHiddenCursor = ::XCreatePixmapCursor(gDisplay, cursorPixmap, cursorPixmap, &color, &color, 0, 0);

	// We don't need the pixmap any longer, free it
	::XFreePixmap(gDisplay, cursorPixmap);
}

}	// namespace MCD
