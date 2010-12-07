#include "Pch.h"
#include "JpegLoader.h"
#include "TextureLoaderBaseImpl.inc"
#include "../Render/Texture.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/StrUtility.h"
#include "../../3Party/SmallJpeg/jpegdecoder.h"

#ifdef MCD_VC
#	pragma comment(lib, "SmallJpeg")
#endif	// MCD_VC

// Reading of jpeg files using the small jpeg decoder.
// It's able to load jpeg in progressive format, but not able
// to display it progressively.
// http://www.users.voicenet.com/~richgel/

namespace MCD {

class Stream : public jpeg_decoder_stream
{
public:
	explicit Stream(std::istream& is) : mIStream(is) {}

	sal_override int read(uchar* Pbuf, int max_bytes_to_read, bool* Peof_flag)
	{
		mIStream.read((char*)Pbuf, max_bytes_to_read);
		std::streamsize readCount = mIStream.gcount();

		if(Peof_flag)
			*Peof_flag = (readCount == 0);

		return static_cast<int>(readCount);
	}

	std::istream& mIStream;
};	// Stream

class JpegLoader::LoaderImpl : public TextureLoaderBase::LoaderBaseImpl
{
public:
	LoaderImpl(JpegLoader& loader)
		: LoaderBaseImpl(loader)
		, mDecoder(nullptr), mStream(nullptr)
		, mRowBytes(0)
		, mProcessedLines(0)
	{}

	sal_override ~LoaderImpl()
	{
		delete mDecoder;
		delete mStream;
	}

	int load(std::istream& is)
	{
		if(!mDecoder) {
			ScopeLock lock(mMutex);
			mDecoder = new jpeg_decoder(mStream = new Stream(is), true);

			if(mDecoder->get_error_code() != JPGD_OKAY) {
				Log::format(Log::Error, "JpegLoader: load error, operation aborted");
				return Aborted;
			}

			if(mDecoder->begin() != JPGD_OKAY) {
				Log::format(Log::Error, "JpegLoader: load error, operation aborted");
				return Aborted;
			}

			int c = mDecoder->get_num_components();
			if(c == 1) {
				MCD_ASSERT(false && "Not implemented");
//				mInternalFmt = mFormat = GL_LUMINANCE;
			}
			else if(c == 3) {
				mSrcFormat = GpuDataFormat::get("uintRGBA8");	// Note that the source format is 4 byte even c == 3
				mGpuFormat = GpuDataFormat::get("uintRGB8");
			}
			else {
				Log::format(Log::Error, "JpegLoader: image with number of color component equals to %i is not supported, operation aborted", c);
				return JPGD_FAILED;
			}

			mWidth = mDecoder->get_width();
			mHeight = mDecoder->get_height();
			mRowBytes = mDecoder->get_bytes_per_scan_line();

			MCD_ASSERT(!mImageData);
			mImageData = ImageData(mRowBytes * mHeight);
		}

		void* Pscan_line_ofs = nullptr;
		uint scan_line_len = 0;

		int result = mDecoder->decode(&Pscan_line_ofs, &scan_line_len);
		(void)result;

		// Copy the scan line to our image data buffer
		char* p = mImageData + mProcessedLines * mRowBytes;
		{	ScopeLock lock(mMutex);
			memcpy(p, Pscan_line_ofs, scan_line_len);
		}

		return result;
	}

	Pjpeg_decoder mDecoder;
	Stream* mStream;

	size_t mRowBytes;		// Number of byte per row of image data
	size_t mProcessedLines;	// The current number of processed scan line
};	// LoaderImpl

JpegLoader::JpegLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState JpegLoader::load(std::istream* is, const Path*, const char*)
{
	MemoryProfiler::Scope scope("JpegLoader::load");
	MCD_ASSUME(mImpl != nullptr);

	if(!is)
		return Aborted;

	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	const int result = impl->load(*is);

	switch(result) {
	case JPGD_DONE:
		return Loaded;
		break;
	case JPGD_OKAY:
		++(impl->mProcessedLines);
		// Only report partial load every 1/4 of the scan lines are loaded
		if(true || impl->mProcessedLines % (impl->mHeight / 4) == 0) {
			continueLoad();
			return PartialLoaded;
		}
		break;
	case JPGD_FAILED:
	default:
		return Aborted;
	}

	return loadingState();
}

void JpegLoader::uploadData(Texture& texture)
{
	MCD_ASSUME(mImpl != nullptr);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);

	impl->genMipmap();

	MCD_ASSERT(mImpl->mMutex.isLocked());
	MCD_VERIFY(texture.create(
		impl->mGpuFormat, impl->mSrcFormat,
		impl->mWidth, impl->mHeight,
		1,
		impl->mMipLevels,
		impl->mImageData, impl->mImageData.size())
	);
}

ResourcePtr JpegLoaderFactory::createResource(const Path& fileId, const char* args)
{
	std::string extStr = fileId.getExtension();
	const char* ext = extStr.c_str();
	if(strCaseCmp(ext, "jpg") == 0 || strCaseCmp(ext, "jpeg") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoaderPtr JpegLoaderFactory::createLoader()
{
	return new JpegLoader;
}

}	// namespace MCD
