#include "Pch.h"
#include "../Font.h"
#include "../Texture.h"
#include "Renderer.inc"
#include "../../Core/Entity/Entity.h"
#include <D3DX9Shader.h>

// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=330742

namespace MCD {

// Note that DirectX expects to find the structure fields in a specific order
// http://insanedevelopers.net/2009/03/22/directx-9-tutorials-02-vertex-buffers/
static const int cQuadVertexFVF = (D3DFVF_XYZ|D3DFVF_TEX1);

void TextLabelComponent::render(void* context)
{
	update();

	if(mFontMaterial && !mVertexBuffer.empty()) {
		Entity* e = entity();
		MCD_ASSUME(e);
		RenderItem r = { e, this, mFontMaterial, e->worldTransform() };
		RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
		renderer.mTransparentQueue.insert(*new RenderItemNode(0, r));
	}
}

void TextLabelComponent::draw(sal_in void* context, Statistic& statistic)
{
/*	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=310926
	device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_COLORVALUE(color.r, color.g, color.b, color.a));
	device->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

	MCD_VERIFY(device->SetFVF(cQuadVertexFVF) == D3D_OK);
	MCD_VERIFY(device->SetTransform(D3DTS_WORLD, (D3DMATRIX*)renderer.mWorldMatrix.getPtr()) == D3D_OK);
	MCD_VERIFY(device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, mVertexBuffer.size() / 3, &mVertexBuffer[0], sizeof(Vertex)) == D3D_OK);

	++statistic.drawCallCount;
	statistic.primitiveCount += mVertexBuffer.size() / 3;*/
}

void BmpFontMaterialComponent::render(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;
}

void BmpFontMaterialComponent::preRender(size_t pass, void* context)
{
/*	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);
	MCD_VERIFY(device->SetVertexShader(nullptr) == D3D_OK);
	MCD_VERIFY(device->SetPixelShader(nullptr) == D3D_OK);

	if(bmpFont && bmpFont->texture)
		bmpFont->texture->bind(0);*/
}

void BmpFontMaterialComponent::postRender(size_t pass, void* context)
{
//	if(bmpFont && bmpFont->texture)
//		bmpFont->texture->unbind();
}

}	// namespace MCD
