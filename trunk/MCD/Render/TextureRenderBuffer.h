#ifndef __MCD_RENDER_TEXTURERENDERBUFFER__
#define __MCD_RENDER_TEXTURERENDERBUFFER__

#include "RenderBuffer.h"

namespace MCD {

class Texture;
typedef IntrusivePtr<Texture> TexturePtr;

class MCD_RENDER_API TextureRenderBuffer : public RenderBuffer
{
public:
	TextureRenderBuffer();
	TextureRenderBuffer(const TexturePtr& tex);

protected:
	sal_override ~TextureRenderBuffer() {}

public:
	sal_override bool bind(RenderTarget& renderTarget);

	sal_override size_t width() const;

	sal_override size_t height() const;

	TexturePtr texture;
};	// TextureRenderBuffer

}	// namespace MCD

#endif	// __MCD_RENDER_TEXTURERENDERBUFFER__
