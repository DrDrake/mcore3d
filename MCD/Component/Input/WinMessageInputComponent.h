#ifndef __MCD_COMPONENT_INPUT_WINMESSAGEINPUTCOMPONENT__
#define __MCD_COMPONENT_INPUT_WINMESSAGEINPUTCOMPONENT__

#include "InputComponent.h"
#include "../../Core/System/Window.h"
#include "../../Core/Math/Vec3.h"
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

// Operations
	sal_override void update(float dt);

	sal_override float getAxis(sal_in_z const wchar_t* axisName) const;

	sal_override float getAxisRaw(sal_in_z const wchar_t* axisName) const;

	sal_override float getAxisDelta(sal_in_z const wchar_t* axisName) const;

	sal_override float getAxisDeltaRaw(sal_in_z const wchar_t* axisName) const;

	sal_override bool anyKey() const;

	sal_override bool anyKeyDown() const;

	sal_override bool getButton(sal_in_z const wchar_t* buttonName) const;

	sal_override bool getButtonDown(sal_in_z const wchar_t* buttonName) const;

	sal_override bool getButtonUp(sal_in_z const wchar_t* buttonName) const;

	sal_override Vec2i getMousePosition() const;

	sal_override bool getMouseButton(int button) const;

	sal_override bool getMouseButtonDown(int button) const;

	sal_override bool getMouseButtonUp(int button) const;

	sal_override sal_out_z const wchar_t* inputString() const;

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

	Window* mWindow;
	typedef std::map<const wchar_t*, int, Compare> EventList;
	EventList mAxisList;
	EventList mKeyList;
	EventList mKeyDownList;
	EventList mKeyUpList;
	Vec2i mMousePosition;
	int8_t mMouseKeyBitArray;	//!< 8 bit can store 8 mouse key
	int8_t mMouseKeyDownBitArray;
	int8_t mMouseKeyUpBitArray;
	std::wstring mInputString;
	Vec3f mMouseAxis, mMouseAxisRaw;	//!< Mouse x, y and wheel as z
	Vec3f mPreviousMouseAxis, mPreviousMouseAxisRaw;	//!< For calculating delta
};	// WinMessageInputComponent

}	// namespace MCD

#endif	// __MCD_COMPONENT_INPUT_WINMESSAGEINPUTCOMPONENT__
