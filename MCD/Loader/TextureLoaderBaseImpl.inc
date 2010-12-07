// To be included in cpp only.
// See also TextureLoaderBase.h

#include "TextureLoaderBase.h"
#include "../Render/GpuDataFormat.h"
#include "../Core/System/Mutex.h"

namespace MCD {

class MCD_LOADER_API TextureLoaderBase::LoaderBaseImpl
{
public:
	//! Handly class for managing ownership of the image data
	struct ImageData
	{
		ImageData() : mImageData(nullptr), mSize(0) {}

		explicit ImageData(size_t size) {
			MCD_ASSERT(size != 0);
			mImageData = (char*)malloc(size);
			mSize = mImageData ? size : 0;
		};

		//! Will transfer ownership from rhs to this
		ImageData(ImageData& rhs) {
			*this = rhs;
		}

		//! Will transfer ownership from rhs to this
		ImageData& operator=(const ImageData& rhs)
		{
			mImageData = rhs.mImageData;
			const_cast<ImageData&>(rhs).mImageData = nullptr;
			mSize = rhs.mSize;
			const_cast<ImageData&>(rhs).mSize = 0;
			return *this;
		}

		char& operator[](size_t i) {
			return mImageData[i];
		}

		~ImageData() { clear(); }

		void clear()
		{
			free(mImageData);
			mImageData = nullptr;
			mSize = 0;
		}

		ImageData clone() const
		{
			MCD_ASSERT(mSize != 0);
			ImageData ret(mSize);
			::memcpy(ret.mImageData, mImageData, mSize);
			return ret;
		}

		operator char*() { return mImageData; }
		operator const char*() const { return mImageData; }
		size_t size() const { return mSize; }

		char* mImageData;
		size_t mSize;
	};	// ImageData

	LoaderBaseImpl(TextureLoaderBase& loader);
	virtual ~LoaderBaseImpl();

	void genMipmap();

	TextureLoaderBase& mLoader;
	ImageData mImageData;	///< byte array which holds the pixels
	size_t mWidth;			///< width of the image
	size_t mHeight;			///< height of the image
	size_t mMipLevels;
	GpuDataFormat mGpuFormat, mSrcFormat;

	Mutex mMutex;
};	// LoaderBaseImpl

}	// namespace MCD
