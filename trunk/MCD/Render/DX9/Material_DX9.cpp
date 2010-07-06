#include "Pch.h"
#include "Material_DX9.inc"
#include "Renderer.inc"
#include "../Light.h"
#include "../RenderWindow.h"
#include "../../Core/System/Log.h"
#include "../../Core/Entity/Entity.h"
#include <D3DX9Shader.h>

namespace MCD {

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
	mConstantHandles.diffuseColor = mPsConstTable->GetConstantByName(nullptr, "mcdDiffuseColor");
	mConstantHandles.specularColor = mPsConstTable->GetConstantByName(nullptr, "mcdSpecularColor");
	mConstantHandles.emissionColor = mPsConstTable->GetConstantByName(nullptr, "mcdEmissionColor");
	mConstantHandles.specularExponent = mPsConstTable->GetConstantByName(nullptr, "mcdSpecularExponent");
	mConstantHandles.opacity = mPsConstTable->GetConstantByName(nullptr, "mcdOpacity");
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
	LPDIRECT3DDEVICE9 device = getDevice();

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
	LPDIRECT3DDEVICE9 device = getDevice();

	static const char code[] =
	"#define MCD_MAX_LIGHT_COUNT 4\n"
	"struct Light {"
	"	float3 position;"
	"	float4 color;"
	"} mcdLights[MCD_MAX_LIGHT_COUNT];"

	"float3 mcdCameraPosition;"
	"float4 mcdDiffuseColor;"
	"float4 mcdSpecularColor;"
	"float4 mcdEmissionColor;"
	"float mcdSpecularExponent;"
	"float mcdOpacity;"
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
	"	float4 diffuseMap = tex2D(texDiffuse, _in.uvDiffuse);"
	"	float3 diffuse = mcdDiffuseColor.rgb * mcdDiffuseColor.a * tex2D(texDiffuse, _in.uvDiffuse).rgb * lightDiffuse;"
	"	float3 specular = mcdSpecularColor.rgb * mcdSpecularColor.a * lightSpecular;"
	"	float3 emission = mcdEmissionColor.rgb * mcdEmissionColor.a;"
	"	_out.color.rgb = _in.color.rgb * diffuse + specular + emission;"
	"	_out.color.a = mcdOpacity * diffuseMap.a;"
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

void MaterialComponent::Impl::updateWorldTransform(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	LPDIRECT3DDEVICE9 device = getDevice();

	MCD_VERIFY(mVsConstTable->SetMatrix(
		device, mConstantHandles.worldViewProj, (D3DXMATRIX*)renderer.mWorldViewProjMatrix.getPtr()
	) == S_OK);

	MCD_VERIFY(mVsConstTable->SetMatrix(
		device, mConstantHandles.world, (D3DXMATRIX*)renderer.mWorldMatrix.getPtr()
	) == S_OK);
}

void MaterialComponent::render2(void* context)
{
	// Push light into Renderer's light list
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;
}

void MaterialComponent::preRender(size_t pass, void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	LPDIRECT3DDEVICE9 device = getDevice();

	{	// Bind system information
		MCD_VERIFY(mImpl.mVsConstTable->SetMatrix(
			device, mImpl.mConstantHandles.worldViewProj, (D3DXMATRIX*)renderer.mWorldViewProjMatrix.getPtr()
		) == S_OK);

		MCD_VERIFY(mImpl.mVsConstTable->SetMatrix(
			device, mImpl.mConstantHandles.world, (D3DXMATRIX*)renderer.mWorldMatrix.getPtr()
		) == S_OK);

		Vec3f cameraPosition = renderer.mCameraTransform.translation();
		MCD_VERIFY(mImpl.mVsConstTable->SetFloatArray(
			device, mImpl.mConstantHandles.cameraPosition, cameraPosition.getPtr(), 3
		) == S_OK);
	}

	{	// Bind light information
		// To match the light data structure in the shader
		struct LightStruct
		{
			Vec3f position;
			ColorRGBAf color;
		} lightStruct;

		for(size_t i=0; i<4; ++i) {
			D3DXHANDLE hi = mImpl.mPsConstTable->GetConstantElement(mImpl.mConstantHandles.lights, i);

			memset(&lightStruct, 0, sizeof(lightStruct));

			if(i < renderer.mLights.size()) {
				LightComponent* light = renderer.mLights[i];
				if(Entity* e = light->entity())
					lightStruct.position = e->worldTransform().translation();
				lightStruct.color = ColorRGBAf(light->color, 1);
			}

			MCD_VERIFY(mImpl.mVsConstTable->SetFloatArray(
				device, hi, (float*)&lightStruct, sizeof(lightStruct) / sizeof(float)
			) == S_OK);
		}
	}

	{	// Bind material information
		MCD_VERIFY(mImpl.mVsConstTable->SetFloatArray(
			device, mImpl.mConstantHandles.diffuseColor, diffuseColor.rawPointer(), 4
		) == S_OK);

		MCD_VERIFY(mImpl.mVsConstTable->SetFloatArray(
			device, mImpl.mConstantHandles.specularColor, specularColor.rawPointer(), 4
		) == S_OK);

		MCD_VERIFY(mImpl.mVsConstTable->SetFloatArray(
			device, mImpl.mConstantHandles.emissionColor, emissionColor.rawPointer(), 4
		) == S_OK);

		MCD_VERIFY(mImpl.mVsConstTable->SetFloat(
			device, mImpl.mConstantHandles.specularExponent, specularExponent
		) == S_OK);

		MCD_VERIFY(mImpl.mVsConstTable->SetFloat(
			device, mImpl.mConstantHandles.opacity, opacity
		) == S_OK);

		if(TexturePtr diffuse = diffuseMap ? diffuseMap : renderer.mWhiteTexture) {
			if(IDirect3DBaseTexture9* texture = reinterpret_cast<IDirect3DBaseTexture9*>(diffuse->handle)) {
				unsigned samplerIndex = mImpl.mPsConstTable->GetSamplerIndex("texDiffuse");
				device->SetTexture(samplerIndex, texture);
			}
		}
	}

	device->SetVertexShader(mImpl.mVs);
	device->SetPixelShader(mImpl.mPs);
}

void MaterialComponent::postRender(size_t pass, void* context) {}

MaterialComponent::MaterialComponent()
	: mImpl(*new Impl)
	, diffuseColor(1, 1)
	, specularColor(1, 1)
	, emissionColor(0, 1)
	, specularExponent(20)
	, opacity(1)
{}

MaterialComponent::~MaterialComponent()
{
	delete &mImpl;
}

void SpriteMaterialComponent::render2(void* context)
{
	// Push light into Renderer's light list
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;
}

void SpriteMaterialComponent::preRender(size_t pass, void* context)
{
	LPDIRECT3DDEVICE9 device = getDevice();

	device->SetVertexShader(nullptr);
	device->SetPixelShader(nullptr);

	if(diffuseMap) {
		if(IDirect3DBaseTexture9* texture = reinterpret_cast<IDirect3DBaseTexture9*>(diffuseMap->handle))
			MCD_VERIFY(device->SetTexture(0, texture) == D3D_OK);
	}
}

void SpriteMaterialComponent::postRender(size_t pass, void* context) {}

}	// namespace MCD
