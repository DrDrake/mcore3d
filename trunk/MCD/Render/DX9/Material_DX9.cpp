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
}

MaterialComponent::Impl::~Impl()
{
	mVs.Release();
	mPs.Release();
}

bool MaterialComponent::Impl::createVs(const char* headerCode)
{
	static const char code[] =
	"float4x4 mcdWorld;\n"
	"float4x4 mcdWorldViewProj;\n"
	"struct VS_INPUT {\n"
	"	float3 position : POSITION;\n"
	"	float3 normal : NORMAL;\n"
	"#if USE_VERTEX_COLOR\n"
	"	float4 color : COLOR0;\n"
	"#endif\n"
	"	float2 uvDiffuse : TEXCOORD0;\n"
	"};\n"
	"struct VS_OUTPUT {\n"
	"	float4 position : POSITION;\n"
	"	float3 worldPosition : position1;\n"
	"	float3 normal : NORMAL;\n"
	"#if USE_VERTEX_COLOR\n"
	"	float4 color : COLOR0;\n"
	"#endif\n"
	"	float2 uvDiffuse : TEXCOORD0;\n"
	"};\n"
	"VS_OUTPUT main(const VS_INPUT _in) {\n"
	"	VS_OUTPUT _out;\n"
	"	_out.position = mul(mcdWorldViewProj, float4(_in.position, 1));\n"
	"	_out.worldPosition = mul(mcdWorld, float4(_in.position, 1)).xyz;\n"
	"	float3x3 rotation = mcdWorld;\n"
	"	_out.normal = mul(rotation, _in.normal);\n"
	"	_out.uvDiffuse = _in.uvDiffuse;\n"
	"#if USE_VERTEX_COLOR\n"
	"	_out.color = _in.color;\n"
	"#endif\n"
	"	return _out;\n"
	"}\n";

	if(mVs.vs) mVs.Release();

	if(ResourceManagerComponent* c = ResourceManagerComponent::fromCurrentEntityRoot())
		mVs = ShaderCache::singleton().getVertexShader(code, headerCode, c->resourceManager());
	else
		mVs = ShaderCache::singleton().getVertexShader(code);

	if(!mVs.vs || !mVs.constTable) return false;

	mConstantHandles.worldViewProj = mVs.constTable->GetConstantByName(nullptr, "mcdWorldViewProj");
	mConstantHandles.world = mVs.constTable->GetConstantByName(nullptr, "mcdWorld");
	return true;
}

bool MaterialComponent::Impl::createPs(const char* headerCode)
{
	static const char code[] =
	"#define MCD_MAX_LIGHT_COUNT 4\n"
	"struct Light {\n"
	"	float3 position;\n"
	"	float4 color;\n"
	"} mcdLights[MCD_MAX_LIGHT_COUNT];\n"

	"float3 mcdCameraPosition;\n"
	"float4 mcdDiffuseColor;\n"
	"float4 mcdSpecularColor;\n"
	"float4 mcdEmissionColor;\n"
	"float mcdSpecularExponent;\n"
	"float mcdOpacity;\n"
	"bool mcdLighting;\n"
	"sampler2D texDiffuse;\n"
	"#if USE_BUMP_MAP\n"
	"sampler2D texBump;\n"
	"#endif\n"

	"void computeLighting(in Light light, in float3 P, in float3 N, in float3 V, inout float3 diffuse, inout float3 specular) {\n"
	"	float3 L = light.position - P;\n"
	"	L = normalize(L);\n"
	"	float3 H = normalize(L + V);\n"
	"	float ndotl = saturate(dot(N, L));\n"
	"	float ndoth = saturate(dot(N, H));\n"
	"	float specExp = pow(ndoth, mcdSpecularExponent);\n"
	"	diffuse += ndotl * light.color.rgb;\n"
	"	specular += specExp * light.color.rgb;\n"
	"}\n"

	// http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter20.html
//	"float4 bspline(in float2 uv, in sampler2D tex, in float2 texSize) {\n"
//	"	float2 coord_hg = uv * texSize - float2(0.5f, 0.5f);\n"
//	"	 float3 hg_x = tex1D(tex, coord_hg.x ).xyz;\n"
//	"}\n"

	// Reference: Bump Mapping Unparametrized Surfaces on the GPU
	// http://jbit.net/~sparky/sfgrad_bump/mm_sfgrad_bump.pdf
	// P: world position, N: interpolated normal, uv: bump map texture uv, map: the bump map
	"float3 computeBump(in float3 P, in float3 N, in float2 uv, in sampler2D map, in float bumpFactor) {\n"
	"	float3 vSigmaS = ddx(P);"
	"	float3 vSigmaT = ddy(P);"
	"	float3 vR1 = cross(vSigmaT, N);"
	"	float3 vR2 = cross(N, vSigmaS);"
	"	float fDet = dot(vSigmaS, vR1);"
	"	\n"
//	"	float dBs = ddx(tex2D(map, uv).x);"
//	"	float dBt = ddy(tex2D(map, uv).x);"
	"	float2 TexDx = ddx(uv);"
	"	float2 TexDy = ddy(uv);"
	"	float2 STll = uv;"
	"	float2 STlr = uv + TexDx;"
	"	float2 STul = uv + TexDy;"
	"	float Hll = tex2D(map, STll).x;"
	"	float Hlr = tex2D(map, STlr).x;"
	"	float Hul = tex2D(map, STul).x;"
/*	"	STlr = uv + TexDx * 4;"
	"	STul = uv + TexDy * 4;"
	"	Hlr += tex2D(map, STlr).x * 0.4;"
	"	Hul += tex2D(map, STul).x * 0.4;"
	"	STlr = uv + TexDx * 8;"
	"	STul = uv + TexDy * 8;"
	"	Hlr += tex2D(map, STlr).x * 0.2;"
	"	Hul += tex2D(map, STul).x * 0.2;"
	"	Hlr /= 1.7;"
	"	Hul /= 1.7;"*/
	"	float dBs = Hlr - Hll;"
	"	float dBt = Hul - Hll;"
	"	\n"
	"	float3 vSurfGrad = sign(fDet) * (dBs * vR1 + dBt * vR2);\n"
	"	return normalize(abs(fDet) * N - bumpFactor * vSurfGrad);\n"
	"}\n"

	"struct PS_INPUT {\n"
	"	float3 position : POSITION;\n"
	"	float3 worldPosition : position1;\n"
	"	float3 normal : NORMAL;\n"
	"#if USE_VERTEX_COLOR\n"
	"	float4 color : COLOR0;\n"
	"#endif\n"
	"	float2 uvDiffuse : TEXCOORD0;\n"
	"};\n"
	"struct PS_OUTPUT { float4 color : COLOR; };\n"

	"PS_OUTPUT main(PS_INPUT _in) {\n"
	"	_in.uvDiffuse.y = 1 - _in.uvDiffuse.y\n;"
	"	PS_OUTPUT _out = (PS_OUTPUT) 0;\n"
	"	if(!mcdLighting) {\n"
	"		float4 diffuseMap = tex2D(texDiffuse, _in.uvDiffuse);\n"
	"		float3 diffuse = mcdDiffuseColor.rgb * mcdDiffuseColor.a * diffuseMap.rgb;\n"
	"#if USE_VERTEX_COLOR\n"
	"		_out.color.rgb = _in.color.rgb * diffuse;\n"
	"#else\n"
	"		_out.color.rgb = diffuse;\n"
	"#endif\n"
	"		_out.color.a = mcdOpacity * diffuseMap.a;\n"
	"		return _out;\n"
	"	}\n"

	"	float3 P = _in.worldPosition;\n"
	"	float3 N = normalize(_in.normal);\n"
	"	float3 V = normalize(mcdCameraPosition - P);\n"
	"	float3 lightDiffuse = 0;\n"
	"	float3 lightSpecular = 0;\n"

	"#if USE_BUMP_MAP\n"
	"	N = computeBump(P, _in.normal, _in.uvDiffuse, texBump, 1.0/10);\n"
	"#endif\n"

	"	for(int i=0; i<MCD_MAX_LIGHT_COUNT; ++i)\n"
	"		computeLighting(mcdLights[i], P, N, V, lightDiffuse, lightSpecular);\n"
	"	float4 diffuseMap = tex2D(texDiffuse, _in.uvDiffuse);\n"
	"	float3 diffuse = mcdDiffuseColor.rgb * mcdDiffuseColor.a * diffuseMap.rgb * lightDiffuse;\n"
	"	float3 specular = mcdSpecularColor.rgb * mcdSpecularColor.a * lightSpecular;\n"
	"	float3 emission = mcdEmissionColor.rgb * mcdEmissionColor.a;\n"

	"#if USE_VERTEX_COLOR\n"
	"	_out.color.rgb = _in.color.rgb * diffuse;\n"
	"#else\n"
	"	_out.color.rgb = diffuse + specular + emission;\n"
	"#endif\n"
	"	_out.color.a = mcdOpacity * diffuseMap.a;\n"
	"	return _out;\n"
	"}\n";

	if(mPs.ps) mPs.Release();

	if(ResourceManagerComponent* c = ResourceManagerComponent::fromCurrentEntityRoot())
		mPs = ShaderCache::singleton().getPixelShader(code, headerCode, c->resourceManager());
	else
		mPs = ShaderCache::singleton().getPixelShader(code);

	if(!mPs.ps || !mPs.constTable) return false;

	mConstantHandles.lights = mPs.constTable->GetConstantByName(nullptr, "mcdLights");
	mConstantHandles.cameraPosition = mPs.constTable->GetConstantByName(nullptr, "mcdCameraPosition");
	mConstantHandles.diffuseColor = mPs.constTable->GetConstantByName(nullptr, "mcdDiffuseColor");
	mConstantHandles.specularColor = mPs.constTable->GetConstantByName(nullptr, "mcdSpecularColor");
	mConstantHandles.emissionColor = mPs.constTable->GetConstantByName(nullptr, "mcdEmissionColor");
	mConstantHandles.specularExponent = mPs.constTable->GetConstantByName(nullptr, "mcdSpecularExponent");
	mConstantHandles.opacity = mPs.constTable->GetConstantByName(nullptr, "mcdOpacity");
	mConstantHandles.lighting = mPs.constTable->GetConstantByName(nullptr, "mcdLighting");

	return true;
}

void MaterialComponent::Impl::createShadersIfNeeded()
{
	static const char* defines[][2] = {
		{ "#define USE_VERTEX_COLOR 0\n", "#define USE_VERTEX_COLOR 1\n" },
		{ "#define USE_BUMP_MAP 0\n", "#define USE_BUMP_MAP 1\n" },
	};

	if(mLastMacro.initialized) {
		const bool hasChanges =
			mLastMacro.useVertexColor != mBackRef->useVertexColor ||
			mLastMacro.useBumpMap != bool(mBackRef->bumpMap);
		if(!hasChanges) return;
	}

	mLastMacro.initialized = true;

	std::string header;
	header += defines[0][mBackRef->useVertexColor ? 1 : 0];
	header += defines[1][mBackRef->bumpMap ? 1 : 0];

	mLastMacro.useVertexColor = mBackRef->useVertexColor;
	mLastMacro.useBumpMap = bool(mBackRef->bumpMap);

	MCD_VERIFY(createVs(header.c_str()));
	MCD_VERIFY(createPs(header.c_str()));
}

void MaterialComponent::Impl::updateWorldTransform(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	// NOTE: To make the shader code more uniform with glsl, we transpose the matrix before submitting to the shader
	MCD_VERIFY(mVs.constTable->SetMatrixTranspose(
		device, mConstantHandles.worldViewProj, (D3DXMATRIX*)renderer.mWorldViewProjMatrix.getPtr()
	) == S_OK);

	MCD_VERIFY(mVs.constTable->SetMatrixTranspose(
		device, mConstantHandles.world, (D3DXMATRIX*)renderer.mWorldMatrix.getPtr()
	) == S_OK);
}

void MaterialComponent::render(void* context)
{
	// Push light into Renderer's light list
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;

	mImpl.createShadersIfNeeded();
}

void MaterialComponent::preRender(size_t pass, void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	{	// Bind system information
		MCD_VERIFY(mImpl.mVs.constTable->SetMatrixTranspose(
			device, mImpl.mConstantHandles.worldViewProj, (D3DXMATRIX*)renderer.mWorldViewProjMatrix.getPtr()
		) == S_OK);

		MCD_VERIFY(mImpl.mVs.constTable->SetMatrixTranspose(
			device, mImpl.mConstantHandles.world, (D3DXMATRIX*)renderer.mWorldMatrix.getPtr()
		) == S_OK);

		if(!useVertexColor) {
			Vec3f cameraPosition = renderer.mCameraTransform.translation();
			MCD_VERIFY(mImpl.mVs.constTable->SetFloatArray(
				device, mImpl.mConstantHandles.cameraPosition, cameraPosition.getPtr(), 3
			) == S_OK);
		}
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

		if(!useVertexColor) {
			MCD_VERIFY(mImpl.mVs.constTable->SetFloatArray(
				device, mImpl.mConstantHandles.specularColor, specularColor.rawPointer(), 4
			) == S_OK);

			MCD_VERIFY(mImpl.mVs.constTable->SetFloatArray(
				device, mImpl.mConstantHandles.emissionColor, emissionColor.rawPointer(), 4
			) == S_OK);

			MCD_VERIFY(mImpl.mVs.constTable->SetFloat(
				device, mImpl.mConstantHandles.specularExponent, specularExponent
			) == S_OK);
		}

		MCD_VERIFY(mImpl.mVs.constTable->SetFloat(
			device, mImpl.mConstantHandles.opacity, opacity
		) == S_OK);

		MCD_VERIFY(mImpl.mVs.constTable->SetFloat(
			device, mImpl.mConstantHandles.lighting, lighting
		) == S_OK);

		if(TexturePtr diffuse = diffuseMap ? diffuseMap : renderer.mWhiteTexture) {
			int samplerIdx = mImpl.mPs.constTable->GetSamplerIndex("texDiffuse");
			device->SetSamplerState(samplerIdx, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			device->SetSamplerState(samplerIdx, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
			diffuse->bind(samplerIdx);
		}

		if(TexturePtr bump = bumpMap ? bumpMap : renderer.mWhiteTexture) {
			int samplerIdx = mImpl.mPs.constTable->GetSamplerIndex("texBump");
			device->SetSamplerState(samplerIdx, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			device->SetSamplerState(samplerIdx, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
			bump->bind(samplerIdx);
		}
	}

	device->SetVertexShader(mImpl.mVs.vs);
	device->SetPixelShader(mImpl.mPs.ps);

	if(!lighting)
		device->SetRenderState(D3DRS_LIGHTING, false);

	if(cullFace)
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	else
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
}

void MaterialComponent::postRender(size_t pass, void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);

	// Material state change early out optimization
	if(renderer.mLastMaterial == this)
		return;

	// Restore the lighting
	if(!lighting) {
		LPDIRECT3DDEVICE9 device = getDevice();
		MCD_ASSUME(device);
		device->SetRenderState(D3DRS_LIGHTING, !renderer.mLights.empty());
	}
}

MaterialComponent::MaterialComponent()
	: mImpl(*new Impl)
	, diffuseColor(1, 1)
	, specularColor(1, 1)
	, emissionColor(0, 1)
	, specularExponent(20)
	, opacity(1)
	, lighting(true)
	, cullFace(true)
	, useVertexColor(false)
{
	mImpl.mBackRef = this;
}

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
