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
	static size_t& width(Texture& texture) {
		return texture.mWidth;
	}
	static size_t& height(Texture& texture) {
		return texture.mHeight;
	}
};	// PrivateAccessor

bool TextureRenderBuffer::bind(RenderTarget& renderTarget)
{
	// Create an empty texture if we didn't have one already.
	if(!texture) {
		if((texture = new Texture(L"TextureRenderBuffer:")) == nullptr)
			return false;

		typedef Texture::PrivateAccessor<TextureRenderBuffer> Accessor;
		size_t width = renderTarget.width();
		size_t height = renderTarget.height();
		GLuint* handle = reinterpret_cast<GLuint*>(&Accessor::handle(*texture));
		MCD_ASSUME(handle);

		Accessor::width(*texture) = width;
		Accessor::height(*texture) = height;
		Accessor::format(*texture) = GL_RGBA;

//		glEnable(GL_TEXTURE_RECTANGLE_ARB);

		glGenTextures(1, handle);
		glBindTexture(GL_TEXTURE_2D, *handle);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
			0, GL_RGBA, GL_INT, nullptr);
	}

	renderTarget.bind();
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
		GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture->handle(), 0/*mipmap level*/);

	// Assure the texture that is binded to the render target is not
	// to be read as texture during rendering.
	glBindTexture(GL_TEXTURE_2D, 0);

	addOwnerShipTo(renderTarget);

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
