#include "Pch.h"
#include "../Font.h"
#include "../Texture.h"
#include "Renderer.inc"
#include "../../Core/Entity/Entity.h"

// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=330742

namespace MCD {

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
	// Reference: es_full_spec_1.1.12.pdf, p93-95
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color.rawPointer());

	// RGB = Cs * Cf
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	// Alpha = As * Af
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mVertexBuffer[0].uv);
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &mVertexBuffer[0].position);
	glDrawArrays(GL_TRIANGLES, 0, mVertexBuffer.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	++statistic.drawCallCount;
	statistic.primitiveCount += mVertexBuffer.size() / 3;
}

void BmpFontMaterialComponent::render(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;
}

void BmpFontMaterialComponent::preRender(size_t pass, void* context)
{
	if(bmpFont && bmpFont->texture) {
		bmpFont->texture->bind(0);

		glDisable(GL_LIGHTING);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

void BmpFontMaterialComponent::postRender(size_t pass, void* context)
{
	if(bmpFont && bmpFont->texture)
		bmpFont->texture->unbind(0);
}

}	// namespace MCD
