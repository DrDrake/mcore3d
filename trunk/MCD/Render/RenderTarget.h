#ifndef __MCD_RENDER_RENDERTARGET__
#define __MCD_RENDER_RENDERTARGET__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/IntrusivePtr.h"
#include <vector>

namespace MCD {

class RenderBuffer;
typedef IntrusivePtr<RenderBuffer> RenderBufferPtr;

/*!	Represent a configuration of buffers that current render context will render to.
	Render target, also know as Framebuffer object (FBO) in opengl, allow you to divert
	your rendering away from your window's framebuffer to one or more offscreen framebuffers
	that you create.

	To use RenderTarget, you have to create some render buffers first. Those buffer
	By design, the binding code is 

	\code
	// Setup the RenderTarget
	RenderTarget renderTarget(width, height);
	RenderBufferPtr textureBuffer = new TextureRenderBuffer();
	textureBuffer->linkTo(renderTarget);
	mRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

	RenderBufferPtr backBuffer = new BackRenderBuffer();
	backBuffer->bind(*mRenderTarget);
	}
	\endcode

	\sa IRenderBuffer
	\sa http://www.gamedev.net/reference/articles/article2333.asp
	\sa http://www.flashbang.se/postbreak.php?id=18
	\sa http://www.songho.ca/opengl/gl_fbo.html
 */
class MCD_RENDER_API RenderTarget : private Noncopyable
{
public:
	RenderTarget(size_t width, size_t height);

	~RenderTarget();

	/*!	Use this render target.
		Subsequence renderings will go to the binded buffers.
	 */
	void bind() const;

	/*!	Un-use this render target.
		Subsequence renderings result will go to the normal front buffer.
	 */
	void unbind() const;

	//! Check the render target configuration is correct or not.
	bool checkCompleteness(sal_out_opt int* errorCode=nullptr) const;

	uint handle() const;

	size_t width() const;

	size_t height() const;

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
