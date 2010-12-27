#include "Pch.h"
#include "WinMessageInputComponent.h"
#include "../Entity/Entity.h"
#include "../System/StrUtility.h"
#include "../System/Utility.h"
#include "../System/Window.h"
#include "../System/WindowEvent.h"

namespace MCD {

// Convert the Key::Code into a string
static sal_maybenull const char* keyToString(MCD::Key::Code key)
{
	// NOTE: The following code need to be in syn with the definition of
	// MCD::Key in Core/System/WindowEvent.h
	static const char* mapping[] = {
		"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
		"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
		"Escape",
		"LControl", "LShift", "LAlt", "LSystem",
		"RControl", "RShift", "RAlt", "RSystem",
		"Menu", "LBracket", "RBracket",
		"SemiColon", "Comma", "Period",
		"Quote", "Slash", "BackSlash",
		"Tilde", "Equal", "Dash",
		"Space", "Return", "Back",
		"Tab", "PageUp", "PageDown",
		"End", "Home", "Insert", "Delete",
		"Add", "Subtract", "Multiply", "Divide",
		"Left", "Right", "Up", "Down",
		"Numpad0", "Numpad1", "Numpad2", "Numpad3", "Numpad4",
		"Numpad5", "Numpad6", "Numpad7", "Numpad8", "Numpad9",
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
		"F9", "F10", "F11", "F12", "F13", "F14", "F15",
		"Pause",
	};

	int k = key;
	if(k >= 'a' && k <= 'z')
		k -= 'a';
	else if(k >= '0' && k <= '9')
		k = (k - '0') + ('z' - 'a') + 1;
	else
		k = k - MCD::Key::Escape + ('9' - '0' + 1 + 'z' - 'a' + 1);

	if(k <MCD_COUNTOF(mapping))
		return mapping[k];
	else
		return nullptr;
}

bool WinMessageInputComponent::Compare::operator()(const char* lhs, const char* rhs) const {
	return strcmp(lhs, rhs) < 0;
}

WinMessageInputComponent::WinMessageInputComponent()
	: mWindow(nullptr), mMousePosition(0),
	  mMouseKeyBitArray(0), mMouseKeyDownBitArray(0), mMouseKeyUpBitArray(0),
	  mMouseAxis(0), mMouseAxisRaw(0),
	  mPreviousMouseAxis(0), mPreviousMouseAxisRaw(0)
{
}

WinMessageInputComponent::~WinMessageInputComponent()
{
	if(mWindow)
		mWindow->removeListener(*this);
}

void WinMessageInputComponent::update(float)
{
	mKeyDownList.clear();
	mKeyUpList.clear();
	mMouseKeyDownBitArray = 0;
	mMouseKeyUpBitArray = 0;
	mInputString.clear();

	mPreviousMouseAxis = mMouseAxis;
	mPreviousMouseAxisRaw = mMouseAxisRaw;

	// Perform axis smoothing
	// TODO: Make the smoothing framerate independent
	mMouseAxis = mMouseAxisRaw * 0.5f + mMouseAxis * 0.5f;

	popEvent();
}

float WinMessageInputComponent::getAxis(sal_in_z const char* axisName) const
{
	if(strCaseCmp(axisName, "mouse x") == 0)
		return mMouseAxis.x;
	if(strCaseCmp(axisName, "mouse y") == 0)
		return mMouseAxis.y;
	if(strCaseCmp(axisName, "mouse z") == 0)
		return mMouseAxis.z;
	return 0;
}

float WinMessageInputComponent::getAxisRaw(sal_in_z const char* axisName) const
{
	if(strCaseCmp(axisName, "mouse x") == 0)
		return mMouseAxisRaw.x;
	if(strCaseCmp(axisName, "mouse y") == 0)
		return mMouseAxisRaw.y;
	if(strCaseCmp(axisName, "mouse z") == 0)
		return mMouseAxisRaw.z;
	return 0;
}

float WinMessageInputComponent::getAxisDelta(sal_in_z const char* axisName) const
{
	if(strCaseCmp(axisName, "mouse x") == 0)
		return mMouseAxis.x - mPreviousMouseAxis.x;
	if(strCaseCmp(axisName, "mouse y") == 0)
		return mMouseAxis.y - mPreviousMouseAxis.y;
	if(strCaseCmp(axisName, "mouse z") == 0)
		return mMouseAxis.z - mPreviousMouseAxis.z;
	return 0;
}

float WinMessageInputComponent::getAxisDeltaRaw(sal_in_z const char* axisName) const
{
	if(strCaseCmp(axisName, "mouse x") == 0)
		return mMouseAxisRaw.x - mPreviousMouseAxisRaw.x;
	if(strCaseCmp(axisName, "mouse y") == 0)
		return mMouseAxisRaw.y - mPreviousMouseAxisRaw.y;
	if(strCaseCmp(axisName, "mouse z") == 0)
		return mMouseAxisRaw.z - mPreviousMouseAxisRaw.z;
	return 0;
}

bool WinMessageInputComponent::anyKey() const
{
	return !mKeyList.empty();
}

bool WinMessageInputComponent::anyKeyDown() const
{
	return !mKeyDownList.empty();
}

bool WinMessageInputComponent::anyKeyUp() const
{
	return !mKeyUpList.empty();
}

bool WinMessageInputComponent::getButton(const char* buttonName) const
{
	EventList::const_iterator i = mKeyList.find(buttonName);
	if(i == mKeyList.end())
		return false;

	return i->second == 1;
}

bool WinMessageInputComponent::getButtonDown(const char* buttonName) const
{
	return mKeyDownList.find(buttonName) != mKeyDownList.end();
}

bool WinMessageInputComponent::getButtonUp(const char* buttonName) const
{
	return mKeyUpList.find(buttonName) != mKeyUpList.end();
}

Vec2i WinMessageInputComponent::getMousePosition() const
{
	return mMousePosition;
}

bool WinMessageInputComponent::getMouseButton(int button) const
{
	if(button >= MCD::Mouse::Count)
		return false;
	return (mMouseKeyBitArray & (1 << button)) > 0;
}

bool WinMessageInputComponent::getMouseButtonDown(int button) const
{
	if(button >= MCD::Mouse::Count)
		return false;
	return (mMouseKeyDownBitArray & (1 << button)) > 0;
}

bool WinMessageInputComponent::getMouseButtonUp(int button) const
{
	if(button >= MCD::Mouse::Count)
		return false;
	return (mMouseKeyUpBitArray & (1 << button)) > 0;
}

const char* WinMessageInputComponent::inputString() const
{
	return mInputString.c_str();
}

void WinMessageInputComponent::attachTo(Window& window)
{
	window.addListener(*this);
	mWindow = &window;
}

Window* WinMessageInputComponent::getWindow()
{
	return mWindow;
}

void WinMessageInputComponent::onEvent(const Event& e)
{
	mStackUpEvent.push_back(e);
}

void WinMessageInputComponent::popEvent()
{
	// This function make sure a button down OR button release
	// event will be seen by the game play programmer.

	bool mouseKeyCaptured = false;
	bool keyboardKeyCaptured = false;

	while(!mStackUpEvent.empty())
	{
		Event e = mStackUpEvent.front();

		const char* keyName = keyToString(e.Key.Code);

		if(e.Type == Event::KeyPressed && !keyName) {
//			MCD_ASSERT(false && "Unknow key code");
			return;
		}

		switch(e.Type)
		{
		case Event::KeyPressed:
			if(keyboardKeyCaptured) return;
			keyboardKeyCaptured = true;
			if(keyName) {
				mKeyList[keyName] = 1;
				mKeyDownList[keyName] = 1;
			}
			break;
		case Event::KeyReleased:
			if(keyboardKeyCaptured) return;
			keyboardKeyCaptured = true;
			if(keyName) {
				mKeyList.erase(keyName);
				mKeyUpList[keyName] = 1;
			}
			break;
		case Event::MouseMoved:
			mPreviousMouseAxisRaw.x = mMouseAxisRaw.x;
			mPreviousMouseAxisRaw.y = mMouseAxisRaw.y;
			{	Vec2i newPos(e.MouseMove.X, e.MouseMove.Y);
				mMouseAxisRaw.x += (newPos.x - mMousePosition.x);
				mMouseAxisRaw.y += (newPos.y - mMousePosition.y);
				mMousePosition = newPos;
			} break;
		case Event::MouseWheelMoved:
			mPreviousMouseAxisRaw.z = mMouseAxisRaw.z;
			mMouseAxisRaw.z += e.MouseWheel.Delta;
			break;
		case Event::MouseButtonPressed:
			if(mouseKeyCaptured) return;
			mouseKeyCaptured = true;
			mMouseKeyBitArray |= (1 << int(e.MouseButton.Button));
			mMouseKeyDownBitArray |= (1 << int(e.MouseButton.Button));
			break;
		case Event::MouseButtonReleased:
			if(mouseKeyCaptured) return;
			mouseKeyCaptured = true;
			mMouseKeyBitArray &= ~(1 << int(e.MouseButton.Button));
			mMouseKeyUpBitArray |= (1 << int(e.MouseButton.Button));
			break;
		case Event::TextEntered:
			{	std::string utf8Str;
				MCD_VERIFY(wStrToUtf8((wchar_t*)(&e.Text.Unicode), 1, utf8Str));
				mInputString += utf8Str;
			} break;
		default:
			break;
		}

		mStackUpEvent.pop_front();
	}
}

}	// namespace MCD
