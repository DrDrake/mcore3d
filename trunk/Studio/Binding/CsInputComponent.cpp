#include "stdafx.h"
#include "CsInputComponent.h"
#include "Utility.h"
#include "../../MCD/Core/System/StrUtility.h"

using namespace System;
using namespace System::Windows::Forms;

using namespace MCD;
using namespace std;

namespace Binding {

bool CsInputComponent::Compare::operator()(const string& lhs, const string& rhs) const {
	return strCaseCmp(lhs.c_str(), rhs.c_str()) < 0;
}

CsInputComponent::CsInputComponent()
	: mControl(nullptr), mMousePosition(0),
	  mMouseKeyBitArray(0), mMouseKeyDownBitArray(0), mMouseKeyUpBitArray(0),
	  mMouseAxis(0), mMouseAxisRaw(0),
	  mPreviousMouseAxis(0), mPreviousMouseAxisRaw(0)
{
	mMessageRouter = gcnew MessageRouter();
	mMessageRouter->mBackRef = this;
}

CsInputComponent::~CsInputComponent()
{
	if(mControl) {
		mControl->KeyUp -= gcnew KeyEventHandler(mMessageRouter, &MessageRouter::onKeyUp);
		mControl->KeyDown -= gcnew KeyEventHandler(mMessageRouter, &MessageRouter::onKeyDown);
		mControl->MouseUp -= gcnew MouseEventHandler(mMessageRouter, &MessageRouter::onMouseUp);
		mControl->MouseDown -= gcnew MouseEventHandler(mMessageRouter, &MessageRouter::onMouseDown);
		mControl->MouseMove -= gcnew MouseEventHandler(mMessageRouter, &MessageRouter::onMouseMove);
		mControl->MouseWheel -= gcnew MouseEventHandler(mMessageRouter, &MessageRouter::onMouseWheel);
	}
}

void CsInputComponent::update(float)
{
	mKeyDownList.clear();
	mKeyUpList.clear();
	mMouseKeyDownBitArray = 0;
	mMouseKeyUpBitArray = 0;
	mInputString.clear();

	mPreviousMouseAxis = mMouseAxis;
	mPreviousMouseAxisRaw = mMouseAxisRaw;

	// Perform axis smoothing
	// TODO: Make the smoothing framerate independent
	mMouseAxis = mMouseAxisRaw * 0.5f + mMouseAxis * 0.5f;
}

float CsInputComponent::getAxis(sal_in_z const char* axisName) const
{
	if(strCaseCmp(axisName, "mouse x") == 0)
		return mMouseAxis.x;
	if(strCaseCmp(axisName, "mouse y") == 0)
		return mMouseAxis.y;
	if(strCaseCmp(axisName, "mouse z") == 0)
		return mMouseAxis.z;
	return 0;
}

float CsInputComponent::getAxisRaw(sal_in_z const char* axisName) const
{
	if(strCaseCmp(axisName, "mouse x") == 0)
		return mMouseAxisRaw.x;
	if(strCaseCmp(axisName, "mouse y") == 0)
		return mMouseAxisRaw.y;
	if(strCaseCmp(axisName, "mouse z") == 0)
		return mMouseAxisRaw.z;
	return 0;
}

float CsInputComponent::getAxisDelta(sal_in_z const char* axisName) const
{
	if(strCaseCmp(axisName, "mouse x") == 0)
		return mMouseAxis.x - mPreviousMouseAxis.x;
	if(strCaseCmp(axisName, "mouse y") == 0)
		return mMouseAxis.y - mPreviousMouseAxis.y;
	if(strCaseCmp(axisName, "mouse z") == 0)
		return mMouseAxis.z - mPreviousMouseAxis.z;
	return 0;
}

float CsInputComponent::getAxisDeltaRaw(sal_in_z const char* axisName) const
{
	if(strCaseCmp(axisName, "mouse x") == 0)
		return mMouseAxisRaw.x - mPreviousMouseAxisRaw.x;
	if(strCaseCmp(axisName, "mouse y") == 0)
		return mMouseAxisRaw.y - mPreviousMouseAxisRaw.y;
	if(strCaseCmp(axisName, "mouse z") == 0)
		return mMouseAxisRaw.z - mPreviousMouseAxisRaw.z;
	return 0;
}

bool CsInputComponent::anyKey() const
{
	return !mKeyList.empty();
}

bool CsInputComponent::anyKeyDown() const
{
	return !mKeyDownList.empty();
}

bool CsInputComponent::getButton(const char* buttonName) const
{
	EventList::const_iterator i = mKeyList.find(buttonName);
	if(i == mKeyList.end())
		return false;

	return i->second == 1;
}

bool CsInputComponent::getButtonDown(const char* buttonName) const
{
	return mKeyDownList.find(buttonName) != mKeyDownList.end();
}

bool CsInputComponent::getButtonUp(const char* buttonName) const
{
	return mKeyUpList.find(buttonName) != mKeyUpList.end();
}

Vec2i CsInputComponent::getMousePosition() const
{
	return mMousePosition;
}

bool CsInputComponent::getMouseButton(int button) const
{
//	if(button >= MCD::Mouse::Count)
//		return false;
	return (mMouseKeyBitArray & (1 << button)) > 0;
}

bool CsInputComponent::getMouseButtonDown(int button) const
{
//	if(button >= MCD::Mouse::Count)
//		return false;
	return (mMouseKeyDownBitArray & (1 << button)) > 0;
}

bool CsInputComponent::getMouseButtonUp(int button) const
{
//	if(button >= MCD::Mouse::Count)
//		return false;
	return (mMouseKeyUpBitArray & (1 << button)) > 0;
}

const char* CsInputComponent::inputString() const
{
	return mInputString.c_str();
}

void CsInputComponent::attachTo(Control^ control)
{
	mControl = control;

	control->KeyUp += gcnew KeyEventHandler(mMessageRouter, &MessageRouter::onKeyUp);
	control->KeyDown += gcnew KeyEventHandler(mMessageRouter, &MessageRouter::onKeyDown);
	control->MouseUp += gcnew MouseEventHandler(mMessageRouter, &MessageRouter::onMouseUp);
	control->MouseDown += gcnew MouseEventHandler(mMessageRouter, &MessageRouter::onMouseDown);
	control->MouseMove += gcnew MouseEventHandler(mMessageRouter, &MessageRouter::onMouseMove);
	control->MouseWheel  += gcnew MouseEventHandler(mMessageRouter, &MessageRouter::onMouseWheel);
}

//! Some key code's string in .net is strange, so we perform a transformation to our usual convention.
static std::string transformKeyString(System::String^ s)
{
	std::string ret = Utility::toUtf8String(s);
	if(ret == "Next")
		ret = "PageDown";
	return ret;
}

void CsInputComponent::MessageRouter::onKeyUp(System::Object^ sender, KeyEventArgs^ e)
{
	std::string s = transformKeyString(e->KeyData.ToString());
	mBackRef->mKeyList.erase(s);
	mBackRef->mKeyUpList[s] = 1;
}

void CsInputComponent::MessageRouter::onKeyDown(System::Object^ sender, KeyEventArgs^ e)
{
	std::string s = transformKeyString(e->KeyData.ToString());
	mBackRef->mKeyList[s] = 1;
	mBackRef->mKeyDownList[s] = 1;
}

void CsInputComponent::MessageRouter::onMouseUp(System::Object^ sender, MouseEventArgs^ e)
{
	int buttonValue;
	switch(e->Button) {
		case MouseButtons::Left:
			buttonValue = 0;
			break;
		case MouseButtons::Right:
			buttonValue = 1;
			break;
		case MouseButtons::Middle:
			buttonValue = 2;
			break;
	}
	mBackRef->mMouseKeyBitArray &= ~(1 << buttonValue);
	mBackRef->mMouseKeyUpBitArray |= (1 << buttonValue);
}

void CsInputComponent::MessageRouter::onMouseDown(System::Object^ sender, MouseEventArgs^ e)
{
	int buttonValue;
	switch(e->Button) {
		case MouseButtons::Left:
			buttonValue = 0;
			break;
		case MouseButtons::Right:
			buttonValue = 1;
			break;
		case MouseButtons::Middle:
			buttonValue = 2;
			break;
	}
	mBackRef->mMouseKeyBitArray |= (1 << buttonValue);
	mBackRef->mMouseKeyDownBitArray |= (1 << buttonValue);
}

void CsInputComponent::MessageRouter::onMouseMove(System::Object^ sender, MouseEventArgs^ e)
{
	mBackRef->mPreviousMouseAxisRaw.x = mBackRef->mMouseAxisRaw.x;
	mBackRef->mPreviousMouseAxisRaw.y = mBackRef->mMouseAxisRaw.y;

	Vec2i newPos(e->X, e->Y);
	mBackRef->mMouseAxisRaw.x += (newPos.x - mBackRef->mMousePosition.x);
	mBackRef->mMouseAxisRaw.y += (newPos.y - mBackRef->mMousePosition.y);
	mBackRef->mMousePosition = newPos;
}

void CsInputComponent::MessageRouter::onMouseWheel(System::Object^ sender, MouseEventArgs^ e)
{
	mBackRef->mPreviousMouseAxisRaw.z = mBackRef->mMouseAxisRaw.z;

	// See http://msdn.microsoft.com/en-us/library/system.windows.forms.control.mousewheel.aspx
	mBackRef->mMouseAxisRaw.z += e->Delta / 120;
}

}	// namespace Binding
