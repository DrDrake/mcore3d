#include "Pch.h"
#include "BackRenderBuffer.h"
#include "RenderTarget.h"
#include "../Core/System/Log.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

BackRenderBuffer::BackRenderBuffer()
	: mHandle(0), mWidth(0), mHeight(0), mFormat(-1), mUsage(-1)
{
}

BackRenderBuffer::~BackRenderBuffer()
{
	glDeleteRenderbuffersEXT(1, &mHandle);
}

bool BackRenderBuffer::linkTo(RenderTarget& renderTarget)
{
	if(!mHandle)
		return false;
	else if(mWidth != renderTarget.width() || mHeight != renderTarget.height())
		return false;

	renderTarget.bind();
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
		mUsage, GL_RENDERBUFFER_EXT, mHandle);
	renderTarget.unbind();

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

void BackRenderBuffer::create(size_t width, size_t height, int format, int usage)
{
	mWidth = width;
	mHeight = height;
	mFormat = format;
	mUsage = usage;

	int maxRenderbufferWidth;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &maxRenderbufferWidth);

	int maxWidthHeight = mWidth > mHeight ? mWidth : mHeight;
	if(maxWidthHeight > maxRenderbufferWidth)
		Log::format(Log::Error, L"Reaching the size limit of render buffer: expecting %i, but only %i is supported",
			maxWidthHeight, maxRenderbufferWidth);

	glGenRenderbuffersEXT(1, &mHandle);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mHandle);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, mFormat, mWidth, mHeight);
}

}	// namespace MCD
