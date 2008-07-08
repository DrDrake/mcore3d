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
	mImpl = new Impl(*this);
}

GlWindow::~GlWindow()
{
	destroy();
	delete mImpl;
}

void GlWindow::create(const wchar_t* options) throw(std::exception)
{
	MCD_ASSUME(mImpl != nullptr);
	if(options)
		mImpl->setOptions(options, true);
	mImpl->createNewWindow();

	// Make it active
	mImpl->makeActive();

	{	// Initialize glew
		GLenum err = glewInit();
		if(err != GLEW_OK)
			throw std::runtime_error((const char*)glewGetErrorString(err));
	}

	// Disable v-sync (by default)
	mImpl->setVerticalSync(false);
}

void GlWindow::destroy()
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->destroy();
}

bool GlWindow::makeActive()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->makeActive();
}

bool GlWindow::swapBuffers()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->swapBuffers();
}

bool GlWindow::setVerticalSync(bool flag)
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->setVerticalSync(flag);
}

}	// namespace MCD
