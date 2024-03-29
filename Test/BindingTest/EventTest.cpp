#include "Pch.h"
#include "../../MCD/Binding/Binding.h"
#include "../../3Party/jkbind/Declarator.h"

using namespace MCD;

namespace {

class Button
{
public:
	Button() : posX(0), posY(0) {}

	// Declaration of the event. All events belongs to the instances of the classes.
	script::Event<void,				// Return type of the event - what c++ expects to get from script.
		script::objNoCare, Button*,	// First policy of the pushing to script, and first argument type.
		script::plain, int,			// Second policy of the pushing to script, and second argument type.
		script::plain, int			// Third policy of the pushing to script, and third argument type.
	> onMousePressed;				// Name of the event.

	void mouse(int x, int y)
	{
		// Checking if the event is assigned from script, calling of non-assigned event is prohibited!
		if(onMousePressed.isAssigned()) {
			// Calling of event is VERY simple - just passing your declared argumens into "Call"
			onMousePressed.call(this, x, y);
		}
	}

	sal_notnull static Button* defaultButton()
	{
		static Button b;
		return &b;
	}

	int posX, posY;
};	// Button

}	// namespace

namespace script {

SCRIPT_CLASS_DECLAR(Button);

SCRIPT_CLASS_REGISTER_NAME(Button, "Button")
	.enableGetset()
	.scriptEvent("onMousePressed", &Button::onMousePressed)
	.staticMethod<objNoCare>("defaultButton", &Button::defaultButton)
	.getset("x", &Button::posX)
	.getset("y", &Button::posY)
;}

}	// namespace script

TEST(EventBindingTest)
{
	ScriptVM vm;
	script::VMCore* v = (script::VMCore*)sq_getforeignptr(HSQUIRRELVM(vm.getImplementationHandle()));
	script::ClassTraits<Button>::bind(v);

	CHECK(vm.runScript("\
		button <- Button.defaultButton();\
		button.onMousePressed().setHandler(\
			function(sender, x, y) {\
				sender.x = x;\
				sender.y = y;\
			}\
		);\
	"));

	Button& b = *Button::defaultButton();
	CHECK_EQUAL(0, b.posX);
	CHECK_EQUAL(0, b.posY);

	b.mouse(10, 20);
	CHECK_EQUAL(10, b.posX);
	CHECK_EQUAL(20, b.posY);

	b.mouse(123, 456);
	CHECK_EQUAL(123, b.posX);
	CHECK_EQUAL(456, b.posY);
}
