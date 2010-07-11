#include "Pch.h"
#include "../Font.h"
#include "../Texture.h"
#include "Renderer.inc"
#include "../../Core/Entity/Entity.h"

// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=330742

namespace MCD {

void FontComponent::render()
{
}

void FontComponent::render2(void* context)
{
	if(text.empty() || !bmpFont)
		return;

	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	IMaterialComponent* m = renderer.mMaterialStack.top();

	const Mat44f worldTransform = entity()->worldTransform();
	Mat44f transform = Mat44f::cIdentity;

	const size_t textureWidth = bmpFont->charSet.width;
	const size_t textureHeight = bmpFont->charSet.height;

	Vec3f charPos = Vec3f::cZero;

	for(std::string::const_iterator i=text.begin(); i != text.end(); ++i) {
		const unsigned char c = *i;
		const BmpFont::CharDescriptor& desc = bmpFont->charSet.chars[c];

		Vec4f uv(desc.x, desc.y, float(desc.x + desc.width), float(desc.y + desc.height));
		uv.x /= textureWidth;
		uv.y /= textureHeight;
		uv.z /= textureWidth;
		uv.w /= textureHeight;

		transform.setTranslation(charPos + Vec3f(desc.xOffset + float(desc.width) / 2, desc.yOffset - float(desc.height), 0));

		// NOTE: Quad renderer use the centre of each quad as the anchor
		renderer.mQuadRenderer->push(worldTransform * transform, desc.width, desc.height, uv, m);

		charPos.x += desc.xAdvance;
	}
}

}	// namespace MCD
