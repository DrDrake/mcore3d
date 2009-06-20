#include "Pch.h"
#include "TextureRenderBuffer.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

TextureRenderBuffer::TextureRenderBuffer(int type)
	: texture(nullptr), mAttachmentType(type)
{
}

TextureRenderBuffer::TextureRenderBuffer(const TexturePtr& tex)
	: texture(tex)
{
	// TODO: Get the type from tex
	mAttachmentType = GL_DEPTH_ATTACHMENT_EXT;
	// GL_RGB,				GL_COLOR_ATTACHMENT0_EXT
	// GL_DEPTH_COMPONENT,	GL_DEPTH_ATTACHMENT_EXT
}

bool TextureRenderBuffer::linkTo(RenderTarget& renderTarget)
{
	if(!texture || texture->width != renderTarget.width() || texture->height != renderTarget.height())
		return false;

	renderTarget.bind();
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
		mAttachmentType, texture->type, texture->handle, 0/*mipmap level*/);
	renderTarget.unbind();

	addOwnerShipTo(renderTarget);

	return true;
}

bool TextureRenderBuffer::createTexture(size_t width, size_t height, int type, int format, const wchar_t* name)
{
	if((texture = new Texture(name ? name : L"TextureRenderBuffer:")) == nullptr)
		return false;

	if(type != GL_TEXTURE_2D && type != GL_TEXTURE_RECTANGLE_ARB)
		return false;

	texture->width = width;
	texture->height = height;
	texture->format = format;
	texture->type = type;

	glEnable(type);
	glGenTextures(1, &texture->handle);
	texture->bind();

	glTexParameterf(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameterf(type, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
//	glTexParameterf(type, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glTexImage2D(texture->type, 0, format, width, height,
		0, format, GL_INT, nullptr);

	// Assure the texture that is binded to the render target is not
	// to be read as texture during rendering.
	glBindTexture(type, 0);

	return true;
}

bool TextureRenderBuffer::create(size_t width, size_t height, int type, int format, int components, int dataType, const wchar_t* name)
{
	if((texture = new Texture(name ? name : L"TextureRenderBuffer:")) == nullptr)
		return false;

	if(type != GL_TEXTURE_2D && type != GL_TEXTURE_RECTANGLE_ARB)
		return false;

	texture->width = width;
	texture->height = height;
	texture->format = format;
	texture->type = type;

	glEnable(type);
	glGenTextures(1, &texture->handle);
	texture->bind();

	glTexParameterf(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(texture->type, 0, format, width, height, 0, components, dataType, nullptr);

	// Assure the texture that is binded to the render target is not
	// to be read as texture during rendering.
	glBindTexture(type, 0);

	return true;
}

size_t TextureRenderBuffer::width() const
{
	return texture ? texture->width : 0;
}

size_t TextureRenderBuffer::height() const
{
	return texture ? texture->height : 0;
}

}	// namespace MCD
