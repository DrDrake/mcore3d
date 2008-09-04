#include "Pch.h"
#include "BackRenderBuffer.h"
#include "RenderTarget.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

BackRenderBuffer::BackRenderBuffer()
	: mHandle(0), mWidth(0), mHeight(0)
{
	initParam();
}

BackRenderBuffer::BackRenderBuffer(size_t width, size_t height)
	: mWidth(width), mHeight(height)
{
	initParam();
	create();
}

BackRenderBuffer::~BackRenderBuffer()
{
	glDeleteRenderbuffersEXT(1, &mHandle);
}

bool BackRenderBuffer::bind(RenderTarget& renderTarget)
{
	if(!mHandle) {
		mWidth = renderTarget.width();
		mHeight = renderTarget.height();
		create();
	} else {
		if(mWidth != renderTarget.width() || mHeight != renderTarget.height())
			return false;
	}

	renderTarget.bind();
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
		mUsage, GL_RENDERBUFFER_EXT, mHandle);

	addOwnerShipTo(renderTarget);

	return true;
}

size_t BackRenderBuffer::width() const
{
	return mWidth;
}

size_t BackRenderBuffer::height() const
{
	return mHeight;
}

uint BackRenderBuffer::handle() const
{
	return mHandle;
}

void BackRenderBuffer::initParam()
{
	// TODO: Some way to set the param rather than hardcode

	// Accepted formats are the same as those accepted by glTexImage*,
	// with the addition of GL_STENCIL_INDEX{1|4|8|16}_EXT formats.
	mFormat = GL_DEPTH_COMPONENT24;

	// Usage can be:
	// GL_DEPTH_ATTACHMENT_EXT, GL_STENCIL_ATTACHMENT_EXT, or GL_COLOR_ATTACHMENTn_EXT
	mUsage = GL_DEPTH_ATTACHMENT_EXT;
}

void BackRenderBuffer::create()
{
	int maxRenderbufferWidth;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &maxRenderbufferWidth);

//		int maxWidthHeight = mWidth > mHeight ? mWidth : mHeight;
//		if(maxWidthHeight > maxRenderbufferWidth)
//			Log::write(Log::Error, L"Reaching the size limit of render buffer: expecting %i, but only %i is supported",
//				maxWidthHeight, maxRenderbufferWidth);

	glGenRenderbuffersEXT(1, &mHandle);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mHandle);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, mFormat, mWidth, mHeight);
}

}	// namespace MCD
