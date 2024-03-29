#ifndef __MCD_CORE_ENTITY_INPUTCOMPONENT__
#define __MCD_CORE_ENTITY_INPUTCOMPONENT__

#include "BehaviourComponent.h"
#include "../Math/Vec2.h"

namespace MCD {

/*!	A base class for abstracting an input component.
	Designed base on Unity 3D's input compoent, see
	http://unity3d.com/support/documentation/ScriptReference/Input.html
 */
class MCD_ABSTRACT_CLASS MCD_CORE_API InputComponent : public BehaviourComponent
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(InputComponent);
	}

	/*!	Returns the value of the virtual axis identified by axisName.
		The value will be in the range -1 to 1 for keyboard and joystick input.
		If the axis is setup to be mouse movement, the value will be in pixel.

		\note Always return 0 if the axisName is not found.
	 */
	virtual float getAxis(sal_in_z const char* axisName) const = 0;

	virtual float getAxisRaw(sal_in_z const char* axisName) const = 0;

	virtual float getAxisDelta(sal_in_z const char* axisName) const = 0;

	virtual float getAxisDeltaRaw(sal_in_z const char* axisName) const = 0;

	//! Is any key or mouse button currently held down?
	virtual bool anyKey() const = 0;

	//!	Returns true the first frame the user hits any key or mouse button.
	virtual bool anyKeyDown() const = 0;

	//!	Returns true the first frame the user releases any key or mouse button.
	virtual bool anyKeyUp() const = 0;

	//!	Returns true while the virtual button identified by buttonName is held down.
	virtual bool getButton(sal_in_z const char* buttonName) const = 0;

	//!	Returns true during the frame the user pressed down the virtual button identified by buttonName.
	virtual bool getButtonDown(sal_in_z const char* buttonName) const = 0;

	//!	Returns true the first frame the user releases the virtual button identified by buttonName.
	virtual bool getButtonUp(sal_in_z const char* buttonName) const = 0;

	//! The current mouse position in pixel coordinates.
	virtual Vec2i getMousePosition() const = 0;

	//! Returns whether the the given mouse button is held down.
	virtual bool getMouseButton(int button) const = 0;

	//! Returns true during the frame the user pressed the given mouse button.
	virtual bool getMouseButtonDown(int button) const = 0;

	//! Returns true during the frame the user releases the given mouse button.
	virtual bool getMouseButtonUp(int button) const = 0;

	//! Returns the keyboard input entered this frame.
	virtual sal_out_z const char* inputString() const = 0;

protected:
	InputComponent() {}
};	// InputComponent

typedef IntrusiveWeakPtr<InputComponent> InputComponentPtr;

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_INPUTCOMPONENT__
