#include "Pch.h"
#include "WinMessageInputComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/System/Window.h"
#include "../../Core/System/WindowEvent.h"
#include "../../Core/System/Utility.h"

namespace MCD {

// Convert the Key::Code into a string
static sal_maybenull const wchar_t* keyToString(MCD::Key::Code key)
{
	// NOTE: The following code need to be in syn with the definition of
	// MCD::Key in Core/System/WindowEvent.h
	static const wchar_t* mapping[] = {
		L"a", L"b", L"c", L"d", L"e", L"f", L"g", L"h", L"i", L"j", L"k", L"l", L"m",
		L"n", L"o", L"p", L"q", L"r", L"s", L"t", L"u", L"v", L"w", L"x", L"y", L"z",
		L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9",
		L"Escape",
		L"LControl", L"LShift", L"LAlt", L"LSystem",
		L"RControl", L"RShift", L"RAlt", L"RSystem",
		L"Menu", L"LBracket", L"RBracket",
		L"SemiColon", L"Comma", L"Period",
		L"Quote", L"Slash", L"BackSlash",
		L"Tilde", L"Equal", L"Dash",
		L"Space", L"Return", L"Back",
		L"Tab", L"PageUp", L"PageDown",
		L"End", L"Home", L"Insert", L"Delete",
		L"Add", L"Subtract", L"Multiply", L"Divide",
		L"Left", L"Right", L"Up", L"Down",
		L"Numpad0", L"Numpad1", L"Numpad2", L"Numpad3", L"Numpad4",
		L"Numpad5", L"Numpad6", L"Numpad7", L"Numpad8", L"Numpad9",
		L"F1", L"F2", L"F3", L"F4", L"F5", L"F6", L"F7", L"F8",
		L"F9", L"F10", L"F11", L"F12", L"F13", L"F14", L"F15",
		L"Pause",
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

bool WinMessageInputComponent::Compare::operator()(const wchar_t* lhs, const wchar_t* rhs) const {
	return wcscmp(lhs, rhs) < 0;
}

WinMessageInputComponent::WinMessageInputComponent()
	: window(nullptr)
{
}

WinMessageInputComponent::~WinMessageInputComponent()
{
	if(window)
		window->removeListener(*this);
}

void WinMessageInputComponent::update()
{
	mKeyDownList.clear();
	mKeyUpList.clear();
}

int WinMessageInputComponent::getAxis(sal_in_z const wchar_t* axisName) const
{
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

bool WinMessageInputComponent::getButton(const wchar_t* buttonName) const
{
	EventList::const_iterator i = mKeyList.find(buttonName);
	if(i == mKeyList.end())
		return false;

	return i->second == 1;
}

bool WinMessageInputComponent::getButtonDown(const wchar_t* buttonName) const
{
	return mKeyDownList.find(buttonName) != mKeyDownList.end();
}

bool WinMessageInputComponent::getButtonUp(const wchar_t* buttonName) const
{
	return mKeyUpList.find(buttonName) != mKeyUpList.end();
}

void WinMessageInputComponent::onEvent(const Event& e)
{
	const wchar_t* keyName = keyToString(e.Key.Code);

	if(e.Type == Event::KeyPressed && !keyName) {
		MCD_ASSERT(false && "Unknow key code");
		return;
	}

	switch(e.Type)
	{
	case Event::KeyPressed:
		mKeyList[keyName] = 1;
		mKeyDownList[keyName] = 1;
		break;
	case Event::KeyReleased:
		mKeyList[keyName] = 0;
		mKeyUpList[keyName] = 1;
		break;
	default:
		break;
	}
}

}	// namespace MCD
