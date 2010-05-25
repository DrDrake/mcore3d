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
		: Super(w), mDevice(nullptr)
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

	void onResize()
	{
		mD3dpp.BackBufferWidth  = mWidth;
		mD3dpp.BackBufferHeight = mHeight;		
		if(mDevice)
			mDevice->Reset(&mD3dpp);
//		(void)hr;
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

	void preUpdate()
	{
		mDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0x33, 0x66, 0x99), 1.0f, 0);
		mDevice->BeginScene();
	}

	void postUpdate()
	{
		mDevice->EndScene();
		mDevice->Present(nullptr, nullptr, nullptr, nullptr);
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

		ZeroMemory(&mD3dpp, sizeof(mD3dpp));
		mD3dpp.hDeviceWindow = mWnd;
		mD3dpp.Windowed = !mFullScreen;
		mD3dpp.BackBufferCount = 1;
		mD3dpp.BackBufferWidth = mWidth;
		mD3dpp.BackBufferHeight = mHeight;
		mD3dpp.BackBufferFormat = D3DFMT_UNKNOWN;	// D3DFMT_UNKNOWN can be used for windowed mode
		mD3dpp.EnableAutoDepthStencil = true;
		mD3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		mD3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		mD3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;

		if( FAILED(gD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mWnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&mD3dpp, &mDevice))
		)
			return false;

		return true;
	}

	sal_maybenull LPDIRECT3DDEVICE9 mDevice;
	D3DPRESENT_PARAMETERS mD3dpp;
};	// Impl

}	// namespace MCD
