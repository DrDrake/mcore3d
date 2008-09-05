#include "Pch.h"
#include "TextureRenderBuffer.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

TextureRenderBuffer::TextureRenderBuffer()
	: texture(nullptr)
{
}

TextureRenderBuffer::TextureRenderBuffer(const TexturePtr& tex)
	: texture(tex)
{
}

bool TextureRenderBuffer::linkTo(RenderTarget& renderTarget)
{
	// Create an empty texture if we didn't have one already.
	// We use GL_TEXTURE_RECTANGLE_ARB as the default rather than GL_TEXTURE because
	// some display card didn't support non-power of 2 texture.
	if(!texture && !createTexture(renderTarget.width(), renderTarget.height(), GL_TEXTURE_RECTANGLE_ARB, GL_RGB))
		return false;

	if(texture->width() != renderTarget.width() || texture->height() != renderTarget.height())
		return false;

	renderTarget.bind();
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
		GL_COLOR_ATTACHMENT0_EXT, texture->type(), texture->handle(), 0/*mipmap level*/);
	renderTarget.unbind();

	addOwnerShipTo(renderTarget);

	return true;
}

template<>
class Texture::PrivateAccessor<TextureRenderBuffer>
{
public:
	static uint& handle(Texture& texture) {
		return texture.mHandle;
	}
	static int& format(Texture& texture) {
		return texture.mFormat;
	}
	static int& type(Texture& texture) {
		return texture.mType;
	}
	static size_t& width(Texture& texture) {
		return texture.mWidth;
	}
	static size_t& height(Texture& texture) {
		return texture.mHeight;
	}
};	// PrivateAccessor

bool TextureRenderBuffer::createTexture(size_t width, size_t height, int type, int format)
{
	if((texture = new Texture(L"TextureRenderBuffer:")) == nullptr)
		return false;

	if(type != GL_TEXTURE_2D && type != GL_TEXTURE_RECTANGLE_ARB)
		return false;

	typedef Texture::PrivateAccessor<TextureRenderBuffer> Accessor;
	GLuint* handle = reinterpret_cast<GLuint*>(&Accessor::handle(*texture));
	MCD_ASSUME(handle);

	Accessor::width(*texture) = width;
	Accessor::height(*texture) = height;
	Accessor::format(*texture) = GL_RGBA;
	Accessor::type(*texture) = type;

	glEnable(type);
	glGenTextures(1, handle);
	texture->bind();

	glTexParameterf(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(texture->type(), 0, format, width, height,
		0, format, GL_UNSIGNED_BYTE, nullptr);

	// Assure the texture that is binded to the render target is not
	// to be read as texture during rendering.
	glBindTexture(type, 0);

	return true;
}

size_t TextureRenderBuffer::width() const
{
	return texture ? texture->width() : 0;
}

size_t TextureRenderBuffer::height() const
{
	return texture ? texture->height() : 0;
}

}	// namespace MCD
