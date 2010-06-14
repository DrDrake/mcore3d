#import "Pch.h"
#import "RenderWindow.inc"
#import "../../Core/System/Log.h"
#import "../../Core/System/Window.inl"
#import "../../Core/System/Window.iPhone.inl"

// Reference:
// http://developer.apple.com/iphone/library/documentation/3DDrawing/Conceptual/OpenGLES_ProgrammingGuide/DeterminingOpenGLESCapabilities/DeterminingOpenGLESCapabilities.html
// http://developer.apple.com/iphone/library/documentation/3DDrawing/Conceptual/OpenGLES_ProgrammingGuide/WorkingwithEAGLContexts/WorkingwithEAGLContexts.html

namespace MCD {

void RenderWindow::Impl::createWindow(Window::Handle existingWindowHandle)
{
	mContext = [[[EAGLContext alloc] retain] initWithAPI:kEAGLRenderingAPIOpenGLES1];
}

void RenderWindow::Impl::destroy()
{
	[mContext release];
	mContext = nil;
}

bool RenderWindow::Impl::makeActive()
{
	return [EAGLContext setCurrentContext: mContext];
}

RenderWindow::RenderWindow()
{
}

RenderWindow::~RenderWindow()
{
	destroy();
}

void* RenderWindow::renderContext()
{
	return mImpl ? static_cast<Impl*>(mImpl)->mContext : nullptr;
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

//	if(options)
//		impl->setOptions(options);

	impl->createWindow(existingControl);
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
	return [EAGLContext currentContext];
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

/*	if(eventReceived.Type == Event::Resized) {
		MCD_VERIFY(makeActive());

		uint w = mImpl->mWidth;
		uint h = mImpl->mHeight;
		glViewport(0, 0, w, h);
		glScissor(0, 0, w, h);
	}*/

	// Let Window (the super class of RenderWindow) to do the remaining job
	Window::onEvent(eventReceived);
}

}	// namespace MCD
