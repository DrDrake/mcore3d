#ifndef __MCD_RENDER_RENDERTARGET__
#define __MCD_RENDER_RENDERTARGET__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/IntrusivePtr.h"
#include <vector>

namespace MCD {

class RenderBuffer;
typedef IntrusivePtr<RenderBuffer> RenderBufferPtr;

class MCD_RENDER_API RenderTarget : private Noncopyable
{
public:
	RenderTarget(size_t width, size_t height);

	~RenderTarget();

	void bind();

	void unbind();

	size_t width() const {
		return mWidth;
	}

	size_t height() const {
		return mHeight;
	}

protected:
	uint mHandle;
	size_t mWidth, mHeight;

	friend class RenderBuffer;

	/*!	The RenderTarget owns it's corresponding render buffers.
		While the render buffer may shared by other render target(s).
	 */
	std::vector<RenderBufferPtr> mRenderBuffers;
};	// RenderTarget

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERTARGET__
