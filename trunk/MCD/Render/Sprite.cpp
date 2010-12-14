#include "Pch.h"
#include "Sprite.h"
#include "../Core/Entity/Entity.h"

namespace MCD {

static SpriteUpdaterComponent* gSpriteUpdater = nullptr;

SpriteComponent::SpriteComponent()
	: color(1, 1), textureRect(0, 0, 1, 1), anchor(0.5f), width(0), height(0), trackOffset(0)
{
}

void SpriteComponent::update()
{
	// Find it's parent SpriteAtlasComponent
	SpriteAtlasComponent* r = nullptr;
	for(Entity* e = entity(); e; e = e->parent()) {
		r = e->findComponentExactType<SpriteAtlasComponent>();
		if(r) {
			r->gatherSprite(this);
			break;
		}
	}
}

void SpriteAtlasComponent::gather()
{
	MCD_ASSUME(gSpriteUpdater);
	gSpriteUpdater->mSpriteAtlas.push_back(this);
}

// NOTE: Re-generating the vertex buffer every frame is going to be slower
// that those implementation with lazy position update, if most of the visible sprite
// are static.
// But since we decided not to impose complexity on notification of positional
// changes of the Entity tree, our best bet is to make the vertex construction fast.
void SpriteAtlasComponent::gatherSprite(SpriteComponent* sprite)
{
	const float left   = -sprite->anchor.x * sprite->width;
	const float right  = left + sprite->width;
	const float top    = sprite->anchor.y * sprite->height;
	const float bottom = top - sprite->height;

	Vec4f uv = sprite->textureRect;
	const ColorRGBAf& c = sprite->color;

	// TODO: Use animated data
	if(sprite->animation) {
		const AnimationState::Pose& pose = sprite->animation->getPose();
		uv = pose[sprite->trackOffset].v;
	}

	if(uv.x >= 2 || uv.z >= 2) {
		// Transform pixle unit into normalized uv unit
		const float invTexWidth = 1.0f / textureAtlas->width;
		const float invTexHeight = 1.0f / textureAtlas->height;
		uv.x *= invTexWidth;
		uv.z *= invTexWidth;
		uv.y *= invTexHeight;
		uv.w *= invTexHeight;
	}

	Vertex v[4] = {
		{	Vec3f(left, top, 0),		Vec2f(uv.x, uv.y), c	},
		{	Vec3f(left, bottom, 0),		Vec2f(uv.x, uv.w), c	},
		{	Vec3f(right, bottom, 0),	Vec2f(uv.z, uv.w), c	},
		{	Vec3f(right, top, 0),		Vec2f(uv.z, uv.y), c	}
	};

	Mat44f m = Mat44f::cIdentity;
	// Traverse up SpriteComponent until it meets SpriteAtlasComponent,
	// and calculate the relative transform
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
	for(size_t i=0; i<mSpriteAtlas.size(); ++i)
		mSpriteAtlas[i]->mVertexBuffer.clear();
	mSpriteAtlas.clear();
}

void SpriteUpdaterComponent::end(float dt)
{
	gSpriteUpdater = nullptr;
}

}	// namespace MCD
