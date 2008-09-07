#include "Pch.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

Texture::Texture(const Path& fileId)
	:
	Resource(fileId),
	mHandle(0),
	mWidth(0), mHeight(0),
	mType(GL_INVALID_ENUM),
	mFormat(GL_INVALID_ENUM)
{
}

Texture::~Texture()
{
	// glDeleteTextures will simple ignore non-valid texture handles
	glDeleteTextures(1, &mHandle);
}

void Texture::bind()
{
	if(mType != GL_INVALID_ENUM)
		glBindTexture(mType, mHandle);
	else
		glBindTexture(GL_TEXTURE_2D, mHandle);
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
