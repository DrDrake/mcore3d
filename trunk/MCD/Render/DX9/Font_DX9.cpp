#include "Pch.h"
#include "../Font.h"
#include "../Texture.h"
#include "Renderer.inc"
#include "../../Core/Entity/Entity.h"
#include <D3DX9Shader.h>

// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=330742

namespace MCD {

static const int cQuadVertexFVF = (D3DFVF_XYZ|D3DFVF_TEX1);

void TextLabelComponent::render() {}

void TextLabelComponent::render2(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	BmpFontMaterialComponent* m = dynamic_cast<BmpFontMaterialComponent*>(renderer.mCurrentMaterial);
	if(!m) return;

	// Get the BmpFont from the current material
	if(const BmpFont* bmpFont = m->bmpFont.get()) if(bmpFont->commitCount() > 0)
	{
		// Check to see if we need to rebuild the vertex buffer
		const size_t hash = StringHash(text.c_str(), 0).hash;

		const bool fontChanged = (mLastBmpFont != bmpFont || bmpFont->commitCount() != mLastBmpFontCommitCount);

		if(mStringHash != hash || fontChanged) {
			buildVertexBuffer(*bmpFont);
			mStringHash = hash;
			mLastBmpFont = bmpFont;
			mLastBmpFontCommitCount = bmpFont->commitCount();
		}
	}

	if(!mVertexBuffer.empty()) {
		Entity* e = entity();
		MCD_ASSUME(e);
		RenderItem r = { e, this, m, e->worldTransform() };
		renderer.mTransparentQueue.insert(*new RenderItemNode(0, r));
	}
}

void TextLabelComponent::draw(sal_in void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);
	MCD_VERIFY(device->SetFVF(cQuadVertexFVF) == D3D_OK);
	MCD_VERIFY(device->SetTransform(D3DTS_WORLD, (D3DMATRIX*)renderer.mWorldMatrix.transpose().getPtr()) == D3D_OK);	// TODO: Why transpose?
	MCD_VERIFY(device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, mVertexBuffer.size() / 3, &mVertexBuffer[0], sizeof(Vertex)) == D3D_OK);
}

void BmpFontMaterialComponent::render2(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;
}

void BmpFontMaterialComponent::preRender(size_t pass, void* context)
{
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);
	MCD_VERIFY(device->SetVertexShader(nullptr) == D3D_OK);
	MCD_VERIFY(device->SetPixelShader(nullptr) == D3D_OK);

	if(bmpFont && bmpFont->texture)
		bmpFont->texture->bind(0);
}

void BmpFontMaterialComponent::postRender(size_t pass, void* context)
{
	if(bmpFont && bmpFont->texture)
		bmpFont->texture->unbind();
}

}	// namespace MCD
