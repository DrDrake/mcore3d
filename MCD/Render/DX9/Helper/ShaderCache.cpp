#include "Pch.h"
#include "ShaderCache.h"
#include "../Renderer.inc"
#include "../../../Core/System/Log.h"
#include "../../../Core/System/Resource.h"
#include "../../../Core/System/ResourceLoader.h"
#include "../../../Core/System/ResourceManager.h"
#include "../../../Core/System/StringHash.h"
#include <D3DX9Shader.h>

namespace MCD {
namespace DX9Helper {

void ShaderCache::Vs::Release()
{
	SAFE_RELEASE(vs);
	SAFE_RELEASE(constTable);
}

void ShaderCache::Vs::AddRef()
{
	if(vs) vs->AddRef();
	if(constTable) constTable->AddRef();
}

void ShaderCache::Ps::Release()
{
	SAFE_RELEASE(ps);
	SAFE_RELEASE(constTable);
}

void ShaderCache::Ps::AddRef()
{
	if(ps) ps->AddRef();
	if(constTable) constTable->AddRef();
}

ShaderCache::~ShaderCache()
{
	clear();
}

ShaderCache& ShaderCache::singleton()
{
	static ShaderCache cache;
	return cache;
}

void ShaderCache::clear()
{
	for(VsMap::iterator i=mVsMap.begin(); i!=mVsMap.end(); ++i)
		i->second.Release();
	for(PsMap::iterator i=mPsMap.begin(); i!=mPsMap.end(); ++i)
		i->second.Release();
}

ShaderCache::Vs ShaderCache::getVertexShader(const char* sourceCode)
{
	const StringHash h(sourceCode, 0);
	const VsMap::iterator i = mVsMap.find(h.hash);
	if(i != mVsMap.end()) {
		i->second.AddRef();
		return i->second;
	}

	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	LPD3DXBUFFER vsBuf = nullptr;
	LPD3DXBUFFER errors = nullptr;

	Vs vs = { nullptr, nullptr };

	HRESULT result = D3DXCompileShader(
		sourceCode, strlen(sourceCode),
		nullptr, nullptr,	// Shader macro and include
		"main", "vs_3_0",
		0,	// flags
		&vsBuf, &errors,
		&vs.constTable	// Constant table
	);

	if(errors) {
		const char* msg = (const char*)errors->GetBufferPointer();
		Log::write(result == D3D_OK ? Log::Warn : Log::Error, msg);
		errors->Release();
	}

	if(result == D3D_OK) {
		device->CreateVertexShader((DWORD*)vsBuf->GetBufferPointer(), &vs.vs);
		vsBuf->Release();
		mVsMap[h] = vs;
	}

	vs.AddRef();
	return vs;
}

ShaderCache::Ps ShaderCache::getPixelShader(const char* sourceCode)
{
	const StringHash h(sourceCode, 0);
	const PsMap::iterator i = mPsMap.find(h.hash);
	if(i != mPsMap.end()) {
		i->second.AddRef();
		return i->second;
	}

	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	LPD3DXBUFFER psBuf = nullptr;
	LPD3DXBUFFER errors = nullptr;

	Ps ps = { nullptr, nullptr };

	HRESULT result = D3DXCompileShader(
		sourceCode, strlen(sourceCode),
		nullptr, nullptr,	// Shader macro and include
		"main", "ps_3_0",
		0,	// flags
		&psBuf, &errors,
		&ps.constTable	// Constant table
	);

	if(errors) {
		const char* msg = (const char*)errors->GetBufferPointer();
		Log::write(result == D3D_OK ? Log::Warn : Log::Error, msg);
		errors->Release();
	}

	if(result == D3D_OK) {
		device->CreatePixelShader((DWORD*)psBuf->GetBufferPointer(), &ps.ps);
		psBuf->Release();
		mPsMap[h] = ps;
	}

	ps.AddRef();
	return ps;
}

class DummyResource : public Resource
{
public:
	explicit DummyResource() : Resource("") {}
};	// DummyResource

// To ensure the graphics API call are invoked in main thread
class VsCommiter : public IResourceLoader
{
public:
	LoadingState load(std::istream* is, const Path* fileId, const char* args) { return Loaded; }
	sal_override void commit(Resource&)
	{
		MCD_ASSUME(vs);
		MCD_ASSUME(shaderCache);
		*vs = shaderCache->getVertexShader(sourceCode);
	}

	ShaderCache::Vs* vs;
	ShaderCache* shaderCache;
	const char* sourceCode;
};	// VsCommiter

class PsCommiter : public IResourceLoader
{
public:
	LoadingState load(std::istream* is, const Path* fileId, const char* args) { return Loaded; }
	sal_override void commit(Resource&)
	{
		MCD_ASSUME(ps);
		MCD_ASSUME(shaderCache);
		*ps = shaderCache->getPixelShader(sourceCode);
	}

	ShaderCache::Ps* ps;
	ShaderCache* shaderCache;
	const char* sourceCode;
};	// PsCommiter

ShaderCache::Vs ShaderCache::getVertexShader(const char* sourceCode, ResourceManager& mgr)
{
	Vs vs;
	VsCommiter* commiter = new VsCommiter;
	commiter->vs = &vs;
	commiter->shaderCache = this;
	commiter->sourceCode = sourceCode;
	IResourceLoaderPtr holder = commiter;
	ResourcePtr dummy = new DummyResource;
	mgr.customLoad(dummy, holder, 1);
	return vs;
}

ShaderCache::Ps ShaderCache::getPixelShader(const char* sourceCode, ResourceManager& mgr)
{
	Ps ps;
	PsCommiter* commiter = new PsCommiter;
	commiter->ps = &ps;
	commiter->shaderCache = this;
	commiter->sourceCode = sourceCode;
	IResourceLoaderPtr holder = commiter;
	ResourcePtr dummy = new DummyResource;
	mgr.customLoad(dummy, holder, 1);
	return ps;
}

}	// namespace DX9Helper
}	// namespace MCD
