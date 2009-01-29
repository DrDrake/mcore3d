#include "Pch.h"
#include "Window.h"
#include "ErrorCode.h"
#include "Log.h"
#include "PlatformInclude.h"
#include "StrUtility.h"
#include "Window.inl"
#include "WindowEvent.h"

namespace MCD {

//! Parse a string with the format "x=123;y=456;"
static void parseXy(const wchar_t* option, int& x, int& y)
{
	const wchar_t* name = nullptr, *value = nullptr;
	NvpParser parser(option);
	while(parser.next(name, value)) {
		if(*name == L'x')
			x = wStr2IntWithDefault(value, 0);
		else if(*name == L'y')
			y = wStr2IntWithDefault(value, 0);
	}
}

ImplBase::ImplBase()
{
	// Give a small value
	mWidth = mHeight = 1;
	mTitle = L"Simple game engine";
}

void ImplBase::setOptions(const wchar_t* option)
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
	create(0, options);
}

void Window::create(Handle existingControl, const wchar_t* options) throw(std::exception)
{
	if(!mImpl)
		mImpl = new Impl(*this);

	MCD_ASSUME(mImpl != nullptr);
	if(options)
		mImpl->setOptions(options);
	mImpl->createWindow(existingControl);
}

void Window::setOptions(const wchar_t* options)
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->setOptions(options);
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

uint Window::width() const
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->mWidth;
}

uint Window::height() const
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->mHeight;
}

}	// namespace MCD
