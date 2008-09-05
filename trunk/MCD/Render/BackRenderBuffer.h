#ifndef __MCD_RENDER_BACKRERENDERBUFFER__
#define __MCD_RENDER_BACKRERENDERBUFFER__

#include "RenderBuffer.h"

namespace MCD {

class MCD_RENDER_API BackRenderBuffer : public RenderBuffer
{
public:
	BackRenderBuffer();

	BackRenderBuffer(size_t width, size_t height);

protected:
	sal_override ~BackRenderBuffer();

public:
	sal_override sal_checkreturn bool linkTo(RenderTarget& renderTarget);

	sal_override size_t width() const;

	sal_override size_t height() const;

	uint handle() const;

protected:
	void initParam();

	void create();

	uint mHandle;
	size_t mWidth, mHeight;
	int mFormat, mUsage;
};	// BackRenderBuffer

}	// namespace MCD

#endif	// __MCD_RENDER_BACKRERENDERBUFFER__
