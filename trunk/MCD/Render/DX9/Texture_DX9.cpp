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

static bool copyToGpu(const GpuDataFormat& srcFormat, GpuDataFormat& destFormat, size_t width, size_t height, byte_t* inData, byte_t* outData, size_t outDataPitch)
{
	// No need to convert if the formats are compressed format
	if(srcFormat == destFormat && srcFormat.isCompressed && destFormat.isCompressed)
		return true;

	byte_t* rowDataDest = outData;

	// RGBA to ARGB
	if(srcFormat == GpuDataFormat::get("uintRGBA8") && srcFormat == destFormat) {
		destFormat = GpuDataFormat::get("uintARGB8");

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
	// RGB to ARGB
	else if(srcFormat == GpuDataFormat::get("uintRGB8") && srcFormat == destFormat) {
		destFormat = GpuDataFormat::get("uintARGB8");

		for(size_t i=0; i<height; ++i) {
			byte_t* data = rowDataDest;
			for(size_t j=0; j<width; ++j, inData += srcFormat.sizeInByte(), data += destFormat.sizeInByte()) {
				data[0] = inData[2];	// B <- R
				data[1] = inData[1];	// G <- G
				data[2] = inData[0];	// R <- B
				data[3] = 255;		// A
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

bool Texture::create(
	const GpuDataFormat& gpuFormat,
	const GpuDataFormat& srcFormat,
	size_t width_, size_t height_,
	size_t surfaceCount, size_t mipLevelCount,
	const void* data, size_t dataSize,
	int apiSpecificflags
)
{
	if(surfaceCount != 1 && surfaceCount != 6)
		return false;
	if(surfaceCount == 6 && width_ != height_)
		return false;

	clear();

	this->format = gpuFormat;
	this->width = width_;
	this->height = height_;

	GpuDataFormat adjustedFormat = format.isCompressed ? format : GpuDataFormat::get("uintARGB8");

	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	const D3DPOOL pool = (apiSpecificflags & D3DUSAGE_RENDERTARGET) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;

	if(pool == D3DPOOL_DEFAULT)
		registerDefaultPoolTexture(*this);

	if(surfaceCount == 1) {
		IDirect3DTexture9*& texture = reinterpret_cast<IDirect3DTexture9*&>(handle);

		if(S_OK != device->CreateTexture(
			width_, height_,
			mipLevelCount,
			apiSpecificflags,	// Usage
			static_cast<D3DFORMAT>(adjustedFormat.format),
			pool,
			&texture,
			nullptr	// SharedHandle, reserved
		))
			return false;

		MCD_ASSUME(texture);

		if(data && dataSize > 0) {
			D3DLOCKED_RECT lockedRect;
			for(size_t level=0; level<mipLevelCount; ++level) {
				if(S_OK != texture->LockRect(level, &lockedRect, nullptr, 0))
					return false;

				if(!copyToGpu(srcFormat, this->format, width, height, (byte_t*)data, (byte_t*)lockedRect.pBits, lockedRect.Pitch))
					return false;

				if(S_OK != texture->UnlockRect(level))
					return false;
			}
		}
	}
	else if(surfaceCount == 6) {
		IDirect3DCubeTexture9*& texture = reinterpret_cast<IDirect3DCubeTexture9*&>(handle);

		if(S_OK != device->CreateCubeTexture(
			width_,
			mipLevelCount,
			apiSpecificflags,	// Usage
			static_cast<D3DFORMAT>(adjustedFormat.format),
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
