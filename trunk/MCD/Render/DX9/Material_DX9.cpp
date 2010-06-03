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

	mConstantHandles.worldViewProj = mVsConstTable->GetConstantByName(nullptr, "mcdWorldViewProj");
	mConstantHandles.world = mVsConstTable->GetConstantByName(nullptr, "mcdWorld");
	mConstantHandles.lights = mPsConstTable->GetConstantByName(nullptr, "mcdLights");
	mConstantHandles.cameraPosition = mPsConstTable->GetConstantByName(nullptr, "mcdCameraPosition");
	mConstantHandles.specularExponent = mPsConstTable->GetConstantByName(nullptr, "mcdSpecularExponent");
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
	"	float2 uvDiffuse : TEXCOORD0;"
	"};"
	"struct VS_OUTPUT {"
	"	float4 position : POSITION;"
	"	float3 worldPosition : position1;"
	"	float3 normal : NORMAL;"
	"	float4 color : COLOR;"
	"	float2 uvDiffuse : TEXCOORD0;"
	"};"
	"VS_OUTPUT main(const VS_INPUT _in) {"
	"	VS_OUTPUT _out;"
	"	_out.position = mul(mcdWorldViewProj, float4(_in.position, 1));"
	"	_out.worldPosition = mul(mcdWorld, float4(_in.position, 1)).xyz;"
	"	float3x3 rotation = mcdWorld;"
	"	_out.normal = mul(rotation, _in.normal);"
	"	_out.uvDiffuse = _in.uvDiffuse;"
	"	_out.color = float4(1, 1, 1, 1);"
	"	return _out;"
	"}";

	LPD3DXBUFFER vsBuf = nullptr;
	LPD3DXBUFFER errors = nullptr;

	HRESULT result = D3DXCompileShader(
		code, sizeof(code),
		nullptr, nullptr,	// Shader macro and include
		"main", "vs_3_0",
		0,	// flags
		&vsBuf, &errors,
		&mVsConstTable	// Constant table
	);

	if(errors) {
		const char* msg = (const char*)errors->GetBufferPointer();
		Log::write(result == D3D_OK ? Log::Warn : Log::Error, msg);
		errors->Release();
	}

	if(result == D3D_OK) {
		device->CreateVertexShader((DWORD*)vsBuf->GetBufferPointer(), &mVs);
		vsBuf->Release();
		return true;
	}

	return false;
}

bool MaterialComponent::Impl::createPs()
{
	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());

	static const char code[] =
	"#define MCD_MAX_LIGHT_COUNT 4\n"
	"struct Light {"
	"	float3 position;"
	"	float4 color;"
	"} mcdLights[MCD_MAX_LIGHT_COUNT];"

	"float3 mcdCameraPosition;"
	"float mcdSpecularExponent;"
	"sampler2D texDiffuse;"

	"void computeLighting(in Light light, in float3 P, in float3 N, in float3 V, inout float3 diffuse, inout float3 specular) {"
	"	float3 L = light.position - P;"
	"	L = normalize(L);"
	"	float3 H = normalize(L + V);"
	"	float ndotl = saturate(dot(N, L));"
	"	float ndoth = saturate(dot(N, H));"
	"	float specExp = pow(ndoth, mcdSpecularExponent);"
	"	diffuse += ndotl * light.color.rgb;"
	"	specular += specExp * light.color.rgb;"
	"}"

	"struct PS_INPUT {"
	"	float3 position : POSITION;"
	"	float3 worldPosition : position1;"
	"	float3 normal : NORMAL;"
	"	float4 color : COLOR0;"
	"	float2 uvDiffuse : TEXCOORD0;"
	"};"
	"struct PS_OUTPUT { float4 color : COLOR; };"

	"PS_OUTPUT main(PS_INPUT _in) {"
	"	PS_OUTPUT _out = (PS_OUTPUT) 0;"
	"	float3 P = _in.worldPosition;"
	"	float3 N = normalize(_in.normal);"
	"	float3 V = normalize(mcdCameraPosition - P);"
	"	float3 lightDiffuse = 0;"
	"	float3 lightSpecular = 0;"
	"	for(int i=0; i<MCD_MAX_LIGHT_COUNT; ++i)"
	"		computeLighting(mcdLights[i], P, N, V, lightDiffuse, lightSpecular);\n"
	"	float3 diffuse = tex2D(texDiffuse, _in.uvDiffuse) * lightDiffuse;"
	"	float3 specular = lightSpecular;"
	"	_out.color.rgb = _in.color.rgb * diffuse + specular;"
	"	return _out;"
	"}";

	LPD3DXBUFFER psBuf = nullptr;
	LPD3DXBUFFER errors = nullptr;

	HRESULT result = D3DXCompileShader(
		code, sizeof(code),
		nullptr, nullptr,	// Shader macro and include
		"main", "ps_3_0",
		0,	// flags
		&psBuf, &errors,
		&mPsConstTable	// Constant table
	);

	if(errors) {
		const char* msg = (const char*)errors->GetBufferPointer();
		Log::write(result == D3D_OK ? Log::Warn : Log::Error, msg);
		errors->Release();
	}

	if(result == D3D_OK) {
		device->CreatePixelShader((DWORD*)psBuf->GetBufferPointer(), &mPs);
		psBuf->Release();
		return true;
	}

	return false;
}

MaterialComponent::MaterialComponent()
	: mImpl(*new Impl)
	, specularExponent(20)
{
}

MaterialComponent::~MaterialComponent()
{
	delete &mImpl;
}

}	// namespace MCD
