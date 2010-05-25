#include "Pch.h"
#include "../RenderWindow.h"
#include "../../Core/System/Log.h"
#include "../../Core/System/MemoryProfiler.h"
#include "../../Core/System/Window.inl"
#include "RenderWindow.Win.inc"

namespace MCD {

RenderWindow::RenderWindow()
{
}

RenderWindow::~RenderWindow()
{
	destroy();
}

void* RenderWindow::renderContext()
{
	if(!mImpl)
		return nullptr;

	return static_cast<Impl*>(mImpl)->renderContext();
}

void RenderWindow::create(const char* options) throw(std::exception)
{
	create(0, options);
}

void RenderWindow::create(Handle existingControl, const char* options) throw(std::exception)
{
	if(!mImpl)
		mImpl = new Impl(*this);

	// Not that we should operate on the local Impl class but not Window::Impl class.
	Impl* impl = static_cast<Impl*>(mImpl);
	MCD_ASSUME(impl != nullptr);

	if(options)
		impl->setOptions(options);

	impl->createWindow(existingControl);

	// Disable v-sync (by default)
	impl->setVerticalSync(false);
}

void RenderWindow::destroy()
{
	MCD_ASSUME(mImpl != nullptr);
	static_cast<Impl*>(mImpl)->destroy();
}

bool RenderWindow::makeActive()
{
	MCD_ASSUME(mImpl != nullptr);
	return static_cast<Impl*>(mImpl)->makeActive();
}

void* RenderWindow::getActiveContext()
{
	return Impl::getActiveContext();
}

void RenderWindow::preUpdate()
{
	MCD_ASSUME(mImpl != nullptr);
	return static_cast<Impl*>(mImpl)->preUpdate();
}

void RenderWindow::postUpdate()
{
	MCD_ASSUME(mImpl != nullptr);
	return static_cast<Impl*>(mImpl)->postUpdate();
}

bool RenderWindow::setVerticalSync(bool flag)
{
	MCD_ASSUME(mImpl != nullptr);
	return static_cast<Impl*>(mImpl)->setVerticalSync(flag);
}

void RenderWindow::onEvent(const Event& eventReceived)
{
	MCD_ASSUME(mImpl != nullptr);

	if(eventReceived.Type == Event::Resized) {
//		static_cast<RenderWindow::Impl*>(mImpl)->onResize();
	}

	// Let Window (the super class of RenderWindow) to do the remaining job
	Window::onEvent(eventReceived);
}

}	// namespace MCD
