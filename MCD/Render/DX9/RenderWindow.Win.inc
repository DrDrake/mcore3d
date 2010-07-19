// This inc file is to be included by RenderWindow.cpp only

#include "../Texture.h"
#include "../../Core/System/StrUtility.h"
#include "../../Core/System/MemoryProfiler.h"
#include "../../Core/System/Utility.h"
#include "../../Core/System/Window.Win.inl"
#include <d3d9.h>
#include <list>

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
	typedef std::list<Context>::iterator iterator;

	GlobalDevice();
	~GlobalDevice();

	// Return -1 on error
	sal_checkreturn iterator createContext(
		size_t width, size_t height, 
		HWND wnd, bool fullScreen,
		iterator existing
	);

	void destroyContext(iterator& itr);

	void setAsRenderTarget(const iterator& itr);

	void present(const iterator& itr);

	void registerDefaultPoolTexture(Texture& texture);

	iterator currentContext;

	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 device;
	D3DPRESENT_PARAMETERS params;
	std::list<Context> contexts;
	std::vector<TextureWeakPtr> defaultPoolTextures;
};	// GlobalDevice

static GlobalDevice gGlobalDevice;

GlobalDevice::GlobalDevice()
	: d3d(nullptr), device(nullptr)
{
	currentContext = contexts.end();
	ZeroMemory(&params, sizeof(params));
}

GlobalDevice::~GlobalDevice()
{
	MCD_ASSERT(!device);
	MCD_ASSERT(!d3d);
}

GlobalDevice::iterator GlobalDevice::createContext(size_t width, size_t height, HWND wnd, bool fullScreen, iterator existing)
{
	if(!d3d) {
		if(nullptr == (d3d = ::Direct3DCreate9(D3D_SDK_VERSION)))
			return contexts.end();
	}

	if(existing != contexts.end()) {
		Context& c = *existing;
		if(width == c.width && height == c.height)
			return existing;

		SAFE_RELEASE(c.swapChain);
		c.width = width;
		c.height = height;
	}
	else {
		Context c = { wnd, width, height, device, nullptr };
		contexts.push_back(c);
		existing = contexts.begin();
		std::advance(existing, contexts.size()-1);
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
		params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	// Turn off v-sync

		if( FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&params, &device))
		)
			return contexts.end();

		existing->device = device;
		if(FAILED(device->GetSwapChain(0, &existing->swapChain)))
			return contexts.end();

		MCD_ASSERT(existing->swapChain);
		return existing;
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

		// Release all swap chain also
		MCD_FOREACH(const Context& c, contexts)
			SAFE_RELEASE(const_cast<Context&>(c).swapChain);

		params.BackBufferWidth = width;
		params.BackBufferHeight = height;

		// NOTE: After device reset, all render state should be re-applied.
		if(FAILED(device->Reset(&params)))
			return contexts.end();

		// Re-create the default pool textures
		// TODO: Handle mip-map, cube texture etc...
		for(size_t i=0; i<defaultPoolTextures.size(); ++i) {
			Texture* tex = defaultPoolTextures[i].get();
			MCD_VERIFY(tex->create(tex->format, tex->format, tex->width, tex->height, 1, 1, nullptr, 0, D3DUSAGE_RENDERTARGET));
		}
	}

	// Create swap chains if needed
	MCD_FOREACH(const Context& c_, contexts)
	{
		Context& c = const_cast<Context&>(c_);
		if(c.swapChain)
			continue;

		D3DPRESENT_PARAMETERS p = params;
		p.hDeviceWindow = c.wnd;
		p.BackBufferWidth = c.width;
		p.BackBufferHeight = c.height;
		if(FAILED(device->CreateAdditionalSwapChain(&p, &c.swapChain)))
			return contexts.end();
	}

	return existing;
}

void GlobalDevice::destroyContext(iterator& itr)
{
	if(itr == currentContext)
		currentContext = contexts.end();

	SAFE_RELEASE(itr->swapChain);
	contexts.erase(itr);
	itr = contexts.end();

	if(contexts.empty()) {
		SAFE_RELEASE(device);
		SAFE_RELEASE(d3d);
	}
}

void GlobalDevice::setAsRenderTarget(const iterator& itr)
{
	// Set mSwapChain as the current render target.
	LPDIRECT3DSURFACE9 backSurface = nullptr;
	itr->swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backSurface);
	device->SetRenderTarget(0, backSurface);
	SAFE_RELEASE(backSurface);
}

void GlobalDevice::present(const iterator& itr)
{
	itr->swapChain->Present(nullptr, nullptr, nullptr, nullptr, 0);
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

	Impl(Window& w) : Super(w), contextItr(gGlobalDevice.contexts.end())
	{}

	sal_override ~Impl()
	{}

	void* renderContext()
	{
		return &*contextItr;
	}

	sal_override void createWindow(Window::Handle existingWindowHandle=0)
	{
		MemoryProfiler::Scope profiler("RenderWindow::createWindow");
		Super::createWindow(existingWindowHandle);

		contextItr = gGlobalDevice.createContext(mWidth, mHeight, mWnd, false, contextItr);
		if(contextItr == gGlobalDevice.contexts.end()) {
			destroy();
			throw std::runtime_error("Fail to create render context");
		}
	}

	sal_override void destroy()
	{
		gGlobalDevice.destroyContext(contextItr);
		Super::destroy();
	}

	void onResize()
	{
		if(mWidth * mHeight == 0)
			return;
		contextItr = gGlobalDevice.createContext(mWidth, mHeight, mWnd, false, contextItr);
	}

	bool makeActive()
	{
		gGlobalDevice.currentContext = contextItr;
		return true;
	}

	static void* getActiveContext()
	{
		return &*gGlobalDevice.currentContext;
	}

	void preUpdate()
	{
		gGlobalDevice.setAsRenderTarget(contextItr);
	}

	void postUpdate()
	{
		gGlobalDevice.present(contextItr);
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
	GlobalDevice::iterator contextItr;
};	// Impl

}	// namespace MCD
