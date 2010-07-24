#ifndef __MCD_CORE_SYSTEM_WINDOWEVENT__
#define __MCD_CORE_SYSTEM_WINDOWEVENT__

#include "../ShareLib.h"
#include "Array.h"

namespace MCD {

struct Key
{
	enum Code {
		A = 'a',
		B = 'b',
		C = 'c',
		D = 'd',
		E = 'e',
		F = 'f',
		G = 'g',
		H = 'h',
		I = 'i',
		J = 'j',
		K = 'k',
		L = 'l',
		M = 'm',
		N = 'n',
		O = 'o',
		P = 'p',
		Q = 'q',
		R = 'r',
		S = 's',
		T = 't',
		U = 'u',
		V = 'v',
		W = 'w',
		X = 'x',
		Y = 'y',
		Z = 'z',
		Num0 = '0',
		Num1 = '1',
		Num2 = '2',
		Num3 = '3',
		Num4 = '4',
		Num5 = '5',
		Num6 = '6',
		Num7 = '7',
		Num8 = '8',
		Num9 = '9',
		Escape = 256,
		LControl,
		LShift,
		LAlt,
		LSystem,		//!< OS specific key (left side) : windows (Win and Linux), apple (MacOS), ...
		RControl,
		RShift,
		RAlt,
		RSystem,		//!< OS specific key (right side) : windows (Win and Linux), apple (MacOS), ...
		Menu,
		LBracket,		//!< [
		RBracket,		//!< ]
		SemiColon,		//!< ;
		Comma,			//!< ,
		Period,			//!< .
		Quote,			//!< '
		Slash,			//!< /
		BackSlash,
		Tilde,			//!< ~
		Equal,			//!< =
		Dash,			//!< -
		Space,
		Return,
		Back,
		Tab,
		PageUp,
		PageDown,
		End,
		Home,
		Insert,
		Delete,
		Add,			//!< +
		Subtract,		//1< -
		Multiply,		//!< *
		Divide,			//!< /
		Left,			//!< Left arrow
		Right,			//!< Right arrow
		Up,				//!< Up arrow
		Down,			//!< Down arrow
		Numpad0,
		Numpad1,
		Numpad2,
		Numpad3,
		Numpad4,
		Numpad5,
		Numpad6,
		Numpad7,
		Numpad8,
		Numpad9,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		Pause,

		Count	// For internal use
	};	// enum Code
};	// Key

struct Mouse
{
	enum Button
	{
		Left = 0,
		Right,
		Middle,
		Button1,
		Button2,
		XButton1,
		XButton2,

		Count // For internal use
	};	// enum Button
};	// Mouse

class Event
{
public:
	struct KeyEvent
	{
		Key::Code Code;
		bool Alt;
		bool Control;
		bool Shift;
	};

	struct TextEvent
	{
		uint16_t Unicode;
	};

	struct MouseMoveEvent
	{
		uint MouseId;
		uint X;
		uint Y;
	};

	struct MouseButtonEvent
	{
		uint MouseId;	//!< Identify the mouse in multi-pointing device (eg. iPhone), value start with 0.
		Mouse::Button Button;
		uint X;
		uint Y;
	};

	struct MouseWheelEvent
	{
		uint MouseId;
		int Delta;
	};

	struct SizeEvent
	{
		uint Width;
		uint Height;
	};

	/*!	Other event that were not specified in this header can be encoded in this
		fixed buffer user data.
	 */
	struct OtherEvent
	{
		Array<char,16> UserData;
	};

	enum EventType
	{
		None,	//!< Means nothing happen
		Closed,
		Resized,
		LostFocus,
		GainedFocus,
		TextEntered,
		KeyPressed,
		KeyReleased,
		MouseWheelMoved,
		MouseButtonPressed,
		MouseButtonReleased,
		MouseMoved,
		OtherEventType
	} Type;

	union
	{
		KeyEvent			Key;
		TextEvent			Text;
		MouseMoveEvent		MouseMove;
		MouseButtonEvent	MouseButton;
		MouseWheelEvent		MouseWheel;
		SizeEvent			Size;
		OtherEvent			Other;
	};
};	// Event

}	// namespace MCD

#endif // __MCD_CORE_SYSTEM_WINDOWEVENT__
