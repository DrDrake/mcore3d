#include "stdafx.h"
#include "CsInputComponent.h"
#include "Utility.h"
#include "../../MCD/Core/System/StrUtility.h"

using namespace System;
using namespace System::Windows::Forms;

using namespace MCD;
using namespace std;

namespace Binding {

bool CsInputComponent::Compare::operator()(const wstring& lhs, const wstring& rhs) const {
	return wstrCaseCmp(lhs.c_str(), rhs.c_str()) < 0;
}

CsInputComponent::CsInputComponent()
	: mControl(nullptr), mMousePosition(0),
	  mMouseKeyBitArray(0), mMouseKeyDownBitArray(0), mMouseKeyUpBitArray(0),
	  mMouseAxis(0), mMouseAxisRaw(0)
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
	}
}

void CsInputComponent::update()
{
	mKeyDownList.clear();
	mKeyUpList.clear();
	mMouseKeyDownBitArray = 0;
	mMouseKeyUpBitArray = 0;
	mInputString.clear();

	// Perform axis smoothing
	// TODO: Make the smoothing framerate independent
	mMouseAxis = mMouseAxisRaw * 0.5f + mMouseAxis * 0.5f;
}

float CsInputComponent::getAxis(sal_in_z const wchar_t* axisName) const
{
	if(wstrCaseCmp(axisName, L"mouse x") == 0)
		return mMouseAxis.x;
	if(wstrCaseCmp(axisName, L"mouse y") == 0)
		return mMouseAxis.y;
	return 0;
}

float CsInputComponent::getAxisRaw(sal_in_z const wchar_t* axisName) const
{
	if(wstrCaseCmp(axisName, L"mouse x") == 0)
		return mMouseAxisRaw.x;
	if(wstrCaseCmp(axisName, L"mouse y") == 0)
		return mMouseAxisRaw.y;
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

bool CsInputComponent::getButton(const wchar_t* buttonName) const
{
	EventList::const_iterator i = mKeyList.find(buttonName);
	if(i == mKeyList.end())
		return false;

	return i->second == 1;
}

bool CsInputComponent::getButtonDown(const wchar_t* buttonName) const
{
	return mKeyDownList.find(buttonName) != mKeyDownList.end();
}

bool CsInputComponent::getButtonUp(const wchar_t* buttonName) const
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

const wchar_t* CsInputComponent::inputString() const
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
}

//! Some key code's string in .net is strange, so we perform a transformation to our usual convention.
static std::wstring transformKeyString(System::String^ s)
{
	std::wstring ret = Utility::toWString(s);
	if(ret == L"Next")
		ret = L"PageDown";
	return ret;
}

void CsInputComponent::MessageRouter::onKeyUp(System::Object^ sender, KeyEventArgs^ e)
{
	std::wstring s = transformKeyString(e->KeyData.ToString());
	mBackRef->mKeyList.erase(s);
	mBackRef->mKeyUpList[s] = 1;
}

void CsInputComponent::MessageRouter::onKeyDown(System::Object^ sender, KeyEventArgs^ e)
{
	std::wstring s = transformKeyString(e->KeyData.ToString());
	mBackRef->mKeyList[s] = 1;
	mBackRef->mKeyDownList[s] = 1;
}

void CsInputComponent::MessageRouter::onMouseUp(System::Object^ sender, MouseEventArgs^ e)
{
	mBackRef->mMouseKeyBitArray &= ~(1 << int(e->Button));
	mBackRef->mMouseKeyUpBitArray |= (1 << int(e->Button));
}

void CsInputComponent::MessageRouter::onMouseDown(System::Object^ sender, MouseEventArgs^ e)
{
	mBackRef->mMouseKeyBitArray |= (1 << int(e->Button));
	mBackRef->mMouseKeyDownBitArray |= (1 << int(e->Button));
}

void CsInputComponent::MessageRouter::onMouseMove(System::Object^ sender, MouseEventArgs^ e)
{
	Vec2i newPos(e->X, e->Y);
	mBackRef->mMouseAxisRaw.x += (newPos.x - mBackRef->mMousePosition.x);
	mBackRef->mMouseAxisRaw.y += (newPos.y - mBackRef->mMousePosition.y);
	mBackRef->mMousePosition = newPos;
}

}	// namespace Binding
