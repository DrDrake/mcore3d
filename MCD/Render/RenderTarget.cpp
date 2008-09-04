#include "Pch.h"
#include "RenderTarget.h"
#include "RenderBuffer.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

RenderTarget::RenderTarget(size_t width, size_t height)
	: mWidth(width), mHeight(height)
{
	glGenFramebuffersEXT(1, &mHandle);
}

RenderTarget::~RenderTarget()
{
	glDeleteFramebuffersEXT(1, &mHandle);
}

void RenderTarget::bind()
{
	// Assure the texture that is binded to the render target is not
	// to be read as texture during rendering.
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mHandle);
}

void RenderTarget::unbind()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

}	// namespace MCD
