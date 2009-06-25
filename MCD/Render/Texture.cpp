#include "Pch.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

Texture::Texture(const Path& fileId)
	:
	Resource(fileId),
	handle(0),
	width(0), height(0),
	type(GL_INVALID_ENUM),
	format(GL_INVALID_ENUM)
{
}

Texture::~Texture()
{
	// glDeleteTextures will simple ignore non-valid texture handles
	glDeleteTextures(1, &handle);
}

void Texture::bind() const
{
	if(isValid())
		glBindTexture(type, handle);
	else
		glBindTexture(GL_TEXTURE_2D, handle);
}

void Texture::unbind() const
{
	if(isValid())
		glBindTexture(type, 0);
	else
		glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::isValid() const
{
	return (type != GL_INVALID_ENUM);
}

bool Texture::hasAlpha(int format)
{
	return (
		format == GL_ALPHA ||
		format == GL_RGBA ||
		format == GL_LUMINANCE_ALPHA ||
		format == GL_COMPRESSED_ALPHA_ARB ||
		format == GL_COMPRESSED_RGBA_ARB ||
		format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
		format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
		format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
}

int Texture::bytePerPixel(int format)
{
	switch(format)
	{
	case GL_BGR:
	case GL_RGB:
		return 3;

	case GL_RGBA:
		return 4;

	case GL_LUMINANCE:
		return 1;

	case GL_LUMINANCE_ALPHA:
		return 2;

	default:
		return 0;
	}
}

bool Texture::autoGenMipmapEnabled()
{
    return true;
}

bool Texture::dataTypeAndComponents(int textureFmt, int &outDataType, int &outComponents)
{
	switch(textureFmt)
	{
	// unsigned int8 with rgba components
	case GL_RGBA8:
		{outDataType = GL_UNSIGNED_BYTE; outComponents = GL_RGBA;} return true;
	// float16 with rgba components
	case GL_RGBA16F_ARB:
		{outDataType = GL_HALF_FLOAT_ARB; outComponents = GL_BGRA;} return true;
	// float16 with rg components
	case GL_LUMINANCE_ALPHA16F_ARB:
		{outDataType = GL_HALF_FLOAT_ARB; outComponents = GL_LUMINANCE_ALPHA;} return true;
	// float16 with r component
	case GL_LUMINANCE16F_ARB:
		{outDataType = GL_HALF_FLOAT_ARB; outComponents = GL_LUMINANCE;} return true;
	// float32 with rgba components
	case GL_RGBA32F_ARB:
		{outDataType = GL_FLOAT; outComponents = GL_BGRA;} return true;
	// float32 with rg components
	case GL_LUMINANCE_ALPHA32F_ARB:
		{outDataType = GL_FLOAT; outComponents = GL_LUMINANCE_ALPHA;} return true;
	// float32 with r component
	case GL_LUMINANCE32F_ARB:
		{outDataType = GL_FLOAT; outComponents = GL_LUMINANCE;} return true;
	// 16-bit depth component
	case GL_DEPTH_COMPONENT16:
		{outDataType = GL_UNSIGNED_INT; outComponents = GL_DEPTH_COMPONENT;} return true;
	// 24-bit depth component
	case GL_DEPTH_COMPONENT24:
		{outDataType = GL_UNSIGNED_INT; outComponents = GL_DEPTH_COMPONENT;} return true;
	// 32-bit depth component
	case GL_DEPTH_COMPONENT32:
		{outDataType = GL_UNSIGNED_INT; outComponents = GL_DEPTH_COMPONENT;} return true;
	}

	return false;
}

}	// namespace MCD
