#include "Pch.h"
#include "Material_DX9.inc"
#include "Renderer.inc"
#include "Helper/ShaderCache.h"
#include "../Light.h"
#include "../RenderWindow.h"
#include "../../Core/System/Log.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/Entity/SystemComponent.h"
#include <D3DX9Shader.h>

namespace MCD {

using namespace DX9Helper;

MaterialComponent::Impl::Impl() 
{
	MCD_VERIFY(createVs());
	MCD_VERIFY(createPs());

	mConstantHandles.worldViewProj = mVs.constTable->GetConstantByName(nullptr, "mcdWorldViewProj");
	mConstantHandles.world = mVs.constTable->GetConstantByName(nullptr, "mcdWorld");
	mConstantHandles.lights = mPs.constTable->GetConstantByName(nullptr, "mcdLights");
	mConstantHandles.cameraPosition = mPs.constTable->GetConstantByName(nullptr, "mcdCameraPosition");
	mConstantHandles.diffuseColor = mPs.constTable->GetConstantByName(nullptr, "mcdDiffuseColor");
	mConstantHandles.specularColor = mPs.constTable->GetConstantByName(nullptr, "mcdSpecularColor");
	mConstantHandles.emissionColor = mPs.constTable->GetConstantByName(nullptr, "mcdEmissionColor");
	mConstantHandles.specularExponent = mPs.constTable->GetConstantByName(nullptr, "mcdSpecularExponent");
	mConstantHandles.opacity = mPs.constTable->GetConstantByName(nullptr, "mcdOpacity");
}

MaterialComponent::Impl::~Impl()
{
	mVs.Release();
	mPs.Release();
}

bool MaterialComponent::Impl::createVs()
{
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

	if(ResourceManagerComponent* c = ResourceManagerComponent::fromCurrentEntityRoot())
		mVs = ShaderCache::singleton().getVertexShader(code, c->resourceManager());
	else
		mVs = ShaderCache::singleton().getVertexShader(code);
	return mVs.vs && mVs.constTable;
}

bool MaterialComponent::Impl::createPs()
{
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

	if(ResourceManagerComponent* c = ResourceManagerComponent::fromCurrentEntityRoot())
		mPs = ShaderCache::singleton().getPixelShader(code, c->resourceManager());
	else
		mPs = ShaderCache::singleton().getPixelShader(code);
	return mPs.ps && mPs.constTable;
}

void MaterialComponent::Impl::updateWorldTransform(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	MCD_VERIFY(mVs.constTable->SetMatrix(
		device, mConstantHandles.worldViewProj, (D3DXMATRIX*)renderer.mWorldViewProjMatrix.getPtr()
	) == S_OK);

	MCD_VERIFY(mVs.constTable->SetMatrix(
		device, mConstantHandles.world, (D3DXMATRIX*)renderer.mWorldMatrix.getPtr()
	) == S_OK);
}

void MaterialComponent::render(void* context)
{
	// Push light into Renderer's light list
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;
}

void MaterialComponent::preRender(size_t pass, void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	{	// Bind system information
		MCD_VERIFY(mImpl.mVs.constTable->SetMatrix(
			device, mImpl.mConstantHandles.worldViewProj, (D3DXMATRIX*)renderer.mWorldViewProjMatrix.getPtr()
		) == S_OK);

		MCD_VERIFY(mImpl.mVs.constTable->SetMatrix(
			device, mImpl.mConstantHandles.world, (D3DXMATRIX*)renderer.mWorldMatrix.getPtr()
		) == S_OK);

		Vec3f cameraPosition = renderer.mCameraTransform.translation();
		MCD_VERIFY(mImpl.mVs.constTable->SetFloatArray(
			device, mImpl.mConstantHandles.cameraPosition, cameraPosition.getPtr(), 3
		) == S_OK);
	}

	// Material state change early out optimization
	if(renderer.mLastMaterial == this)
		return;

	{	// Bind light information
		// To match the light data structure in the shader
		struct LightStruct
		{
			Vec3f position;
			ColorRGBAf color;
		} lightStruct;

		for(size_t i=0; i<4; ++i) {
			D3DXHANDLE hi = mImpl.mPs.constTable->GetConstantElement(mImpl.mConstantHandles.lights, i);

			memset(&lightStruct, 0, sizeof(lightStruct));

			if(i < renderer.mLights.size()) {
				LightComponent* light = renderer.mLights[i];
				if(Entity* e = light->entity())
					lightStruct.position = e->worldTransform().translation();
				lightStruct.color = ColorRGBAf(light->color, 1);
			}

			MCD_VERIFY(mImpl.mVs.constTable->SetFloatArray(
				device, hi, (float*)&lightStruct, sizeof(lightStruct) / sizeof(float)
			) == S_OK);
		}
	}

	{	// Bind material information
		MCD_VERIFY(mImpl.mVs.constTable->SetFloatArray(
			device, mImpl.mConstantHandles.diffuseColor, diffuseColor.rawPointer(), 4
		) == S_OK);

		MCD_VERIFY(mImpl.mVs.constTable->SetFloatArray(
			device, mImpl.mConstantHandles.specularColor, specularColor.rawPointer(), 4
		) == S_OK);

		MCD_VERIFY(mImpl.mVs.constTable->SetFloatArray(
			device, mImpl.mConstantHandles.emissionColor, emissionColor.rawPointer(), 4
		) == S_OK);

		MCD_VERIFY(mImpl.mVs.constTable->SetFloat(
			device, mImpl.mConstantHandles.specularExponent, specularExponent
		) == S_OK);

		MCD_VERIFY(mImpl.mVs.constTable->SetFloat(
			device, mImpl.mConstantHandles.opacity, opacity
		) == S_OK);

		if(TexturePtr diffuse = diffuseMap ? diffuseMap : renderer.mWhiteTexture)
			diffuse->bind(mImpl.mPs.constTable->GetSamplerIndex("texDiffuse"));
	}

	device->SetVertexShader(mImpl.mVs.vs);
	device->SetPixelShader(mImpl.mPs.ps);
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

void SpriteMaterialComponent::render(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;
}

void SpriteMaterialComponent::preRender(size_t pass, void* context)
{
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	device->SetVertexShader(nullptr);
	device->SetPixelShader(nullptr);

	if(diffuseMap)
		diffuseMap->bind(0);
}

void SpriteMaterialComponent::postRender(size_t pass, void* context) {}

}	// namespace MCD
