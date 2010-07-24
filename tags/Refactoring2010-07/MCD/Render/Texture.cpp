#include "Pch.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

Texture::Texture(const Path& fileId)
	:
	Resource(fileId), handle(0)
{
	clear();
}

Texture::~Texture()
{
	clear();
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
