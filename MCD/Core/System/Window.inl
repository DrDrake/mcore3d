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

/*!	Common base class for Impl on different platforms.
	Opps! After I read the bridge pattern http://en.wikipedia.org/wiki/Bridge_pattern
	I realized that I am alredy using it even I didn't have that in mind.
 */
class MCD_ABSTRACT_CLASS ImplBase
{
public:
	ImplBase();

	virtual ~ImplBase() {}

	virtual void setOption(const char* name, const char* value) = 0;

	MCD_CORE_API virtual void setOptions(const char* option);

	//! Send event to all window listeners
	void SendEvent(const Event& eventToSend);

	void onEvent(const Event& eventReceived);

	virtual void processEvent(bool blocking) = 0;

	bool popEvent(Event& event, bool blocking);

	void addListener(IWindowListener& listener);

	void removeListener(IWindowListener& listener);

	//! Width and height of the window (not client area)
	uint mWidth, mHeight;
	std::string mTitle;

	std::set<IWindowListener*> mListeners;
	std::queue<Event> mEventQueue;
};	// ImplBase

}	// namespace MCD
