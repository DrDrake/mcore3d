#include "Pch.h"
#include "PvrLoader.h"
#include "../Render/Texture.h"
#include "../Render/TextureLoaderBaseImpl.inc"
#include "../Core/Math/BasicFunction.h"
#include "../Core/System/Log.h"
#include "../Core/System/StrUtility.h"
#include "../../3Party/PowerVR/PVRTDecompress.h"
#include "../../3Party/PowerVR/PVRTTexture.h"

#undef max

namespace MCD {

class PvrLoader::LoaderImpl : public TextureLoaderBase::LoaderBaseImpl
{
public:
	LoaderImpl(PvrLoader& loader)
		:
		LoaderBaseImpl(loader)
	{
	}

	// TODO: Many dirty code and not enough error checking.
	int load(std::istream& is)
	{
		// Read the file header
		is.read((char*)&header, sizeof(header));

		// Perform checks for old PVR psPVRHeader
		if(header.dwHeaderSize != sizeof(PVR_Texture_Header))
			return -1;

		mWidth = header.dwWidth;
		mHeight = header.dwHeight;

		switch(header.dwpfFlags & PVRTEX_PIXELTYPE)
		{
		case OGL_RGBA_8888:
			mGpuFormat = mSrcFormat = GpuDataFormat::get("uintRGBA8");
			break;
		case OGL_RGB_888:
			mGpuFormat = mSrcFormat = GpuDataFormat::get("uintRGB8");
			break;
		case OGL_PVRTC2:
		case OGL_PVRTC4:
			mGpuFormat = mSrcFormat = GpuDataFormat::get("uintRGBA8");
			break;
		default:
			// TODO: Support more formats
			Log::format(Log::Error, "PvrLoader: Unsupported texture format");
			return -1;
		}

		// TODO: dwTextureDataSize only specify for size of one surface, Support multiple surface, 
		mImageData = ImageData(header.dwTextureDataSize);

		if(!mImageData) {
			Log::format(Log::Error, "PvrLoader: Corruption of file or not enough memory, operation aborted");
			return -1;
		}

		is.read(mImageData, header.dwTextureDataSize);
		if(is.gcount() != int(header.dwTextureDataSize)) {
			Log::format(Log::Warn, "PvrLoader: End of file, pixel data incomplete");
			return 0;
		}

		// Decompress the PVRTC format on platforms without native hardware support
		const int pixelFormat = header.dwpfFlags & PVRTEX_PIXELTYPE;
		if(pixelFormat == OGL_PVRTC2 || pixelFormat == OGL_PVRTC4)
		{
			const bool twoBitMode = (pixelFormat == OGL_PVRTC2);

			// Calculate the total memory required for all the mip-map levels
			size_t memSize = 0;
			for(size_t i=0; i<header.dwMipMapCount + 1; ++i) {
				const size_t factor = 1 << i;
				memSize += (mWidth * mHeight * 4) / (factor * factor);
			}

			ImageData deCompressed(memSize);

			char* srcPtr = mImageData;
			char* destPtr = deCompressed;
			size_t sizeX = mWidth, sizeY = mHeight;

			for(size_t i=0; i<header.dwMipMapCount + 1; ++i,
				sizeX = Math<size_t>::max(sizeX/2, 1u), sizeY = Math<size_t>::max(sizeY/2, 1u))
			{
				PVRTDecompressPVRTC(srcPtr, twoBitMode, sizeX, sizeY, (unsigned char*)destPtr);

				const size_t minTexWidth = twoBitMode ? PVRTC2_MIN_TEXWIDTH : PVRTC4_MIN_TEXWIDTH;
				const size_t minTexHeight = twoBitMode ? PVRTC2_MIN_TEXHEIGHT : PVRTC4_MIN_TEXHEIGHT;

				const size_t compressedSize =
					Math<size_t>::max(sizeX, minTexWidth) *
					Math<size_t>::max(sizeY, minTexHeight) * header.dwBitCount / 8;

				srcPtr += compressedSize;

				const size_t deCompressedSize = sizeX * sizeY * 4;
				destPtr += deCompressedSize;
			}

			mImageData = deCompressed;
		}

		return 0;
	}

	PVR_Texture_Header header;
};	// LoaderImpl

PvrLoader::PvrLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState PvrLoader::load(std::istream* is, const Path*, const char*)
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

void PvrLoader::uploadData(Texture& texture)
{
	MCD_ASSUME(mImpl != nullptr);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	MCD_VERIFY(texture.create(
		impl->mGpuFormat, impl->mSrcFormat,
		impl->mWidth, impl->mHeight,
		impl->header.dwNumSurfs, 1,
		impl->mImageData, impl->mImageData.size())
	);
}

ResourcePtr PvrLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "pvr") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* PvrLoaderFactory::createLoader()
{
	return new PvrLoader();
}

}	// namespace MCD
