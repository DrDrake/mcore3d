#ifndef __MCD_RENDER_TEXTURERENDERBUFFER__
#define __MCD_RENDER_TEXTURERENDERBUFFER__

#include "RenderBuffer.h"

namespace MCD {

typedef IntrusivePtr<class Texture> TexturePtr;

/*!	Use a texture as the render target.
	\note
		You cannot read from the texture that is bound to
		it's RenderTargetthat you currently render to.
	\sa RenderTarget
 */
class MCD_RENDER_API TextureRenderBuffer : public RenderBuffer
{
public:
	TextureRenderBuffer(int type);
	TextureRenderBuffer(const TexturePtr& tex);

protected:
	sal_override ~TextureRenderBuffer() {}

public:
// Operations
	/*!	Bind this buffer to the render target.
		Must have the texture created before linking.
	 */
	sal_override sal_checkreturn bool linkTo(RenderTarget& renderTarget);

	/*!	Helper function to create a texture for use as render buffer.
		Although you can supply your own pre-defined texture by simply assigning
		to the \em texture member variable. But this function is handy enough
		for most tasks, for example creating a color/depth texture.

		\note The ownership of the original texture (if any) will be removed.
		\param type Can be GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_ARB.
		\param usage Can be GL_COLOR_ATTACHMENTn_EXT or GL_DEPTH_ATTACHMENT_EXT
	 */
	sal_checkreturn bool createTexture(size_t width, size_t height, int type, int format, const char* name=nullptr);

	sal_checkreturn bool create(size_t width, size_t height, int type, int format, int dataType, int components, const char* name=nullptr);

// Attributes
	sal_override size_t width() const;

	sal_override size_t height() const;

	TexturePtr texture;

protected:
	int mAttachmentType;	//!< Type of the texture render buffer, can be depth or color.
};	// TextureRenderBuffer

typedef IntrusivePtr<TextureRenderBuffer> TextureRenderBufferPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_TEXTURERENDERBUFFER__
