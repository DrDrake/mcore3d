#include "Pch.h"
#include "CubemapLoader.h"
#include "Texture.h"
#include "TextureLoaderBaseImpl.inc"
#include "../Core/System/Log.h"
#include "../Core/System/StaticAssert.h"
#include "../../3Party/glew/glew.h"
#include <memory.h> // For memset

// http://www.spacesimulator.net/tut4_3dsloader.html
// http://www.gamedev.net/reference/articles/article1966.asp

namespace MCD {

class CubemapLoader::LoaderImpl : public TextureLoaderBase::LoaderBaseImpl
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
	LoaderImpl(CubemapLoader& loader)
		:
		LoaderBaseImpl(loader)
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
			Log::format(Log::Error, L"CubemapLoader: Invalid bitmap header, operation aborted");
			return -1;
		}

		is.read((char*)&infoHeader, sizeof(infoHeader));
		mWidth = infoHeader.biWidth;

		// Only RGB is supported
		mFormat = GL_RGB;

		if(infoHeader.biBitCount != 24) {
			Log::format(Log::Error, L"CubemapLoader: Only 24-bit color is supported, operation aborted");
			return -1;
		}

		if(infoHeader.biCompression != 0) {
			Log::format(Log::Error, L"CubemapLoader: Compressed bmp is not supported, operation aborted");
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

		mImageData = new byte_t[rowByte * mHeight];

		if(!mImageData) {
			Log::format(Log::Error, L"CubemapLoader: Corruption of file or not enough memory, operation aborted");
			return -1;
		}

		// At this point we can read every pixel of the image
		for(size_t h = 0; h<mHeight; ++h) {
			// Bitmap file is differ from other image format like jpg and png that
			// the vertical scan line order is inverted.
			const size_t invertedH = flipVertical ? mHeight - 1 - h : h;

			byte_t* p = &mImageData[invertedH * rowByte];
			is.read((char*)p, rowByte);
			if(is.gcount() != std::streamsize(rowByte)) {
				Log::format(Log::Warn, L"CubemapLoader: End of file, bitmap data incomplete");
				return 0;
			}
		}

		return 0;
	}

	void upload()
	{
		glTexImage2D(GL_TEXTURE_2D, 0, mFormat, mWidth, mHeight,
			0, GL_BGR, GL_UNSIGNED_BYTE, &mImageData[0]);	// Note that the external format is GL_BGR but not GL_RGB
	}
};	// LoaderImpl

CubemapLoader::CubemapLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState CubemapLoader::load(std::istream* is, const Path*)
{
	MCD_ASSUME(mImpl != nullptr);

	loadingState = is ? NotLoaded : Aborted;

	if(loadingState & Stopped)
		return loadingState;

	// There is no need to do a mutex lock during loading, since
	// no body can access the mImageData if the loading isn't finished.
	int result = static_cast<LoaderImpl*>(mImpl)->load(*is);

	ScopeLock lock(mImpl->mMutex);

	if(result != 0)
		loadingState = Aborted;
	else
		loadingState = Loaded;

	return loadingState;
}

void CubemapLoader::uploadData()
{
	MCD_ASSUME(mImpl != nullptr);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	impl->upload();
}

}	// namespace MCD