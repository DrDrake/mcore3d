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
static void parseXy(const char* option, int& x, int& y)
{
	const char* name = nullptr, *value = nullptr;
	NvpParser parser(option);
	while(parser.next(name, value)) {
		if(*name == 'x')
			x = str2IntWithDefault(value, 0);
		else if(*name == 'y')
			y = str2IntWithDefault(value, 0);
	}
}

ImplBase::ImplBase()
{
	// Give a small value
	mWidth = mHeight = 1;
	mTitle = "Simple game engine";
}

void ImplBase::setOptions(const char* option)
{
	const char* name = nullptr, *value = nullptr;
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
	if(mEventQueue.empty()) {
		event.Type = Event::None;
		return false;
	}

	event = mEventQueue.front();
	mEventQueue.pop();

	return true;
}

void ImplBase::addListener(IWindowListener& listener)
{
	mListeners.insert(&listener);
}

void ImplBase::removeListener(IWindowListener& listener)
{
	mListeners.erase(&listener);
}

}	// namespace MCD

#if defined(MCD_VC)
#	include "Window.Win.inc"
#elif defined(MCD_IPHONE)
#	include "Window.iPhone.inc"
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
	if(mImpl)	// mImpl may be null if create() is never called
		destroy();
	delete mImpl;
}

Window::Handle Window::handle()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->mWnd;
}

bool Window::create(const char* options)
{
	return create(0, options);
}

bool Window::create(Handle existingControl, const char* options)
{
	if(!mImpl)
		mImpl = new Impl(*this);

	MCD_ASSUME(mImpl != nullptr);
	if(options)
		mImpl->setOptions(options);
	return mImpl->createWindow(existingControl);
}

void Window::setOptions(const char* options)
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

void Window::addListener(IWindowListener& listener)
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->addListener(listener);
}

void Window::removeListener(IWindowListener& listener)
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->removeListener(listener);
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
