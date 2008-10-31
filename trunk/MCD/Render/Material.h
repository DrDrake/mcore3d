#ifndef __MCD_RENDER_MATERIAL__
#define __MCD_RENDER_MATERIAL__

#include "Color.h"
#include "../Core/System/IntrusivePtr.h"

namespace MCD {

class Texture;
typedef IntrusivePtr<Texture> TexturePtr;

class MCD_RENDER_API Material
{
public:
	Material();
	~Material();

	void bind() const;

	ColorRGBf mAmbient, mDiffuse, mSpecular;
	uint8_t mShininess;
	TexturePtr mTexture;
};	// Material

}	// namespace MCD

#endif	// __MCD_RENDER_MATERIAL__
