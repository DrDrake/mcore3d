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
	format = GpuDataFormat();
}

static size_t _max(size_t a, size_t b) { return a > b ? a : b; }

static size_t getMipLevelSize(int format, size_t bytePerPixel, size_t level, size_t& w, size_t& h, bool& isCompressed)
{
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
}

bool Texture::create(
	const GpuDataFormat& gpuFormat,
	const GpuDataFormat& srcFormat,
	size_t width_, size_t height_,
	size_t surfaceCount, size_t mipLevelCount,
	const void* data, size_t dataSize
)
{
	MCD_ASSERT(data);

	if(surfaceCount != 1 && surfaceCount != 6)
		return false;

	clear();

	this->format = gpuFormat;
	this->width = width_;
	this->height = height_;
	this->type = surfaceCount == 1 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;

	glEnable(type);
	glGenTextures(1, &handle);
	glBindTexture(type, handle);

	const byte_t* surfaceData = (byte_t*)data;
	for(size_t surface=0; surface<surfaceCount; ++surface)
	{
		const byte_t* levelData = (byte_t*)surfaceData;
		for(size_t level=0; level<mipLevelCount; ++level)
		{
			size_t w = width;
			size_t h = height;
			bool isCompressed;
			const size_t levelSize = getMipLevelSize(format.format, format.sizeInByte(), level, w, h, isCompressed);

			const int textureType = surfaceCount == 1 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP_POSITIVE_X + surface;

			if(isCompressed)
				glCompressedTexImage2D(textureType, level, format.format, w, h, 0, levelSize, levelData);
			else {
				// NOTE: To compress texture on the fly, just pass GL_COMPRESSED_XXX_ARB as the internal format
				// Reference: www.oldunreal.com/editing/s3tc/ARB_texture_compression.pdf
				glTexImage2D(textureType, level, format.format, w, h, 0, srcFormat.components, format.dataType, levelData);
			}

			levelData += levelSize;
		}
		surfaceData += dataSize / surfaceCount;
	}

	glTexParameteri(type, GL_TEXTURE_MAX_LEVEL, mipLevelCount - 1);

	// Auto mip-map generation
	// Reference on comparison between gluBuild2DMipmaps / GL_GENERATE_MIPMAP and glGenerateMipmapEXT
	// http://www.gamedev.net/community/forums/topic.asp?topic_id=452780
	// http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=233955
	if(true) {
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
		glTexParameteri(type, GL_GENERATE_MIPMAP, GL_TRUE);
	}
	else
		glTexParameteri(type, GL_GENERATE_MIPMAP, GL_FALSE);

	if(surfaceCount == 1) {
		glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else {
		MCD_ASSERT(type == GL_TEXTURE_CUBE_MAP);
		// NOTE: Use GL_CLAMP_TO_EDGE, otherwise there will be seams when using GL_LINEAR
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	if(mipLevelCount > 1) {
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glBindTexture(type, 0);

	return true;
}

}	// namespace MCD
