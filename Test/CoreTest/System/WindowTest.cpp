#include "Pch.h"
#include "../../../SGE/Core/System/Thread.h"
#include "../../../SGE/Core/System/Window.h"
#include "../../../SGE/Core/System/WindowEvent.h"

using namespace SGE;

TEST(Create_WindowTest)
{
	Window window;
	window.create(L"title='Hello world!';width=400;height=400");
	window.setOptions(L"showCursor=0");

	while(true) {
		Event e;
		if(!window.getEvent(e, true)) {
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
	window1.create(L"title='Window1';width=100;height=200");
	window2.create(L"title='Window2';width=200;height=100");
	window1.setOptions(L"showCursor=1");

	bool w1Active = true;
	bool w2Active = true;

	while(true) {
		Event e1, e2;
		bool hasEvent = false;

		if(w1Active) {
			hasEvent |= window1.getEvent(e1, false);

			if(e1.Type == Event::Closed) {
				window1.destroy();
				w1Active = false;
			}
		}

		if(w2Active) {
			hasEvent |= window2.getEvent(e2, false);

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
}
