#include "Pch.h"
#include "OglContext.h"
#include "../Core/System/PlatformInclude.h"

#define GLEW_STATIC

#include "../../3Party/glew/glew.h"

#if defined(MCD_VC)
#	include "../../3Party/glew/wglew.h"
#	pragma comment(lib, "OpenGL32")
#	pragma comment(lib, "GLU32")
#	pragma comment(lib, "GlAux")
#	pragma comment(lib, "glew")
#else
#	include <X11/Xlib.h>	// XOpenDisplay
#	include <GL/glx.h>		// glXQueryVersion
#endif

#undef GLEW_STATIC

namespace MCD {

class OglContext::Impl
{
public:
	Impl(OglContext::Handle windowHandle)
		: mWnd(windowHandle)
	{
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

		makeActive();

		{	// Initialize glew
			GLenum err = glewInit();
			if(err != GLEW_OK)
				throw std::runtime_error((const char*)glewGetErrorString(err));
		}

		// Disable v-sync
		if(glewIsSupported("WGLEW_swap_control"))
			wglSwapIntervalEXT(0);
	}

	~Impl()
	{
		// De-select the current rendering context and delete it
		::wglMakeCurrent(mDc, nullptr);
		::wglDeleteContext(mRc);

		::ReleaseDC(HWND(mWnd), mDc);
	}

	bool makeActive()
	{
		return (::wglMakeCurrent(mDc, mRc) == TRUE);
	}

	bool swapBuffers()
	{
		return (::SwapBuffers(mDc) == TRUE);
	}

	OglContext::Handle mWnd;
	HDC mDc;
	HGLRC mRc;
};	// Impl

OglContext::OglContext(Handle windowHandle) throw(std::runtime_error)
{
	mImpl = new Impl(windowHandle);
}

OglContext::~OglContext()
{
	delete mImpl;
}

bool OglContext::makeActive()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->makeActive();
}

bool OglContext::swapBuffers()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->swapBuffers();
}

}	// namespace MCD
