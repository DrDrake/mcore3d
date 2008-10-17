#include "Pch.h"
#include "RenderTarget.h"
#include "RenderBuffer.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

RenderTarget::RenderTarget(size_t width, size_t height)
	: mWidth(width), mHeight(height)
{
	MCD_ASSERT(width > 0);
	MCD_ASSERT(height > 0);
	glGenFramebuffersEXT(1, &mHandle);
}

RenderTarget::~RenderTarget()
{
	glDeleteFramebuffersEXT(1, &mHandle);
}

void RenderTarget::bind() const
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mHandle);
}

void RenderTarget::unbind() const
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

bool RenderTarget::checkCompleteness(int* errorCode) const
{
	bind();

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(errorCode)
		*errorCode = status;

	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE_EXT:
		return true;
	default:
		break;
	}

	return false;
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
