#include "Pch.h"
#include "BitmapLoader.h"
#include "TextureLoaderBaseImpl.inc"
#include "../Render/Texture.h"
#include "../Core/System/Log.h"
#include "../Core/System/StaticAssert.h"
#include "../Core/System/StrUtility.h"
#include <memory.h> // For memset

// http://www.spacesimulator.net/tut4_3dsloader.html
// http://www.gamedev.net/reference/articles/article1966.asp

namespace MCD {

class BitmapLoader::LoaderImpl : public TextureLoaderBase::LoaderBaseImpl
{
	// File information header, can be found in Windows.h, but defining inplace
	// can help us to get away from header junk.
	#pragma pack(push, 2)
	typedef struct tagBITMAPFILEHEADER {
		uint16_t bfType;
		uint32_t bfSize;
		uint16_t bfReserved1;
		uint16_t bfReserved2;
		uint32_t bfOffBits;
	} BITMAPFILEHEADER;
	#pragma pack(pop)

	typedef struct tagBITMAPINFOHEADER{
		uint32_t biSize;
		int32_t  biWidth;
		int32_t  biHeight;
		uint16_t biPlanes;
		uint16_t biBitCount;
		uint32_t biCompression;
		uint32_t biSizeImage;
		int32_t  biXPelsPerMeter;
		int32_t  biYPelsPerMeter;
		uint32_t biClrUsed;
		uint32_t biClrImportant;
	} BITMAPINFOHEADER;

public:
	LoaderImpl(BitmapLoader& loader) : LoaderBaseImpl(loader)
	{
		MCD_STATIC_ASSERT(sizeof(BITMAPFILEHEADER) == 14);
		MCD_STATIC_ASSERT(sizeof(BITMAPINFOHEADER) == 40);
	}

	int load(std::istream& is)
	{
		// Windows.h gives us these types to work with the Bitmap files
		BITMAPFILEHEADER fileHeader;
		BITMAPINFOHEADER infoHeader;
		memset(&fileHeader, 0, sizeof(fileHeader));

		// Read the file header
		is.read((char*)&fileHeader, sizeof(fileHeader));

		// Check against the magic 2 bytes.
		// The value of 'BM' in integer is 19778 (assuming little endian)
		if(fileHeader.bfType != 19778u) {
			Log::format(Log::Error, "BitmapLoader: Invalid bitmap header, operation aborted");
			return -1;
		}

		is.read((char*)&infoHeader, sizeof(infoHeader));
		mWidth = infoHeader.biWidth;

		// No aplha channel is supported
		mSrcFormat = GpuDataFormat::get("uintBGR8");	// Note that the source format is BGR but not RGB
		mGpuFormat = GpuDataFormat::get("uintRGB8");

		if(infoHeader.biBitCount != 24) {
			Log::format(Log::Error, "BitmapLoader: Only 24-bit color is supported, operation aborted");
			return -1;
		}

		if(infoHeader.biCompression != 0) {
			Log::format(Log::Error, "BitmapLoader: Compressed bmp is not supported, operation aborted");
			return -1;
		}

		bool flipVertical;
		if(infoHeader.biHeight > 0) {
			mHeight = infoHeader.biHeight;
			flipVertical = true;
		}
		else {
			mHeight = -infoHeader.biHeight;
			flipVertical = false;
		}

		// Memory usage for one row of image
		const size_t rowByte = mWidth * (sizeof(char) * 3);

		mImageData = ImageData(rowByte * mHeight);

		if(!mImageData) {
			Log::format(Log::Error, "BitmapLoader: Corruption of file or not enough memory, operation aborted");
			return -1;
		}

		// At this point we can read every pixel of the image
		for(size_t h = 0; h<mHeight; ++h) {
			// Bitmap file is differ from other image format like jpg and png that
			// the vertical scan line order is inverted.
			const size_t invertedH = flipVertical ? mHeight - 1 - h : h;

			char* p = &mImageData[invertedH * rowByte];
			is.read(p, rowByte);
			if(is.gcount() != std::streamsize(rowByte)) {
				Log::format(Log::Warn, "BitmapLoader: End of file, bitmap data incomplete");
				return 0;
			}
		}

		return 0;
	}
};	// LoaderImpl

BitmapLoader::BitmapLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState BitmapLoader::load(std::istream* is, const Path*, const char*)
{
	MCD_ASSUME(mImpl);

	if(!is)
		return Aborted;

	const int result = static_cast<LoaderImpl*>(mImpl)->load(*is);

	return result == 0 ? Loaded : Aborted;
}

void BitmapLoader::uploadData(Texture& texture)
{
	MCD_ASSUME(mImpl != nullptr);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	MCD_VERIFY(texture.create(
		impl->mGpuFormat, impl->mSrcFormat,
		impl->mWidth, impl->mHeight,
		1, 1,
		impl->mImageData, impl->mImageData.size())
	);
}

ResourcePtr BitmapLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "bmp") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoaderPtr BitmapLoaderFactory::createLoader()
{
	return new BitmapLoader;
}

}	// namespace MCD
