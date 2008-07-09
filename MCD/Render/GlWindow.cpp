#include "Pch.h"
#include "GlWindow.h"
#include "../Core/System/Window.inl"

#define GLEW_STATIC
#include "../../3Party/glew/glew.h"

#if defined(MCD_VC)
#	include "GlWindow.Win.inc"
#else
#	include "GlWindow.X11.inc"
#endif

#undef GLEW_STATIC

namespace MCD {

GlWindow::GlWindow()
{
}

GlWindow::~GlWindow()
{
	destroy();
}

void GlWindow::create(const wchar_t* options) throw(std::exception)
{
	if(!mImpl)
		mImpl = new Impl(*this);

	// Not that we should operate on the local Impl class but not Window::Impl class.
	Impl* impl = static_cast<Impl*>(mImpl);
	MCD_ASSUME(impl != nullptr);

	if(options)
		impl->setOptions(options);
	impl->createNewWindow();

	// Make it active
	impl->makeActive();

	{	// Initialize glew
		GLenum err = glewInit();
		if(err != GLEW_OK)
			throw std::runtime_error((const char*)glewGetErrorString(err));
	}

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
	MCD_ASSUME(mImpl != nullptr);
	return static_cast<Impl*>(mImpl)->swapBuffers();
}

bool GlWindow::setVerticalSync(bool flag)
{
	MCD_ASSUME(mImpl != nullptr);
	return static_cast<Impl*>(mImpl)->setVerticalSync(flag);
}

}	// namespace MCD
