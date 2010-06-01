#include "Pch.h"
#include "TgaLoader.h"
#include "Texture.h"
#include "TextureLoaderBaseImpl.inc"
#include "../Core/System/Log.h"

// Reference: http://www.flashbang.se/download/nehe_tga.rar

namespace MCD {

class TgaLoader::LoaderImpl : public TextureLoaderBase::LoaderBaseImpl
{
public:
	LoaderImpl(TgaLoader& loader)
		:
		LoaderBaseImpl(loader)
	{
	}

	int load(std::istream& is)
	{
		char header[18] = {0};

		// Read the file header
		is.read(header, sizeof(header));

		// Should be image type 2 (color) or type 10 (rle compressed color)
		if(header[2] != 2 && header[2] != 10) {
			Log::format(Log::Error, "TgaLoader: Invalid tga header, operation aborted");
			return -1;
		}

		// If there is an image ID section then skip over it
		if(header[0])
			is.seekg(header[0], std::ios_base::cur);

		// Get the size and bitdepth from the header
		mWidth = header[13] * 256 + header[12];
		mHeight = header[15] * 256 + header[14];
		const size_t bytePerPixel = header[16] / 8;

		if(bytePerPixel == 3) {
			mSrcFormat = GpuDataFormat::get("uintBGR8");	// Note that the source format is BGR but not RGB
			mGpuFormat = GpuDataFormat::get("uintRGB8");
		}
		else if(bytePerPixel == 4) {
			mSrcFormat = GpuDataFormat::get("uintBGRA8");	// Note that the source format is BGR but not RGB
			mGpuFormat = GpuDataFormat::get("uintRGBA88");
		}
		else {
			Log::format(Log::Error, "TgaLoader: Invalid bit-per pixel, operation aborted");
			return -1;
		}

		// Memory usage for one row of image
		const size_t rowByte = mWidth * (sizeof(char) * bytePerPixel);
		const size_t imageByte = rowByte * mHeight;

		mImageData = ImageData(imageByte);

		if(!mImageData) {
			Log::format(Log::Error, "TgaLoader: Corruption of file or not enough memory, operation aborted");
			return -1;
		}

		// Read the uncompressed image data if type 2
		if(header[2] == 2)
		{
			is.read(mImageData, sizeof(char) * imageByte);
		}

		// Read the compressed image data if type 10
		if(header[2] == 10)
		{
			size_t i = 0;
			while(i < imageByte)
			{
				// Reads the the RLE header
				char rle;
				is.read(&rle, 1);

				// If the rle header is below 128 it means that what folows is just raw data with rle+1 pixels
				if(rle<128)
				{
					is.read(&mImageData[i], bytePerPixel * (rle + 1));
					i += bytePerPixel * (rle + 1);
				}

				// If the rle header is equal or above 128 it means that we have a string of rle-127 pixels
				// that use the folowing pixels color
				else
				{
					// Read what color we should use
					char color[4];
					is.read(&color[0], bytePerPixel);

					// Insert the color stored in tmp into the folowing rle-127 pixels
					int j = 0;
					while(j < rle - 127)
					{
						mImageData[i] = color[0];
						mImageData[i+1] = color[1];
						mImageData[i+2] = color[2];

						if(bytePerPixel == 4)
							mImageData[i+3] = color[3];

						i += bytePerPixel;
						++j;
					}
				}
			}
		}

		// Flip the image in the y-direction as in bimap image.
		for(size_t i=0; i<mHeight/2; ++i) {
			char* row1 = mImageData + i * rowByte;
			char* row2 = mImageData + (mHeight - 1 - i) * rowByte;
			// Swap 2 rows
			for(size_t j=0; j<rowByte; ++j)
				row1[j] ^= row2[j] ^= row1[j] ^= row2[j];
		}

		return 0;
	}
};	// LoaderImpl

TgaLoader::TgaLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState TgaLoader::load(std::istream* is, const Path*, const char*)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mMutex);

	if(!is)
		loadingState = Aborted;
	else if(loadingState == Aborted)
		loadingState = NotLoaded;

	if(loadingState & Stopped)
		return loadingState;

	int result;
	{	// There is no need to do a mutex lock during loading, since
		// no body can access the mImageData if the loading isn't finished.

		ScopeUnlock unlock(mImpl->mMutex);
		result = static_cast<LoaderImpl*>(mImpl)->load(*is);
	}

	return (loadingState = (result == 0) ? Loaded : Aborted);
}

void TgaLoader::uploadData(Texture& texture)
{
	MCD_ASSUME(mImpl != nullptr);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	MCD_VERIFY(texture.create(impl->mGpuFormat, impl->mSrcFormat, impl->mWidth, impl->mHeight, 1, impl->mImageData));
}

}	// namespace MCD
