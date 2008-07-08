// This inc file is to be included by GlWindow.cpp only

#include "../Core/System/Window.Win.inl"
#include "../../3Party/glew/wglew.h"

#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32")
#pragma comment(lib, "GlAux")
#pragma comment(lib, "glew")

namespace MCD {

class GlWindow::Impl : public Window::Impl
{
public:
	typedef Window::Impl Super;

	Impl(Window& w)
		: Super(w)
	{
	}

	~Impl()
	{
		destroy();
	}

	sal_override void createNewWindow()
	{
		Super::createNewWindow();

		// Get the device context
		mDc = ::GetDC(HWND(mWnd));
		if(mDc == nullptr)
			throw std::runtime_error("Win32 API call 'GetDC' failed");

		// Initialize WGL
		PIXELFORMATDESCRIPTOR pfd;
		::ZeroMemory(&pfd, sizeof(pfd));
		pfd.nVersion = 1;
		pfd.nSize = sizeof(pfd);
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 16;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int pixelFormat = ::ChoosePixelFormat(mDc, &pfd);
		if(pixelFormat == 0)
			throw std::runtime_error("Win32 API call 'ChoosePixelFormat' failed");

		if(!::SetPixelFormat(mDc, pixelFormat, &pfd))
			throw std::runtime_error("Win32 API call 'SetPixelFormat' failed");

		mRc = ::wglCreateContext(mDc);
		if(!mRc)
			throw std::runtime_error("Win32 API call 'wglCreateContext' failed");
	}

	sal_override void destroy()
	{
		// De-select the current rendering context and delete it
		::wglMakeCurrent(mDc, nullptr);
		::wglDeleteContext(mRc);

		::ReleaseDC(HWND(mWnd), mDc);

		Super::destroy();
	}

	bool makeActive()
	{
		return (::wglMakeCurrent(mDc, mRc) == TRUE);
	}

	bool swapBuffers()
	{
		return (::SwapBuffers(mDc) == TRUE);
	}

	bool setVerticalSync(bool flag)
	{
		if(glewIsSupported("WGLEW_swap_control")) {
			wglSwapIntervalEXT(flag ? 1 : 0);
			return true;
		}
		return false;
	}

	HDC mDc;
	HGLRC mRc;
};	// Impl

}	// namespace MCD
