#include "Pch.h"
#include "GlWindow.h"
#include "../Core/System/Log.h"
#include "../Core/System/Window.inl"
#include "../../3Party/glew/glew.h"

#if defined(MCD_VC)
#	include "GlWindow.Win.inc"
#else
#	include "GlWindow.X11.inc"
#endif

namespace MCD {

GlWindow::GlWindow()
{
}

GlWindow::~GlWindow()
{
	destroy();
}

void* GlWindow::glContext()
{
	if(!mImpl)
		return nullptr;

	return static_cast<Impl*>(mImpl)->glContext();
}

void GlWindow::create(const wchar_t* options) throw(std::exception)
{
	create(0, options);
}

void GlWindow::create(Handle existingControl, const wchar_t* options) throw(std::exception)
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
			Log::format(Log::Warn, L"The requested level of full-screen anti-aliasing is not supported.");
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

void GlWindow::destroy()
{
	MCD_ASSUME(mImpl != nullptr);
	static_cast<Impl*>(mImpl)->destroy();
}

bool GlWindow::makeActive()
{
	MCD_ASSUME(mImpl != nullptr);
	return static_cast<Impl*>(mImpl)->makeActive();
}

bool GlWindow::swapBuffers()
{
	MemoryProfiler::Scope profiler("GlWindow::swapBuffers");

	MCD_ASSUME(mImpl != nullptr);
	return static_cast<Impl*>(mImpl)->swapBuffers();
}

bool GlWindow::setVerticalSync(bool flag)
{
	MCD_ASSUME(mImpl != nullptr);
	return static_cast<Impl*>(mImpl)->setVerticalSync(flag);
}

void GlWindow::onEvent(const Event& eventReceived)
{
	MCD_ASSUME(mImpl != nullptr);

	if(eventReceived.Type == Event::Resized) {
		uint w = mImpl->mWidth;
		uint h = mImpl->mHeight;
		glViewport(0, 0, w, h);
		glScissor(0, 0, w, h);
	}

	// Let Window (the super class of GlWindow) to do the remaining job
	Window::onEvent(eventReceived);
}

}	// namespace MCD
