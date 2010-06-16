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
static LPDIRECT3DDEVICE9 gDevice;
static void* gCurrentContext;

class RenderWindow::Impl : public Window::Impl
{
public:
	typedef Window::Impl Super;

	Impl(Window& w) : Super(w)
	{
		mContext.mDevice = nullptr;
		mContext.mSwapChain = nullptr;
	}

	sal_override ~Impl()
	{
		MCD_ASSERT(!mContext.mSwapChain && "destroy() should be already invoked");
	}

	void* renderContext()
	{
		return &mContext;
	}

	sal_override void createWindow(Window::Handle existingWindowHandle=0)
	{
		MemoryProfiler::Scope profiler("RenderWindow::createWindow");
		Super::createWindow(existingWindowHandle);

		if(!createContext()) {
			destroy();
			throw std::runtime_error("Fail to create render context");
		}
	}

	sal_override void destroy()
	{
		if(gCurrentContext == &mContext)
			gCurrentContext = nullptr;

		if(mContext.mSwapChain) {
			mContext.mSwapChain->Release();
			mContext.mSwapChain = nullptr;
		}

		mContext.mDevice = nullptr;

		if((--gD3DUseCount) == 0)
		{
			if(gDevice) {
				gDevice->Release();
				gDevice = nullptr;
			}

			gD3D->Release();
			gD3D = nullptr;
		}

		Super::destroy();
	}

	void onResize()
	{
		mD3dpp.BackBufferWidth  = mWidth;
		mD3dpp.BackBufferHeight = mHeight;
		if(gDevice)
			gDevice->Reset(&mD3dpp);
	}

	bool makeActive()
	{
		gCurrentContext = &mContext;
		return true;
	}

	static void* getActiveContext()
	{
		return gCurrentContext;
	}

	void preUpdate()
	{
		// Set mSwapChain as the current render target.
		LPDIRECT3DSURFACE9 backSurface = nullptr;
		mContext.mSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backSurface);
		gDevice->SetRenderTarget(0, backSurface);
		backSurface->Release();
	}

	void postUpdate()
	{
		mContext.mSwapChain->Present(nullptr, nullptr, nullptr, nullptr, 0);
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

		// Create the first device if none is created yet
		if(!gDevice) {
			if( FAILED(gD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING,
				&mD3dpp, &gDevice))
			)
				return false;

			MCD_ASSUME(gDevice);
			if(FAILED(gDevice->GetSwapChain(0, &mContext.mSwapChain)))
				return false;

			MCD_ASSERT(mContext.mSwapChain);
		}
		// And once a single device is created, swap chain is created for addition windows
		else {
			if(FAILED(gDevice->CreateAdditionalSwapChain(&mD3dpp, &mContext.mSwapChain)))
				return false;

			MCD_ASSERT(mContext.mSwapChain);
		}

		mContext.mDevice = gDevice;

		return true;
	}

	D3DPRESENT_PARAMETERS mD3dpp;

	struct Context {
		sal_maybenull LPDIRECT3DDEVICE9 mDevice;	//!< The same as gDevice
		sal_maybenull LPDIRECT3DSWAPCHAIN9 mSwapChain;
	} mContext;
};	// Impl

}	// namespace MCD
