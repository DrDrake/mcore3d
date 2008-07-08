#include "Pch.h"
#include "Window.h"
#include "PlatformInclude.h"
#include "StrUtility.h"
#include "Window.inl"
#include "WindowEvent.h"
#include <iostream>
namespace MCD {

static int toInt(const wchar_t* value, int defaultVal)
{
	if(!value) return defaultVal;
	int ret;
	if(wStr2Int(value, ret))
		return ret;
	return defaultVal;
}

//! Parse a string with the format "x=123;y=456;"
static void parseXy(const wchar_t* option, int& x, int& y)
{
	const wchar_t* name = nullptr, *value = nullptr;
	NvpParser parser(option);
	while(parser.next(name, value)) {
		if(*name == L'x')
			x = toInt(value, 0);
		else if(*name == L'y')
			y = toInt(value, 0);
	}
}

ImplBase::ImplBase()
{
	// Give a small value
	mWidth = mHeight = 1;
	mTitle = L"Simple game engine";
}

void ImplBase::setOptions(const wchar_t* option, bool updateMemberOnly)
{
	const wchar_t* name = nullptr, *value = nullptr;
	NvpParser parser(option);
	while(parser.next(name, value)) {
		setOption(name, value);
	}
}

//! Send event to all window listeners
void ImplBase::SendEvent(const Event& eventToSend)
{
	for(std::set<IWindowListener*>::iterator i = mListeners.begin(); i != mListeners.end(); ++i)
		(*i)->onEvent(eventToSend);
}

void ImplBase::onEvent(const Event& eventReceived)
{
	mEventQueue.push(eventReceived);
}

bool ImplBase::popEvent(Event& event, bool blocking)
{
	if(mEventQueue.empty())
		processEvent(blocking);

	// After event may push into the queue after calling processEvent()
	// therefore we need to check for empty() again
	if(mEventQueue.empty())
		return false;

	event = mEventQueue.front();
	mEventQueue.pop();

	return true;
}

}	// namespace MCD

#if defined(MCD_VC)
#	include "Window.Win.inc"
#else
#	include "Window.X11.inc"
#endif

namespace MCD {

Window::Window()
	: mImpl(nullptr)
{
}

Window::~Window()
{
	destroy();
	delete mImpl;
}

Window::Handle Window::handle()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->mWnd;
}

void Window::create(const wchar_t* options) throw(std::exception)
{
	if(!mImpl)
		mImpl = new Impl(*this);

	MCD_ASSUME(mImpl != nullptr);
	if(options)
		mImpl->setOptions(options, true);
	mImpl->createNewWindow();
}

void Window::setOptions(const wchar_t* options)
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->setOptions(options, false);
}

void Window::destroy()
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->destroy();
}

void Window::processEvent(bool blocking)
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->processEvent(blocking);
}

bool Window::popEvent(Event& event, bool blocking)
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->popEvent(event, blocking);
}

void Window::onEvent(const Event& eventReceived)
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->onEvent(eventReceived);
}

}	// namespace MCD
