#ifndef __MCD_COMPONENT_INPUT_INPUTCOMPONENT__
#define __MCD_COMPONENT_INPUT_INPUTCOMPONENT__

#include "../ShareLib.h"
#include "../../Core/Entity/BehaviourComponent.h"

namespace MCD {

/*!	A base class for abstracting an input component.
 */
class MCD_ABSTRACT_CLASS MCD_COMPONENT_API InputComponent : public BehaviourComponent
{
public:
	/*!	Returns the value of the virtual axis identified by axisName.
		The value will be in the range -1 to 1 for keyboard and joystick input.
		If the axis is setup to be mouse movement, the value will be in pixel.

		\note Always return 0 if the axisName is not found.
	 */
	virtual int getAxis(sal_in_z const wchar_t* axisName) const = 0;

	//! Is any key or mouse button currently held down?
	virtual bool anyKey() const = 0;

	//!	Returns true the first frame the user hits any key or mouse button.
	virtual bool anyKeyDown() const = 0;

	//!	Returns true while the virtual button identified by buttonName is held down.
	virtual bool getButton(sal_in_z const wchar_t* buttonName) const = 0;

	//!	Returns true during the frame the user pressed down the virtual button identified by buttonName.
	virtual bool getButtonDown(sal_in_z const wchar_t* buttonName) const = 0;

	//!	Returns true the first frame the user releases the virtual button identified by buttonName.
	virtual bool getButtonUp(sal_in_z const wchar_t* buttonName) const = 0;

protected:
	InputComponent() {}
};	// InputComponent

}	// namespace MCD

#endif	// __MCD_COMPONENT_INPUT_INPUTCOMPONENT__
