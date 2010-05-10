// This inc file is to be included by RenderWindow.cpp only

#include "../../Core/System/StrUtility.h"
#include "../../Core/System/MemoryProfiler.h"
#include "../../Core/System/Window.Win.inl"
#include <d3d9.h>

#pragma comment(lib, "d3d9")

PFNGLLOADTRANSPOSEMATRIXFPROC a;

namespace MCD {

static LPDIRECT3D9 gD3D = nullptr;
static size_t gD3DUseCount = 0;
static LPDIRECT3DDEVICE9 gCurrentDevice;

class RenderWindow::Impl : public Window::Impl
{
public:
	typedef Window::Impl Super;

	Impl(Window& w)
		: Super(w)
	{
	}

	sal_override ~Impl()
	{
		destroy();
	}

	void* renderContext()
	{
		return mDevice;
	}

	sal_override void createWindow(Window::Handle existingWindowHandle=0)
	{
		MemoryProfiler::Scope profiler("RenderWindow::createWindow");
		Super::createWindow(existingWindowHandle);

		if(!createContext())
			throw std::runtime_error("Fail to create render context");
	}

	sal_override void destroy()
	{
		if(mDevice) {
			mDevice->Release();
			mDevice = nullptr;
		}

		if((--gD3DUseCount) == 0) {
			gD3D->Release();
			gD3D = nullptr;
		}

		Super::destroy();
	}

	bool makeActive()
	{
		gCurrentDevice = mDevice;
		return true;
	}

	static void* getActiveContext()
	{
		return gCurrentDevice;
	}

	bool swapBuffers()
	{
		// TODO: Optimization here
		mDevice->EndScene();
		mDevice->Present(nullptr, nullptr, nullptr, nullptr);
		mDevice->BeginScene();
		return true;
	}

	bool setVerticalSync(bool flag)
	{
		return false;
	}

	sal_override void setOption(const char* name, const char* value)
	{
		Super::setOption(name, value);
	}

private:
	bool createContext()
	{
		if(!gD3D) {
			if(nullptr == (gD3D = ::Direct3DCreate9(D3D_SDK_VERSION)))
				return false;
		}

		++gD3DUseCount;

		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

		if( FAILED(gD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mWnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&d3dpp, &mDevice))
		)
			return false;

		return true;
	}

	sal_maybenull LPDIRECT3DDEVICE9 mDevice;
};	// Impl

}	// namespace MCD
