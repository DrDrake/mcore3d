#ifndef __MCD_COMPONENT_INPUT_WINMESSAGEINPUTCOMPONENT__
#define __MCD_COMPONENT_INPUT_WINMESSAGEINPUTCOMPONENT__

#include "InputComponent.h"
#include <map>

namespace MCD {

class Window;

/*!	A base class for abstracting an input component.
 */
class MCD_COMPONENT_API WinMessageInputComponent : public InputComponent
{
public:
	WinMessageInputComponent();

	sal_override void update();

	sal_override int getAxis(sal_in_z const wchar_t* axisName) const;

	sal_override bool anyKey() const;

	sal_override bool anyKeyDown() const;

	sal_override bool getButton(sal_in_z const wchar_t* buttonName) const;

	sal_override bool getButtonDown(sal_in_z const wchar_t* buttonName) const;

	sal_override bool getButtonUp(sal_in_z const wchar_t* buttonName) const;

	MCD::Window* window;

protected:
	typedef std::map<const char*, int> EventList;
	EventList mAxisList;
	EventList mKeyList;
	EventList mKeyDownList;
	EventList mKeyUpList;
};	// WinMessageInputComponent

}	// namespace MCD

#endif	// __MCD_COMPONENT_INPUT_WINMESSAGEINPUTCOMPONENT__
