#include "Pch.h"
#include "WinMessageInputComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/System/Window.h"
#include "../../Core/System/WindowEvent.h"
#include "../../Core/System/Utility.h"

namespace MCD {

// Convert the Key::Code into a string
static const char* keyToString(MCD::Key::Code key)
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
		"Left", "Right", "Up", "Down"
	};

	int k = key;
	if(k >= 'a' && k <= 'z')
		k -= 'a';
	else if(k >= '0' && k <= '9')
		k = k - ('0' + 'z' - 'a');
	else
		k = k - MCD::Key::Escape + ('9' - '0' + 'z' - 'a');

	MCD_ASSERT(k < MCD_COUNTOF(mapping));
	return mapping[k];
}

WinMessageInputComponent::WinMessageInputComponent()
{
}

void WinMessageInputComponent::update()
{
	while(true)
	{
		MCD::Event e;
		if(!window || !window->popEvent(e, false))
			return;

		const char* keyName = keyToString(e.Key.Code);

		mKeyDownList.clear();
		mKeyUpList.clear();

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
}

int WinMessageInputComponent::getAxis(sal_in_z const wchar_t* axisName) const
{
	return 0;
}

bool WinMessageInputComponent::anyKey() const
{
	return false;
}

bool WinMessageInputComponent::anyKeyDown() const
{
	return false;
}

bool WinMessageInputComponent::getButton(sal_in_z const wchar_t* buttonName) const
{
	return false;
}

bool WinMessageInputComponent::getButtonDown(sal_in_z const wchar_t* buttonName) const
{
	return false;
}

bool WinMessageInputComponent::getButtonUp(sal_in_z const wchar_t* buttonName) const
{
	return false;
}

}	// namespace MCD
