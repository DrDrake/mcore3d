#include "Pch.h"
#include "../Font.h"
#include "../Texture.h"
#include "Renderer.inc"
#include "../../Core/Entity/Entity.h"

// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=330742

namespace MCD {

void TextLabelComponent::render() {}

void TextLabelComponent::render2(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	BmpFontMaterialComponent* m = dynamic_cast<BmpFontMaterialComponent*>(renderer.mCurrentMaterial);
	if(!m) return;

	// Get the BmpFont from the current material
	if(const BmpFont* bmpFont = m->bmpFont.get()) if(bmpFont->commitCount > 0)
	{
		// Check to see if we need to rebuild the vertex buffer
		const size_t hash = StringHash(text.c_str(), 0).hash;

		const bool fontChanged = (mLastBmpFont != bmpFont || bmpFont->commitCount != mLastBmpFontCommitCount);

		if(mStringHash != hash || fontChanged) {
			buildVertexBuffer(*bmpFont);
			mStringHash = hash;
			mLastBmpFont = bmpFont;
			mLastBmpFontCommitCount = bmpFont->commitCount;
		}
	}

	if(!mVertexBuffer.empty()) {
		RenderItem r = { entity(), this, m };
		renderer.mTransparentQueue.insert(*new RenderItemNode(0, r));
	}
}

void TextLabelComponent::draw(sal_in void* context)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mVertexBuffer[0].uv);
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &mVertexBuffer[0].position);
	glDrawArrays(GL_TRIANGLES, 0, mVertexBuffer.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void BmpFontMaterialComponent::render2(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;
}

void BmpFontMaterialComponent::preRender(size_t pass, void* context)
{
	if(bmpFont && bmpFont->texture)
		bmpFont->texture->bind();
}

void BmpFontMaterialComponent::postRender(size_t pass, void* context)
{
	if(bmpFont && bmpFont->texture)
		bmpFont->texture->unbind();
}

}	// namespace MCD
