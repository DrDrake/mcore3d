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
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mHandle);
}

void RenderTarget::unbind()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

uint RenderTarget::handle() const
{
	return mHandle;
}

size_t RenderTarget::width() const
{
	return mWidth;
}

size_t RenderTarget::height() const
{
	return mHeight;
}

}	// namespace MCD
