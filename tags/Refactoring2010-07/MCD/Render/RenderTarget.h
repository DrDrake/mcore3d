#ifndef __MCD_RENDER_RENDERTARGET__
#define __MCD_RENDER_RENDERTARGET__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/IntrusivePtr.h"
#include <vector>

namespace MCD {

typedef IntrusivePtr<class RenderBuffer> RenderBufferPtr;

/*!	Represent a configuration of buffers that current render context will render to.
	Render target, also know as Framebuffer object (FBO) in opengl, allow you to divert
	your rendering away from your window's framebuffer to one or more offscreen framebuffers
	that you create.

	To use RenderTarget, you have to create some render buffers first. Those buffer are then
	linked to the RenderTarget and identified by the opengl enum GL_COLOR_ATTACHMENT0_EXT,
	GL_COLOR_ATTACHMENT1_EXT etc... How the buffers are finally accessed in the glsl depends
	on the call of glDrawBuffers() to specify.

	Example setup:
	\code
	// Create a texture buffer
	RenderTarget renderTarget(width, height);
	RenderBufferPtr textureBuffer = new TextureRenderBuffer();
	// You can either ask texture buffer to create a texture for you,
	textureBuffer->createTexture(width, height, GL_TEXTURE_RECTANGLE_ARB, GL_RGB);
	// or supply with your own.
	textureBuffer->texture = yourTexture;
	textureBuffer->linkTo(renderTarget);

	// Create a back buffer
	RenderBufferPtr backBuffer = new BackRenderBuffer();
	backBuffer->linkTo(renderTarget);

	// Definds the order of the 2 buffer appear in glsl
	// This can also be redefined in the render loop
	renderTarget->bind();
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
	glDrawBuffers(2, buffers);
	renderTarget->unbind();

	// Check that everythings are fine
	if(!renderTarget->checkCompleteness())
		exit(-1);
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
	sal_checkreturn bool checkCompleteness(sal_out_opt int* errorCode=nullptr) const;

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
