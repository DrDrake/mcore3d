// This inc file is to be included by GlWindow.cpp only

#include "../Core/System/StrUtility.h"
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
		: Super(w),
		mDefaultPixelFormat(0), mMultiSamplePixelFormat(0),
		mMultiSampleLevel(0)
	{
	}

	sal_override ~Impl()
	{
		destroy();
	}

	sal_override void createWindow(Window::Handle existingWindowHandle=0)
	{
		Super::createWindow(existingWindowHandle);

		// Get the device context
		mDc = ::GetDC(HWND(mWnd));
		if(mDc == nullptr)
			throw std::runtime_error("Win32 API call 'GetDC' failed");

		if(!mDefaultPixelFormat) {
			// Initialize WGL
			::ZeroMemory(&mPfd, sizeof(mPfd));
			mPfd.nVersion = 1;
			mPfd.nSize = sizeof(mPfd);
			mPfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			mPfd.iPixelType = PFD_TYPE_RGBA;
			mPfd.cColorBits = mColorBits;
			mPfd.cDepthBits = 24;
			mPfd.cStencilBits = 8;
			mPfd.iLayerType = PFD_MAIN_PLANE;

			mDefaultPixelFormat = ::ChoosePixelFormat(mDc, &mPfd);
		}

		if(mMultiSamplePixelFormat)
			mDefaultPixelFormat = mMultiSamplePixelFormat;

		if(!createContext())
			throw std::runtime_error("Fail to create render context");

		if(mMultiSamplePixelFormat)
			glEnable(GL_MULTISAMPLE);
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

	sal_override void setOption(const wchar_t* name, const wchar_t* value)
	{
		if(::wcscmp(name, L"FSAA") == 0)
			mMultiSampleLevel = wStr2IntWithDefault(value, 0);
		else
			Super::setOption(name, value);
	}

	/*!	Get a pixel format that support the requested level of multi-sampling.
		Example from Nehe: http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=46
		\return false if failed
	 */
	bool detectMultiSamplePixelFormat()
	{
		// If the user didn't ask for FSAA, just use the default pixel format
		if(mMultiSampleLevel == 0) {
			mMultiSamplePixelFormat = mDefaultPixelFormat;
			return true;
		}

		float fAttributes[] = { 0, 0 };

		int iAttributes[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 24,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, mPfd.cDepthBits,
			WGL_STENCIL_BITS_ARB, mPfd.cStencilBits,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, mMultiSampleLevel,
			0, 0
		};

		if(!wglChoosePixelFormatARB)
			return false;

		uint numFormats;
		BOOL valid = wglChoosePixelFormatARB(mDc, iAttributes, fAttributes, 1, &mMultiSamplePixelFormat, &numFormats);

		return (valid && numFormats >= 1);
	}

private:
	bool createContext()
	{
		if(mDefaultPixelFormat == 0 || !::SetPixelFormat(mDc, mDefaultPixelFormat, &mPfd))
			return false;

		mRc = ::wglCreateContext(mDc);
		if(!mRc || !makeActive())
			return false;

		{	// Initialize glew
			GLenum err = glewInit();
			if(err != GLEW_OK)
				return false;
		}

		return true;
	}

	HDC mDc;
	HGLRC mRc;
	int mDefaultPixelFormat;
	int mMultiSamplePixelFormat;
	uint mMultiSampleLevel;
	PIXELFORMATDESCRIPTOR mPfd;
};	// Impl

}	// namespace MCD
