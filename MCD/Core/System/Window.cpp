#include "Pch.h"
#include "Window.h"
#include "PlatformInclude.h"
#include "StrUtility.h"
#include "WindowEvent.h"

#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning(disable: 6011)
#endif
#include <map>		// For name/value pair map
#include <queue>	// For event queue
#include <set>		// For storing unique IWindowListener
#ifdef MCD_VC
#	pragma warning(pop)
#endif

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

//! Common base class for Impl of different platforms
class ImplBase
{
public:
	ImplBase()
	{
		// Give a small value
		mWidth = mHeight = 1;
		mTitle = L"Simple game engine";
	}

	virtual ~ImplBase() {}

	virtual void setOption(const wchar_t* name, const wchar_t* value) = 0;

	void setOptions(const wchar_t* option, bool updateMemberOnly=true)
	{
		const wchar_t* name = nullptr, *value = nullptr;
		NvpParser parser(option);
		while(parser.next(name, value)) {
			setOption(name, value);
		}
	}

	//! Send event to all window listeners
	void SendEvent(const Event& eventToSend)
	{
		for(std::set<IWindowListener*>::iterator i = mListeners.begin(); i != mListeners.end(); ++i)
			(*i)->onEvent(eventToSend);
	}

	void onEvent(const Event& eventReceived)
	{
		mEventQueue.push(eventReceived);
	}

	virtual void processEvent(bool blocking) = 0;

	bool getEvent(Event& event, bool blocking)
	{
		if(mEventQueue.empty())
			processEvent(blocking);

		if(mEventQueue.empty())
			return false;

		event = mEventQueue.front();
		mEventQueue.pop();

		return true;
	}

	uint mWidth, mHeight;
	std::wstring mTitle;

	std::set<IWindowListener*> mListeners;
	std::queue<Event> mEventQueue;
};	// ImplBase

}	// namespace MCD

#if defined(MCD_VC)
#	include "Window.Win.inc"
#else
#	include "Window.X11.inc"
#endif

namespace MCD {

Window::Window()
{
	mImpl = new Impl(*this);
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

void Window::create(const wchar_t* options) throw(std::runtime_error)
{
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

bool Window::getEvent(Event& event, bool blocking)
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->getEvent(event, blocking);
}

void Window::onEvent(const Event& eventReceived)
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->onEvent(eventReceived);
}

}	// namespace MCD
