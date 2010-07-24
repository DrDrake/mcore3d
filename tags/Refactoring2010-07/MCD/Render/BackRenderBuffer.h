#ifndef __MCD_RENDER_BACKRERENDERBUFFER__
#define __MCD_RENDER_BACKRERENDERBUFFER__

#include "RenderBuffer.h"

namespace MCD {

class MCD_RENDER_API BackRenderBuffer : public RenderBuffer
{
public:
	BackRenderBuffer();

	sal_override sal_checkreturn bool linkTo(RenderTarget& renderTarget);

	sal_override size_t width() const;

	sal_override size_t height() const;

	/*!	Create the render buffer with the given parameters.
		\param format
			The format argument for glRenderbufferStorageEXT();
			Accepted values are the same as those accepted by glTexImage*,
			with the addition of GL_STENCIL_INDEX{1|4|8|16}_EXT formats.

		\param usage GL_COLOR_ATTACHMENT{0,1...n}_EXT, GL_DEPTH_ATTACHMENT_EXT or GL_STENCIL_ATTACHMENT_EXT
	 */
	void create(size_t width, size_t height, int format, int usage);

	uint handle() const;

protected:
	sal_override ~BackRenderBuffer();

protected:
	uint mHandle;
	size_t mWidth, mHeight;
	int mFormat, mUsage;
};	// BackRenderBuffer

typedef IntrusivePtr<BackRenderBuffer> BackRenderBufferPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_BACKRERENDERBUFFER__
