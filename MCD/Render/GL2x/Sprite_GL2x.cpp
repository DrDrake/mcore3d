#include "Pch.h"
#include "../Sprite.h"
#include "Renderer.inc"

namespace MCD {

SpriteAtlasComponent::SpriteAtlasComponent() : mImpl(nullptr) {}

SpriteAtlasComponent::~SpriteAtlasComponent() {}

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

void SpriteAtlasComponent::draw(sal_in void* context, Statistic& statistic)
{
	MCD_ASSERT(textureAtlas);

	textureAtlas->bind(0);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mVertexBuffer[0].uv);

	glColorPointer(4, GL_FLOAT, sizeof(Vertex), &mVertexBuffer[0].color);
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &mVertexBuffer[0].position);
	glDrawArrays(GL_TRIANGLES, 0, mVertexBuffer.size());

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	textureAtlas->unbind(0);

	++statistic.drawCallCount;
	statistic.primitiveCount += mVertexBuffer.size() / 3;
}

}	// namespace MCD
