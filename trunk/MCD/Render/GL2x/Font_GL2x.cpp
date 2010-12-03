#include "Pch.h"
#include "../Font.h"
#include "../RenderTarget.h"
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
		renderer.mTransparentQueue.insert(*new RenderItemNode(r.worldTransform.translation().z, r));
	}
}

void TextLabelComponent::draw(sal_in void* context, Statistic& statistic)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);

	if(renderer.mCurrentCamera->frustum.projectionType == Frustum::Perspective)
	{
		// Create orthogonal projection
		const float width = (float)renderer.mCurrentRenderTarget->targetWidth();
		const float height = (float)renderer.mCurrentRenderTarget->targetHeight();
		Frustum f;
		f.projectionType = Frustum::YDown2D;
		f.create(-0, width, 0, height, -1, 1);
		Mat44f proj;
		f.computeOrtho(proj.data);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixf(proj.data);

		// Calculate screen position (0,0 -> Lower left, Screen width,height -> upper right)
		Vec3f p(0);
		renderer.mWorldViewProjMatrix.transformPointPerspective(p);	// Homogeneous screen position
		p = (p + 1) * 0.5;
		p.x *= width;
		p.y *= height;

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadMatrixf(Mat44f::makeTranslation(p).data);
	}

	// Reference: es_full_spec_1.1.12.pdf, p93-95
	// Reference: http://www.opengl.org/wiki/Texture_Combiners
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

	if(renderer.mCurrentCamera->frustum.projectionType == Frustum::Perspective)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

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
