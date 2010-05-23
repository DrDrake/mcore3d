#include "Pch.h"
#include "../Material.h"
#include "../RenderWindow.h"
#include "../../Core/System/Log.h"

#include "../Camera.h"
#include "../../Core/Math/Mat44.h"

#include <d3d9.h>
#include <D3DX9Shader.h>

#include "Material_DX9.inc"

namespace MCD {

template<typename T> void SAFE_RELEASE(T& p)
{
	if(p) {
		p->Release();
		p = nullptr;
	}
}

MaterialComponent::Impl::Impl() 
	: mVs(nullptr), mPs(nullptr)
	, mVsConstTable(nullptr), mPsConstTable(nullptr)
{
	MCD_VERIFY(createVs());
	MCD_VERIFY(createPs());
}

MaterialComponent::Impl::~Impl()
{
	SAFE_RELEASE(mVs);
	SAFE_RELEASE(mPs);
	SAFE_RELEASE(mVsConstTable);
	SAFE_RELEASE(mPsConstTable);
}

bool MaterialComponent::Impl::createVs()
{
	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());

	static const char code[] =
	"float4x4 mcdWorld;"
	"float4x4 mcdWorldViewProj;"
	"struct VS_INPUT {"
	"	float3 position : POSITION;"
	"	float3 normal : NORMAL;"
	"};"
	"struct VS_OUTPUT {"
	"	float4 position : POSITION;"
	"	float3 normal : NORMAL;"
	"	float4 color : COLOR;"
	"};"
	"VS_OUTPUT main(const VS_INPUT _in) {"
	"	VS_OUTPUT _out;"
	"	_out.position = mul(mcdWorldViewProj, float4(_in.position, 1));"
	"	_out.normal = mul(mcdWorld, _in.normal);"
	"	_out.color = float4(1, 1, 1, 1);"
	"	return _out;"
	"}";

	LPD3DXBUFFER vsBuf = nullptr;
	LPD3DXBUFFER errors = nullptr;

	if(D3D_OK != D3DXCompileShader(
		code, sizeof(code),
		nullptr, nullptr,	// Shader macro and include
		"main", "vs_3_0",
		0,	// flags
		&vsBuf, &errors,
		&mVsConstTable	// Constant table
	))
	{
		const char* msg = (const char*)errors->GetBufferPointer();
		msg = msg;
		errors->Release();
		return false;
	}

	device->CreateVertexShader((DWORD*)vsBuf->GetBufferPointer(), &mVs);
	vsBuf->Release();
	return true;
}

bool MaterialComponent::Impl::createPs()
{
	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());

	static const char code[] =
	"struct PS_INPUT {"
	"	float3 position : POSITION;"
	"	float3 normal : NORMAL;"
	"	float4 color : COLOR0;"
	"};"
	"struct PS_OUTPUT { float4 color : COLOR; };"
	"PS_OUTPUT main(PS_INPUT _in) {"
	"	PS_OUTPUT _out = (PS_OUTPUT) 0;"
	"	_out.color = _in.color;"
	"	return _out;"
	"}";

	LPD3DXBUFFER psBuf = nullptr;
	LPD3DXBUFFER errors = nullptr;

	if(D3D_OK != D3DXCompileShader(
		code, sizeof(code),
		nullptr, nullptr,	// Shader macro and include
		"main", "ps_3_0",
		0,	// flags
		&psBuf, &errors,
		&mPsConstTable	// Constant table
	))
	{
		const char* msg = (const char*)errors->GetBufferPointer();
		msg = msg;
		errors->Release();
		return false;
	}

	device->CreatePixelShader((DWORD*)psBuf->GetBufferPointer(), &mPs);
	psBuf->Release();
	return true;
}

MaterialComponent::MaterialComponent()
	: mImpl(*new Impl)
{
}

MaterialComponent::~MaterialComponent()
{
	delete &mImpl;
}

}	// namespace MCD
