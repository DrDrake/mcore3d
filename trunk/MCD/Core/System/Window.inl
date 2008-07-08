// User must include "Window.h" before Window.inl

#include "WindowEvent.h"

#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning(disable: 6011)
#endif
#include <queue>	// For event queue
#include <set>		// For storing unique IWindowListener
#ifdef MCD_VC
#	pragma warning(pop)
#endif

namespace MCD {

//!	Common base class for Impl on different platforms.
class MCD_NOVTABLE ImplBase
{
public:
	ImplBase();

	virtual ~ImplBase() {}

	virtual void setOption(const wchar_t* name, const wchar_t* value) = 0;

	MCD_CORE_API void setOptions(const wchar_t* option, bool updateMemberOnly=true);

	//! Send event to all window listeners
	void SendEvent(const Event& eventToSend);

	void onEvent(const Event& eventReceived);

	virtual void processEvent(bool blocking) = 0;

	bool popEvent(Event& event, bool blocking);

	//! Width and height of the window (not client area)
	uint mWidth, mHeight;
	std::wstring mTitle;

	std::set<IWindowListener*> mListeners;
	std::queue<Event> mEventQueue;
};	// ImplBase

}	// namespace MCD
