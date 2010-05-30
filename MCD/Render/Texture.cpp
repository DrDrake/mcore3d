#include "Pch.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

Texture::Texture(const Path& fileId)
	:
	Resource(fileId)
{
	clear();
}

Texture::~Texture()
{
	clear();
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

}	// namespace MCD
