#ifndef __MCD_CORE_ENTITY_WINMESSAGEINPUTCOMPONENT__
#define __MCD_CORE_ENTITY_WINMESSAGEINPUTCOMPONENT__

#include "InputComponent.h"
#include "../System/Window.h"
#include "../System/WindowEvent.h"
#include "../Math/Vec3.h"
#include <map>
#include <queue>

namespace MCD {

class Window;

/*!	An input component that use MCD::Window as the message source.
 */
class MCD_CORE_API WinMessageInputComponent : public InputComponent, public IWindowListener
{
public:
	WinMessageInputComponent();

	sal_override ~WinMessageInputComponent();

// Operations
	sal_override void update(float dt);

	sal_override float getAxis(sal_in_z const char* axisName) const;

	sal_override float getAxisRaw(sal_in_z const char* axisName) const;

	sal_override float getAxisDelta(sal_in_z const char* axisName) const;

	sal_override float getAxisDeltaRaw(sal_in_z const char* axisName) const;

	sal_override bool anyKey() const;

	sal_override bool anyKeyDown() const;

	sal_override bool getButton(sal_in_z const char* buttonName) const;

	sal_override bool getButtonDown(sal_in_z const char* buttonName) const;

	sal_override bool getButtonUp(sal_in_z const char* buttonName) const;

	sal_override Vec2i getMousePosition() const;

	sal_override bool getMouseButton(int button) const;

	sal_override bool getMouseButtonDown(int button) const;

	sal_override bool getMouseButtonUp(int button) const;

	sal_override sal_out_z const char* inputString() const;

	/*!	Attach the WinMessageInputComponent to an existing Window.
		\note Make sure the window object keep alive during the life time of this.
	 */
	void attachTo(Window& window);

	sal_maybenull Window* getWindow();

protected:
	sal_override void onEvent(const Event& eventReceived);

	void popEvent();

	struct Compare {
		bool operator()(sal_in_z const char* lhs, sal_in_z const char* rhs) const;
	};	// Compare

	Window* mWindow;
	std::deque<Event> mStackUpEvent;	///< Prevents multiple up/down event to be reported in the same frame
	typedef std::map<const char*, int, Compare> EventList;
	EventList mAxisList;
	EventList mKeyList;
	EventList mKeyDownList;
	EventList mKeyUpList;
	Vec2i mMousePosition;
	int8_t mMouseKeyBitArray;	//!< 8 bit can store 8 mouse key
	int8_t mMouseKeyDownBitArray;
	int8_t mMouseKeyUpBitArray;
	std::string mInputString;
	Vec3f mMouseAxis, mMouseAxisRaw;	//!< Mouse x, y and wheel as z
	Vec3f mPreviousMouseAxis, mPreviousMouseAxisRaw;	//!< For calculating delta
};	// WinMessageInputComponent

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_WINMESSAGEINPUTCOMPONENT__
