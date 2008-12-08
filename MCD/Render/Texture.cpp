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

}	// namespace MCD
