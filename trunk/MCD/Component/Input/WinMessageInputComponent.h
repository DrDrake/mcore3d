#ifndef __MCD_COMPONENT_INPUT_WINMESSAGEINPUTCOMPONENT__
#define __MCD_COMPONENT_INPUT_WINMESSAGEINPUTCOMPONENT__

#include "InputComponent.h"
#include "../../Core/System/Window.h"
#include <map>

namespace MCD {

class Window;

/*!	An input component that use MCD::Window as the message source.
 */
class MCD_COMPONENT_API WinMessageInputComponent : public InputComponent, public IWindowListener
{
public:
	WinMessageInputComponent();

	sal_override ~WinMessageInputComponent();

// Cloning
	sal_override bool cloneable() const { return false; }

	sal_override Component* clone() const { return nullptr; }

// Operations
	sal_override void update();

	sal_override int getAxis(sal_in_z const wchar_t* axisName) const;

	sal_override bool anyKey() const;

	sal_override bool anyKeyDown() const;

	sal_override bool getButton(sal_in_z const wchar_t* buttonName) const;

	sal_override bool getButtonDown(sal_in_z const wchar_t* buttonName) const;

	sal_override bool getButtonUp(sal_in_z const wchar_t* buttonName) const;

	/*!	Attach the WinMessageInputComponent to an existing Window.
		\note Make sure the window object keep alive during the life time of this.
	 */
	void attachTo(Window& window);

	sal_maybenull Window* getWindow();

protected:
	sal_override void onEvent(const Event& eventReceived);

	struct Compare {
		bool operator()(sal_in_z const wchar_t* lhs, sal_in_z const wchar_t* rhs) const;
	};	// Compare

	typedef std::map<const wchar_t*, int, Compare> EventList;
	EventList mAxisList;
	EventList mKeyList;
	EventList mKeyDownList;
	EventList mKeyUpList;
	Window* mWindow;
};	// WinMessageInputComponent

}	// namespace MCD

#endif	// __MCD_COMPONENT_INPUT_WINMESSAGEINPUTCOMPONENT__
