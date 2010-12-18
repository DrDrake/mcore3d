// This inc file is to be included by RenderWindow.cpp only

#include "Helper/ShaderCache.h"
#include "../Texture.h"
#include "../../Core/System/StrUtility.h"
#include "../../Core/System/MemoryProfiler.h"
#include "../../Core/System/Utility.h"
#include "../../Core/System/Window.Win.inl"
#include <d3d9.h>
#include <list>

#pragma comment(lib, "d3d9")
#pragma comment(lib, "dxguid")

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
		iterator existing,
		size_t preferedMultiSample
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

static bool isMSAAModeSupported(D3DMULTISAMPLE_TYPE type, D3DFORMAT backBufferFmt, D3DFORMAT depthStencilFmt, BOOL windowed, DWORD &qualityLevels)
{
	DWORD backBufferQualityLevels = 0;
	DWORD depthStencilQualityLevels = 0;

	HRESULT hr = gGlobalDevice.d3d->CheckDeviceMultiSampleType(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, backBufferFmt, windowed, type, &backBufferQualityLevels
	);

	if(!SUCCEEDED(hr)) return false;

	hr = gGlobalDevice.d3d->CheckDeviceMultiSampleType(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, depthStencilFmt, windowed, type, &depthStencilQualityLevels
	);

	if(!SUCCEEDED(hr)) return false;
	if(backBufferQualityLevels != depthStencilQualityLevels) return false;

	// The valid range is between zero and one less than the level
	// returned by IDirect3D9::CheckDeviceMultiSampleType().
	if (backBufferQualityLevels > 0)
		qualityLevels = backBufferQualityLevels - 1;
	else
		qualityLevels = backBufferQualityLevels;

	return true;
}

static void chooseBestMSAAMode(D3DFORMAT backBufferFmt, D3DFORMAT depthStencilFmt, BOOL windowed, D3DMULTISAMPLE_TYPE& type, DWORD &qualityLevels, size_t& perferedSamplesPerPixel)
{
	bool supported = false;

	struct MSAAMode
	{
		D3DMULTISAMPLE_TYPE type;
		DWORD samples;
	};

	MSAAMode multsamplingTypes[] =
	{
		{ D3DMULTISAMPLE_16_SAMPLES,  16 },
		{ D3DMULTISAMPLE_15_SAMPLES,  15 },
		{ D3DMULTISAMPLE_14_SAMPLES,  14 },
		{ D3DMULTISAMPLE_13_SAMPLES,  13 },
		{ D3DMULTISAMPLE_12_SAMPLES,  12 },
		{ D3DMULTISAMPLE_11_SAMPLES,  11 },
		{ D3DMULTISAMPLE_10_SAMPLES,  10 },
		{ D3DMULTISAMPLE_9_SAMPLES,   9  },
		{ D3DMULTISAMPLE_8_SAMPLES,   8  },
		{ D3DMULTISAMPLE_7_SAMPLES,   7  },
		{ D3DMULTISAMPLE_6_SAMPLES,   6  },
		{ D3DMULTISAMPLE_5_SAMPLES,   5  },
		{ D3DMULTISAMPLE_4_SAMPLES,   4  },
		{ D3DMULTISAMPLE_3_SAMPLES,   3  },
		{ D3DMULTISAMPLE_2_SAMPLES,   2  }
	};

	for(int i = 0; i < MCD_COUNTOF(multsamplingTypes); ++i) {
		if(multsamplingTypes[i].samples > perferedSamplesPerPixel)
			continue;

		type = multsamplingTypes[i].type;
		supported = isMSAAModeSupported(type, backBufferFmt, depthStencilFmt, windowed, qualityLevels);

		if(supported) {
			perferedSamplesPerPixel = multsamplingTypes[i].samples;
			return;
		}
	}

	type = D3DMULTISAMPLE_NONE;
	qualityLevels = 0;
	perferedSamplesPerPixel = 1;
}


GlobalDevice::iterator GlobalDevice::createContext(size_t width, size_t height, HWND wnd, bool fullScreen, iterator existing, size_t preferedMultiSample)
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

		// Get the current desktop display mode as the default
		D3DDISPLAYMODE desktop = {0};
		d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &desktop);

		params.hDeviceWindow = wnd;
		params.Windowed = !fullScreen;
		params.BackBufferCount = 1;
		params.BackBufferWidth = width;
		params.BackBufferHeight = height;
		params.BackBufferFormat =  desktop.Format;
		params.EnableAutoDepthStencil = true;
		params.AutoDepthStencilFormat = D3DFMT_D16;
		params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		params.MultiSampleType = D3DMULTISAMPLE_NONE;
		params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	// Turn off v-sync

		// Select the highest quality multi-sample anti-aliasing (MSAA) mode.
		chooseBestMSAAMode(
			params.BackBufferFormat, params.AutoDepthStencilFormat,
			params.Windowed, params.MultiSampleType, params.MultiSampleQuality,
			preferedMultiSample
		);

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
			if(Texture* tex = defaultPoolTextures[i].get())
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
		DX9Helper::ShaderCache::singleton().clear();
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

	sal_override bool createWindow(Window::Handle existingWindowHandle=0)
	{
		MemoryProfiler::Scope profiler("RenderWindow::createWindow");
		if(!Super::createWindow(existingWindowHandle))
			return false;

		contextItr = gGlobalDevice.createContext(mWidth, mHeight, mWnd, false, contextItr, mMultiSampleLevel);
		if(contextItr == gGlobalDevice.contexts.end()) {
			destroy();
			Log::write(Log::Warn, "Fail to create render context");
			return false;
		}

		return true;
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
		contextItr = gGlobalDevice.createContext(mWidth, mHeight, mWnd, false, contextItr, mMultiSampleLevel);
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
		if(::strcmp(name, "FSAA") == 0)
			mMultiSampleLevel = str2IntWithDefault(value, 0);
		else
			Super::setOption(name, value);
	}

private:
	uint mMultiSampleLevel;
	GlobalDevice::iterator contextItr;
};	// Impl

}	// namespace MCD
