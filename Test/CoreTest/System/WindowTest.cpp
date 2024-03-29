#include "Pch.h"
#include "../../../MCD/Core/System/Thread.h"
#include "../../../MCD/Core/System/Window.h"
#include "../../../MCD/Core/System/WindowEvent.h"

using namespace MCD;

#ifndef MCD_IPHONE

TEST(Create_WindowTest)
{
	Window window;
	CHECK(window.create("title='Hello world!';width=400;height=300"));
	window.setOptions("showCursor=0");

	CHECK(window.handle() != 0);
	CHECK_EQUAL(400u, window.width());
	CHECK_EQUAL(300u, window.height());

	while(true) {
		Event e;
		if(!window.popEvent(e, true)) {
			mSleep(10);
			continue;
		}

		if(e.Type == Event::Closed) {
			window.destroy();
			break;
		}
	}
}

TEST(Multiple_WindowTest)
{
	Window window1, window2;
	CHECK(window1.create("title='Window1';width=100;height=200"));
	CHECK(window2.create("title='Window2';width=200;height=100"));
	window1.setOptions("showCursor=1");

	bool w1Active = true;
	bool w2Active = true;

	while(true) {
		Event e1, e2;
		bool hasEvent = false;

		if(w1Active) {
			hasEvent |= window1.popEvent(e1, false);

			if(e1.Type == Event::Closed) {
				window1.destroy();
				w1Active = false;
			}
		}

		if(w2Active) {
			hasEvent |= window2.popEvent(e2, false);

			if(e2.Type == Event::Closed) {
				window2.destroy();
				w2Active = false;
			}
		}

		if(!hasEvent) {
			mSleep(10);
			continue;
		}

		if(!w1Active && !w2Active)
			break;
	}

	CHECK(true);
}

#endif
