#include "Pch.h"
#include "../RenderWindow.h"
#include "../../Core/System/Log.h"
#include "../../Core/System/MemoryProfiler.h"
#include "../../Core/System/Window.inl"
#include "../../../3Party/glew/glew.h"

#if defined(MCD_VC)
#	include "RenderWindow.Win.inc"
#else
#	include "RenderWindow.X11.inc"
#endif

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

#ifdef MCD_VC
	{	// Create a dummy window first
		// See http://www.gamedev.net/community/forums/topic.asp?topic_id=423903 about a discussion
		// on using a dummy window.
		int showBackup = impl->mShowWindow;
		impl->mShowWindow = false;
		impl->createWindow();
		if(options)
			impl->setOptions(options);
		if(!impl->detectMultiSamplePixelFormat())
			Log::format(Log::Warn, "The requested level of full-screen anti-aliasing is not supported.");
		impl->destroy();
		impl->mShowWindow = showBackup;
	}
#endif

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
	return (::wglGetCurrentContext());
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

	if(eventReceived.Type == Event::Resized && renderContext()) {
		MCD_VERIFY(makeActive());

		uint w = mImpl->mWidth;
		uint h = mImpl->mHeight;
		glViewport(0, 0, w, h);
		glScissor(0, 0, w, h);
	}

	// Let Window (the super class of RenderWindow) to do the remaining job
	Window::onEvent(eventReceived);
}

}	// namespace MCD
