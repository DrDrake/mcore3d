#import "../RenderWindow.h"
#import "../../Core/System/Window.inl"
#import "../../Core/System/Window.iPhone.inl"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>

namespace MCD {

class RenderWindow::Impl : public Window::Impl
{
	typedef Window::Impl Super;

public:
	Impl(Window& w) : Super(w), mContext(nil) {}
	sal_override void createWindow(Window::Handle existingWindowHandle=0);
	sal_override void destroy();
	bool makeActive();
	void preUpdate();
	void postUpdate();
	bool setVerticalSync(bool flag);
	sal_override void setOption(const char* name, const char* value);

	EAGLContext* mContext;
//	size_t mWidth, mHeight;
};	// Impl

}	// namespace MCD
