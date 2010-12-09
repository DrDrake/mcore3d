#include "Pch.h"
#include "../Texture.h"
#include "Renderer.inc"
#include "../GpuDataFormat.h"

#include <d3d9.h>

namespace MCD {

void Texture::bind(size_t textureUnit) const
{
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);
	if(IDirect3DBaseTexture9* texture = reinterpret_cast<IDirect3DBaseTexture9*>(handle))
		MCD_VERIFY(device->SetTexture(textureUnit, texture) == D3D_OK);
}

void Texture::unbind(size_t textureUnit) const
{
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);
	MCD_VERIFY(device->SetTexture(textureUnit, nullptr) == D3D_OK);
}

void Texture::clear()
{
	if(IDirect3DTexture9*& texture = reinterpret_cast<IDirect3DTexture9*&>(handle))
		texture->Release();

	handle = 0;
	width = height = 0;
	format = GpuDataFormat::get("none");
	type = 0;	// NOTE: Not used in DirectX
}

// Defined in RenderWindow.Win.inc
extern void registerDefaultPoolTexture(Texture& texture);

// To unify the texture type among other renderer API
static GpuDataFormat adjustFormat(const GpuDataFormat& format)
{
	if(format == GpuDataFormat::get("uintRGBA8"))
		return GpuDataFormat::get("uintARGB8");
	if(format == GpuDataFormat::get("uintRGB8"))
		return GpuDataFormat::get("uintARGB8");
	return format;
}

static bool copyToGpu(const GpuDataFormat& srcFormat, const GpuDataFormat& destFormat, size_t width, size_t height, byte_t* inData, byte_t* outData, size_t outDataPitch)
{
	// No need to convert if the formats are compressed format
	if(srcFormat == destFormat && srcFormat.isCompressed && destFormat.isCompressed)
		return true;

	byte_t* rowDataDest = outData;

	// RGBA to BGRA
	if(srcFormat == GpuDataFormat::get("uintRGBA8") && (destFormat == GpuDataFormat::get("uintARGB8"))) {
		for(size_t i=0; i<height; ++i) {
			byte_t* data = rowDataDest;
			for(size_t j=0; j<width; ++j, inData += srcFormat.sizeInByte(), data += destFormat.sizeInByte()) {
				data[0] = inData[2];	// B <- R
				data[1] = inData[1];	// G <- G
				data[2] = inData[0];	// R <- B
				data[3] = inData[3];	// A <- A
			}
			rowDataDest += outDataPitch;
		}
		return true;
	}
	// RGB to BGRA
	else if(srcFormat == GpuDataFormat::get("uintRGB8") && (destFormat == GpuDataFormat::get("uintARGB8"))) {
		for(size_t i=0; i<height; ++i) {
			byte_t* data = rowDataDest;
			for(size_t j=0; j<width; ++j, inData += srcFormat.sizeInByte(), data += destFormat.sizeInByte()) {
				data[0] = inData[2];	// B <- R
				data[1] = inData[1];	// G <- G
				data[2] = inData[0];	// R <- B
				data[3] = 255;			// A
			}
			rowDataDest += outDataPitch;
		}
		return true;
	}
	else {
		// Perform simple copy
		for(size_t i=0; i<height; ++i) {
			byte_t* data = rowDataDest;
			for(size_t j=0; j<width; ++j, inData += srcFormat.sizeInByte(), data += destFormat.sizeInByte()) {
				memcpy(data, inData, destFormat.sizeInByte());
			}
			rowDataDest += outDataPitch;
		}
	}

	return true;
};

static size_t _max(size_t a, size_t b) { return a > b ? a : b; }

static size_t getMipLevelSize(int format, size_t bytePerPixel, size_t level, size_t& w, size_t& h)
{
	for(size_t i=0; i<level; ++i) {
		w = _max(w >> 1, 1);
		h = _max(h >> 1, 1);
	}

	return w * h * bytePerPixel;
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
	if(surfaceCount != 1 && surfaceCount != 6)
		return false;
	if(surfaceCount == 6 && width_ != height_)
		return false;

	clear();

	this->format = adjustFormat(gpuFormat);
	this->width = width_;
	this->height = height_;

	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	const D3DPOOL pool = (apiSpecificflags & D3DUSAGE_RENDERTARGET) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;

	if(pool == D3DPOOL_DEFAULT)
		registerDefaultPoolTexture(*this);

	// Auto mip-map generation
	if(mipLevelCount == 0) {
		size_t s = width < height ? width : height;
		for(;s >= 1; s /= 2) ++mipLevelCount;
	}

	if(surfaceCount == 1) {
		IDirect3DTexture9*& texture = reinterpret_cast<IDirect3DTexture9*&>(handle);

		if(S_OK != device->CreateTexture(
			width_, height_,
			mipLevelCount,
			apiSpecificflags,	// Usage
			static_cast<D3DFORMAT>(format.format),
			pool,
			&texture,
			nullptr	// SharedHandle, reserved
		))
			return false;

		MCD_ASSUME(texture);

		if(data && dataSize > 0)
		{
			D3DLOCKED_RECT lockedRect;
			format = adjustFormat(format);

			for(size_t level=0; level<mipLevelCount; ++level) {
				if(S_OK != texture->LockRect(level, &lockedRect, nullptr, 0))
					return false;

				size_t w = width, h = height;
				const size_t levelSize = getMipLevelSize(format.format, srcFormat.sizeInByte(), level, w, h);
				if(!copyToGpu(srcFormat, format, w, h, (byte_t*)data, (byte_t*)lockedRect.pBits, lockedRect.Pitch))
					return false;

				data += levelSize;

				if(S_OK != texture->UnlockRect(level))
					return false;
			}
		}
	}
	else if(surfaceCount == 6)
	{
		IDirect3DCubeTexture9*& texture = reinterpret_cast<IDirect3DCubeTexture9*&>(handle);

		if(S_OK != device->CreateCubeTexture(
			width_,
			mipLevelCount,
			apiSpecificflags,	// Usage
			static_cast<D3DFORMAT>(format.format),
			pool,
			&texture,
			nullptr	// SharedHandle, reserved
		))
			return false;

		MCD_ASSUME(texture);
	}
	else
		return false;

	return true;
}

}	// namespace MCD
