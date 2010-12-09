#include "Pch.h"
#include "../Sprite.h"
#include "Renderer.inc"
#include "Helper/ShaderCache.h"
#include <D3DX9Shader.h>

typedef const char* D3DXHANDLE;

namespace MCD {

using namespace DX9Helper;

class SpriteAtlasComponent::Impl
{
public:
	Impl();

	~Impl() {
		mVs.Release();
		mPs.Release();
		mVertexDecl->Release();
	}

	DX9Helper::ShaderCache::Vs mVs;
	DX9Helper::ShaderCache::Ps mPs;

	struct ConstantHandles
	{
		D3DXHANDLE worldViewProj;
	};	// ConstantHandles

	ConstantHandles mConstantHandles;

	LPDIRECT3DVERTEXDECLARATION9 mVertexDecl;

	static Impl* singleton;
	static size_t refCount;
};	// Impl

SpriteAtlasComponent::Impl* SpriteAtlasComponent::Impl::singleton = nullptr;
size_t SpriteAtlasComponent::Impl::refCount = 0;

static const char* gVsSource = "\
float4x4 mcdWorldViewProj;\n\
struct VS_INPUT {\n\
	float3 position : POSITION;\n\
	float4 color : COLOR0;\n\
	float2 uv : TEXCOORD0;\n\
};\n\
struct VS_OUTPUT {\n\
	float4 position : POSITION;\n\
	float4 color : COLOR0;\n\
	float2 uv : TEXCOORD0;\n\
};\n\
VS_OUTPUT main(const VS_INPUT _in) {\n\
	VS_OUTPUT _out;\n\
	_out.position = mul(mcdWorldViewProj, float4(_in.position, 1));\n\
	_out.uv = _in.uv;\n\
	_out.color = _in.color;\n\
	return _out;\n\
}";

static const char* gPsSource = "\
sampler2D tex;\n\
struct PS_INPUT {\n\
	float4 color : COLOR0;\n\
	float2 uv: TEXCOORD0;\n\
};\n\
struct PS_OUTPUT { float4 color : COLOR; };\n\
PS_OUTPUT main(PS_INPUT _in) {\n\
	PS_OUTPUT _out = (PS_OUTPUT) 0;\n\
	_out.color = _in.color * tex2Dlod(tex, float4(_in.uv, 0, 0));\n\
	return _out;\n\
}";

SpriteAtlasComponent::Impl::Impl()
{
	if(mVs.vs) mVs.Release();
	mVs = ShaderCache::singleton().getVertexShader(gVsSource);
	MCD_ASSERT(mVs.vs && mVs.constTable);

	mConstantHandles.worldViewProj = mVs.constTable->GetConstantByName(nullptr, "mcdWorldViewProj");

	if(mPs.ps) mPs.Release();
	mPs = ShaderCache::singleton().getPixelShader(gPsSource);
	MCD_ASSERT(mPs.ps && mPs.constTable);

	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	D3DVERTEXELEMENT9 vertexDecl[3+1] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, sizeof(float)*3, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, sizeof(float)*(3+2), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};
	MCD_VERIFY(device->CreateVertexDeclaration(vertexDecl, &mVertexDecl) ==  D3D_OK);
	MCD_ASSUME(mVertexDecl);
}

SpriteAtlasComponent::SpriteAtlasComponent()
{
	if(Impl::singleton) mImpl = Impl::singleton;
	else mImpl = Impl::singleton = new Impl;

	++Impl::refCount;
}

SpriteAtlasComponent::~SpriteAtlasComponent()
{
	MCD_ASSUME(Impl::singleton);
	MCD_ASSUME(Impl::singleton == mImpl);
	--Impl::refCount;

	if(Impl::refCount == 0) {
		delete Impl::singleton;
		Impl::singleton = nullptr;
	}
}

void SpriteAtlasComponent::render(void* context)
{
	if(mVertexBuffer.empty() || !textureAtlas)
		return;

	Entity* e = entity();
	MCD_ASSUME(e);
	RenderItem r = { e, this, nullptr, e->worldTransform() };
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mTransparentQueue.insert(*new RenderItemNode(r.worldTransform.translation().z, r));
}

void SpriteAtlasComponent::draw(void* context, Statistic& statistic)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	MCD_VERIFY(mImpl->mVs.constTable->SetMatrixTranspose(
		device, mImpl->mConstantHandles.worldViewProj, (D3DXMATRIX*)renderer.mWorldViewProjMatrix.getPtr()
	) == S_OK);

	MCD_ASSERT(textureAtlas);
	const int samplerIdx = mImpl->mPs.constTable->GetSamplerIndex("tex");
	textureAtlas->bind(samplerIdx);
	device->SetSamplerState(samplerIdx, D3DSAMP_MAXANISOTROPY, 1);

	device->SetVertexShader(mImpl->mVs.vs);
	device->SetPixelShader(mImpl->mPs.ps);

	MCD_VERIFY(device->SetVertexDeclaration(mImpl->mVertexDecl) == D3D_OK);
	MCD_VERIFY(device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, mVertexBuffer.size() / 3, &mVertexBuffer[0], sizeof(Vertex)) == D3D_OK);

	textureAtlas->unbind(samplerIdx);

	++statistic.drawCallCount;
	statistic.primitiveCount += mVertexBuffer.size() / 3;
}

}	// namespace MCD
