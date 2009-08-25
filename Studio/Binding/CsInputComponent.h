#pragma once

#include "../../MCD/Component/Input/InputComponent.h"
#undef nullptr
#include <gcroot.h>
#include <map>

namespace Binding {

/*!	An input component that use a C# Gui control as the message source.
 */
class CsInputComponent : public MCD::InputComponent
{
public:
	ref class MessageRouter
	{
	public:
		MessageRouter() {}

		void onKeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
		void onKeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
		void onMouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);
		void onMouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);
		void onMouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);

		CsInputComponent* mBackRef;
	};	// MessageRouter

public:
	CsInputComponent();

	sal_override ~CsInputComponent();

// Operations
	sal_override void update();

	sal_override float getAxis(sal_in_z const wchar_t* axisName) const;

	sal_override float getAxisRaw(sal_in_z const wchar_t* axisName) const;

	sal_override bool anyKey() const;

	sal_override bool anyKeyDown() const;

	sal_override bool getButton(sal_in_z const wchar_t* buttonName) const;

	sal_override bool getButtonDown(sal_in_z const wchar_t* buttonName) const;

	sal_override bool getButtonUp(sal_in_z const wchar_t* buttonName) const;

	sal_override MCD::Vec2i getMousePosition() const;

	sal_override bool getMouseButton(int button) const;

	sal_override bool getMouseButtonDown(int button) const;

	sal_override bool getMouseButtonUp(int button) const;

	sal_override sal_out_z const wchar_t* inputString() const;

	//!	Attach the CsInputComponent to an existing windows control.
	void attachTo(System::Windows::Forms::Control^ control);

protected:
	struct Compare {
		bool operator()(const std::wstring& lhs, const std::wstring& rhs) const;
	};	// Compare

	gcroot<System::Windows::Forms::Control^> mControl;
	gcroot<MessageRouter^> mMessageRouter;

	typedef std::map<std::wstring, int, Compare> EventList;
	EventList mAxisList;
	EventList mKeyList;
	EventList mKeyDownList;
	EventList mKeyUpList;
	MCD::Vec2i mMousePosition;
	int8_t mMouseKeyBitArray;	//!< 8 bit can store 8 mouse key
	int8_t mMouseKeyDownBitArray;
	int8_t mMouseKeyUpBitArray;
	std::wstring mInputString;
	MCD::Vec2f mMouseAxis, mMouseAxisRaw;
};	// CsInputComponent

}	// namespace Binding
