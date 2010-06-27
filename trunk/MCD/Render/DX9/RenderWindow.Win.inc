// This inc file is to be included by RenderWindow.cpp only

#include "../Texture.h"
#include "../../Core/System/StrUtility.h"
#include "../../Core/System/MemoryProfiler.h"
#include "../../Core/System/Utility.h"
#include "../../Core/System/Window.Win.inl"
#include <d3d9.h>

#pragma comment(lib, "d3d9")

PFNGLLOADTRANSPOSEMATRIXFPROC a;

template<typename T> void SAFE_RELEASE(T& p)
{
	if(p) {
		p->Release();
		p = nullptr;
	}
}

namespace MCD {

struct Context
{
	HWND wnd;
	size_t width, height;
	LPDIRECT3DDEVICE9 device;
	LPDIRECT3DSWAPCHAIN9 swapChain;
};	// Context

// A single device will serve multiple windows
class GlobalDevice
{
public:
	GlobalDevice();
	~GlobalDevice();

	// Return -1 on error
	sal_checkreturn int createContext(
		size_t width, size_t height, 
		HWND wnd, bool fullScreen,
		int existing	// Negative value for no existing
	);

	void destroyContext(int index);

	void setAsRenderTarget(int index);

	void present(int index);

	void registerDefaultPoolTexture(Texture& texture);

	int currentContext;
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 device;
	D3DPRESENT_PARAMETERS params;
	std::vector<Context> contexts;
	std::vector<TextureWeakPtr> defaultPoolTextures;
};	// GlobalDevice

static GlobalDevice gGlobalDevice;

GlobalDevice::GlobalDevice()
	: currentContext(-1), d3d(nullptr), device(nullptr)
{
	ZeroMemory(&params, sizeof(params));
}

GlobalDevice::~GlobalDevice()
{
	MCD_ASSERT(!device);
	MCD_ASSERT(!d3d);
}

int GlobalDevice::createContext(size_t width, size_t height, HWND wnd, bool fullScreen, int existing)
{
	if(!d3d) {
		if(nullptr == (d3d = ::Direct3DCreate9(D3D_SDK_VERSION)))
			return -1;
	}

	if(existing > -1) {
		Context& c = contexts[existing];
		if(width == c.width && height == c.height)
			return existing;

		if(existing >= (int)contexts.size())
			return -1;

		SAFE_RELEASE(c.swapChain);
		c.width = width;
		c.height = height;
	}
	else {
		Context c = { wnd, width, height, device, nullptr };
		existing = contexts.size();
		contexts.push_back(c);
	}

	if(!device)
	{
		MCD_ASSERT(wnd);

		params.hDeviceWindow = wnd;
		params.Windowed = !fullScreen;
		params.BackBufferCount = 1;
		params.BackBufferWidth = width;
		params.BackBufferHeight = height;
		params.BackBufferFormat = D3DFMT_UNKNOWN;	// D3DFMT_UNKNOWN can be used for windowed mode
		params.EnableAutoDepthStencil = true;
		params.AutoDepthStencilFormat = D3DFMT_D16;
		params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		params.MultiSampleType = D3DMULTISAMPLE_NONE;

		if( FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&params, &device))
		)
			return -1;

		contexts[0].device = device;
		if(FAILED(device->GetSwapChain(0, &contexts[0].swapChain)))
			return -1;

		MCD_ASSERT(contexts[0].swapChain);
		return 0;
	}
	else if(width > params.BackBufferWidth || height > params.BackBufferHeight)
	{
		// Release all default pool textures first
		for(size_t i=0; i<defaultPoolTextures.size();) {
			if(!defaultPoolTextures[i])
				defaultPoolTextures.erase(defaultPoolTextures.begin() + i);
			else {
				if(IDirect3DTexture9*& texture = reinterpret_cast<IDirect3DTexture9*&>(defaultPoolTextures[i]->handle))
					SAFE_RELEASE(texture);
				++i;
			}
		}

		params.BackBufferWidth = width;
		params.BackBufferHeight = height;

		// Release all swap chain first
		for(size_t i=0; i<contexts.size(); ++i)
			SAFE_RELEASE(contexts[i].swapChain);

		if(FAILED(device->Reset(&params)))
			return -1;

		// TODO: Inform the Renderer about the device reset
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

		// Re-create the default pool textures
		// TODO: Handle mip-map, cube texture etc...
		for(size_t i=0; i<defaultPoolTextures.size(); ++i) {
			Texture* tex = defaultPoolTextures[i].get();
			MCD_VERIFY(tex->create(tex->format, tex->format, tex->width, tex->height, 1, 1, nullptr, 0, D3DUSAGE_RENDERTARGET));
		}
	}

	// Create swap chains if needed
	for(size_t i=0; i<contexts.size(); ++i)
	{
		Context& c = contexts[i];
		if(c.swapChain)
			continue;

		D3DPRESENT_PARAMETERS p = params;
		p.hDeviceWindow = c.wnd;
		p.BackBufferWidth = c.width;
		p.BackBufferHeight = c.height;
		if(FAILED(device->CreateAdditionalSwapChain(&p, &c.swapChain)))
			return -1;
	}

	return existing;
}

void GlobalDevice::destroyContext(int index)
{
	if(index == currentContext)
		currentContext = -1;

	SAFE_RELEASE(contexts[index].swapChain);
	contexts.erase(contexts.begin() + index);

	if(contexts.empty()) {
		SAFE_RELEASE(device);
		SAFE_RELEASE(d3d);
	}
}

void GlobalDevice::setAsRenderTarget(int index)
{
	// Set mSwapChain as the current render target.
	LPDIRECT3DSURFACE9 backSurface = nullptr;
	contexts[index].swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backSurface);
	device->SetRenderTarget(0, backSurface);
	SAFE_RELEASE(backSurface);
}

void GlobalDevice::present(int index)
{
	contexts[index].swapChain->Present(nullptr, nullptr, nullptr, nullptr, 0);
}

void GlobalDevice::registerDefaultPoolTexture(Texture& texture)
{
	std::vector<TextureWeakPtr>::const_iterator itr = std::find(defaultPoolTextures.begin(), defaultPoolTextures.end(), &texture);
	if(itr == defaultPoolTextures.end())
		defaultPoolTextures.push_back(&texture);
}

void registerDefaultPoolTexture(Texture& texture)
{
	gGlobalDevice.registerDefaultPoolTexture(texture);
}

class RenderWindow::Impl : public Window::Impl
{
public:
	typedef Window::Impl Super;

	Impl(Window& w) : Super(w), contextIndex(-1)
	{}

	sal_override ~Impl()
	{}

	void* renderContext()
	{
		return &gGlobalDevice.contexts[contextIndex];
	}

	sal_override void createWindow(Window::Handle existingWindowHandle=0)
	{
		MemoryProfiler::Scope profiler("RenderWindow::createWindow");
		Super::createWindow(existingWindowHandle);

		contextIndex = gGlobalDevice.createContext(mWidth, mHeight, mWnd, false, contextIndex);
		if(contextIndex < 0) {
			destroy();
			throw std::runtime_error("Fail to create render context");
		}
	}

	sal_override void destroy()
	{
		gGlobalDevice.destroyContext(contextIndex);
		contextIndex = -1;
		Super::destroy();
	}

	void onResize()
	{
		if(mWidth * mHeight == 0)
			return;
		contextIndex = gGlobalDevice.createContext(mWidth, mHeight, mWnd, false, contextIndex);
	}

	bool makeActive()
	{
		gGlobalDevice.currentContext = contextIndex;
		return true;
	}

	static void* getActiveContext()
	{
		return &gGlobalDevice.contexts[gGlobalDevice.currentContext];
	}

	void preUpdate()
	{
		gGlobalDevice.setAsRenderTarget(contextIndex);
	}

	void postUpdate()
	{
		gGlobalDevice.present(contextIndex);
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
	int contextIndex;
};	// Impl

}	// namespace MCD
