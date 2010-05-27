#include "Pch.h"
#include "../Texture.h"
#include "../GpuDataFormat.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

void Texture::clear()
{
	// glDeleteTextures will simple ignore non-valid texture handles
	glDeleteTextures(1, &handle);
	handle = 0;
	width = height = 0;
	type = GL_INVALID_ENUM;
	format = GL_INVALID_ENUM;
}

static size_t getMipLevelSize(int format, size_t bytePerPixel, size_t level, size_t& w, size_t& h, bool& isCompressed)
{
#define _max(a, b) (a > b ? a : b)

	for(size_t i=0; i<level; ++i) {
		w = _max(w >> 1, 1);
		h = _max(h >> 1, 1);
	}

	isCompressed = true;

	switch(format) {
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		return 8 * _max(w / 4, 1) * _max(h / 4, 1);
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		return 16 * _max(w / 4, 1) * _max(h / 4, 1); 
	}

	isCompressed = false;
	return w * h * bytePerPixel;

#undef _max
}

bool Texture::create(
	const GpuDataFormat& dataFormat,
	size_t width_, size_t height_,
	size_t mipLevelCount,
	const void* data
)
{
	clear();

	this->format = dataFormat.format;
	this->width = width_;
	this->height = height_;
	this->type = GL_TEXTURE_2D;

	glGenTextures(1, &handle);
	glBindTexture(type, handle);

	const byte_t* levelData = (byte_t*)data;
	for(size_t level=0; level<mipLevelCount; ++level)
	{
		size_t w = width;
		size_t h = height;
		bool isCompressed;
		const size_t levelSize = getMipLevelSize(format, bytePerPixel(format), level, w, h, isCompressed);

		if(isCompressed)
			glCompressedTexImage2D(GL_TEXTURE_2D, level, format, w, h, 0, levelSize, levelData);
		else
			glTexImage2D(GL_TEXTURE_2D, level, format, w, h, 0, format, GL_UNSIGNED_BYTE, levelData);

		levelData += levelSize;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipLevelCount - 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(mipLevelCount > 1) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glBindTexture(type, 0);

	return true;
}

}	// namespace MCD
