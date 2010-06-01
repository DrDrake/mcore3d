#include "Pch.h"
#include "../Texture.h"
#include "../GpuDataFormat.h"

namespace MCD {

void Texture::clear()
{
}

bool Texture::create(
	const GpuDataFormat& gpuFormat,
	const GpuDataFormat& srcFormat,
	size_t width_, size_t height_,
	size_t mipLevelCount,
	const void* data
)
{
	clear();

	this->format = gpuFormat;
	this->width = width_;
	this->height = height_;
	this->type = GL_TEXTURE_2D;

	return true;
}

}	// namespace MCD
