#include "Pch.h"
#include "../Texture.h"
#include "../GpuDataFormat.h"

namespace MCD {

void Texture::bind(size_t textureUnit) const {}

void Texture::unbind(size_t textureUnit) const {}

void Texture::clear()
{
	handle = 0;
	width = height = 0;
	type = 0;
	format = GpuDataFormat::get("none");
}

bool Texture::create(
	const GpuDataFormat& gpuFormat,
	const GpuDataFormat& srcFormat,
	size_t width_, size_t height_,
	size_t surfaceCount, size_t mipLevelCount,
	const char* data, size_t dataSize,
	int apiSpecificflags
)
{
	return true;
}

}	// namespace MCD
