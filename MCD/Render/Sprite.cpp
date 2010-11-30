#include "Pch.h"
#include "Sprite.h"
#include "../Core/Entity/Entity.h"

namespace MCD {

static SpriteUpdaterComponent* gSpriteUpdater = nullptr;

void SpriteComponent::gather()
{
	// Find it's parent SpriteAtlasComponent
	SpriteAtlasComponent* r = nullptr;
	for(Entity* e = entity(); e; e = e->parent()) {
		r = e->findComponentExactType<SpriteAtlasComponent>();
		if(r) break;
	}

	if(r)
		r->gatherSprite(this);
}

void SpriteAtlasComponent::gather()
{
	MCD_ASSUME(gSpriteUpdater);
	gSpriteUpdater->mSpriteAtlas.push_back(this);
}

void SpriteAtlasComponent::gatherSprite(SpriteComponent* sprite)
{
	ColorRGBAf c(1, 1);
	const float left = 0;
	const float right = sprite->width;
	const float top = 0;
	const float bottom = sprite->height;
	const Vec4f& uv = sprite->uv;
	Vertex v[4] = {
		{	Vec3f(left, -top, 0),		Vec2f(uv.x, uv.y), c	},	// Left top
		{	Vec3f(left, -bottom, 0),	Vec2f(uv.x, uv.w), c	},	// Left bottom
		{	Vec3f(right, -bottom, 0),	Vec2f(uv.z, uv.w), c	},	// Right bottom
		{	Vec3f(right, -top, 0),		Vec2f(uv.z, uv.y), c	}	// Right top
	};

	Mat44f m = Mat44f::cIdentity;
	// Traverse up SpriteComponent until it meets SpriteAtlasComponent
	for(Entity* e=sprite->entity(); e!=entity(); e=e->parent())
		m = e->localTransform * m;

	m.transformPoint(v[0].position);
	m.transformPoint(v[1].position);
	m.transformPoint(v[2].position);
	m.transformPoint(v[3].position);

	mVertexBuffer.push_back(v[0]);
	mVertexBuffer.push_back(v[1]);
	mVertexBuffer.push_back(v[2]);
	mVertexBuffer.push_back(v[0]);
	mVertexBuffer.push_back(v[2]);
	mVertexBuffer.push_back(v[3]);
}

void SpriteUpdaterComponent::begin()
{
	gSpriteUpdater = this;
}

void SpriteUpdaterComponent::end(float dt)
{
	for(size_t i=0; i<mSpriteAtlas.size(); ++i)
		mSpriteAtlas[i]->mVertexBuffer.clear();

	gSpriteUpdater = nullptr;
}

}	// namespace MCD
