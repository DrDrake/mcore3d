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
		: Super(w), mMultiSampleLevel(0)
	{
	}

	sal_override ~Impl()
	{
		destroy();
	}

	sal_override void createNewWindow()
	{
		{	// Create a dummy window just for quering of multi-sample's pixel format
			BOOL showWindowBackup = mShowWindow;
			mShowWindow = false;
			Super::createNewWindow();
			mShowWindow = showWindowBackup;
		}

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
		pfd.cColorBits = mColorBits;
		pfd.cDepthBits = 16;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int pixelFormat = ::ChoosePixelFormat(mDc, &pfd);

		if(!createContext(pixelFormat, pfd))
			throw std::runtime_error("Fail to create render context");

		if(mMultiSampleLevel == 0) {
			setShowWindow(mShowWindow ? L"1" : L"0");
			return;
		}

		pixelFormat = getMultiSamplePixelFormat(pfd, mMultiSampleLevel);
		if(pixelFormat > 0) {
			destroy();
			Super::createNewWindow();
			mDc = ::GetDC(HWND(mWnd));
		}

		if(!createContext(pixelFormat, pfd))
			throw std::runtime_error("Fail to create render context");

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

private:
	bool createContext(int pixelFormat, const PIXELFORMATDESCRIPTOR& pfd)
	{
		if(pixelFormat == 0 || !::SetPixelFormat(mDc, pixelFormat, &pfd))
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

	/*!	Get a pixel format that support the requested level of multi-sampling.
		Example from Nehe: http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=46
		See http://www.gamedev.net/community/forums/topic.asp?topic_id=423903 about a discussion
		on using a dummy window.
		\return 0 if failed
	 */
	int getMultiSamplePixelFormat(const PIXELFORMATDESCRIPTOR& pfd, uint multiSampleLevel)
	{
		float fAttributes[] = { 0, 0 };

		int iAttributes[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 24,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, pfd.cDepthBits,
			WGL_STENCIL_BITS_ARB, pfd.cStencilBits,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, multiSampleLevel,
			0, 0
		};

		if(!wglChoosePixelFormatARB)
			return 0;

		int pixelFormat = 0;
		uint numFormats;
		// First We Check To See If We Can Get A Pixel Format For 4 Samples
		BOOL valid = wglChoosePixelFormatARB(mDc, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);

		if(!valid || numFormats < 1)
			return 0;

		return pixelFormat;
	}

	HDC mDc;
	HGLRC mRc;
	uint mMultiSampleLevel;
};	// Impl

}	// namespace MCD
