#include "Pch.h"
#include "JpegLoader.h"
#include "Texture.h"
#include "TextureLoaderBaseImpl.inc"
#include "../Core/System/Log.h"

#include "../../3Party/glew/glew.h"
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
		size_t readCount = mIStream.gcount();

		if(Peof_flag)
			*Peof_flag = (readCount == 0);

		return readCount;
	}

	std::istream& mIStream;
};	// Stream

class JpegLoader::LoaderImpl : public TextureLoaderBase::LoaderBaseImpl
{
public:
	LoaderImpl(JpegLoader& loader)
		:
		LoaderBaseImpl(loader),
		mDecoder(nullptr), mStream(nullptr),
		mRowBytes(0),
		mInternalFormat(-1), mExternalFormat(-1),
		mProcessedLines(0)
	{
	}

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
				Log::format(Log::Error, L"JpegLoader: load error, operation aborted");
				return Aborted;
			}

			if(mDecoder->begin() != JPGD_OKAY) {
				Log::format(Log::Error, L"JpegLoader: load error, operation aborted");
				return Aborted;
			}

			int c = mDecoder->get_num_components();
			if(c == 1)
				mInternalFormat = mExternalFormat = GL_LUMINANCE;
			else if(c == 3) {
				mInternalFormat = GL_RGB;
				mExternalFormat = GL_RGBA;
			}
			else {
				Log::format(Log::Error, L"JpegLoader: image with number of color component equals to %i is not supported, operation aborted", c);
				return JPGD_FAILED;
			}

			mWidth = mDecoder->get_width();
			mHeight = mDecoder->get_height();
			mFormat = mInternalFormat;
			mRowBytes = mDecoder->get_bytes_per_scan_line();

			mImageData = new byte_t[mRowBytes * mHeight];
		}

		void* Pscan_line_ofs = nullptr;
		uint scan_line_len = 0;

		int result = mDecoder->decode(&Pscan_line_ofs, &scan_line_len);
		(void)result;

		// Copy the scan line to our image data buffer
		byte_t* p = mImageData + mProcessedLines * mRowBytes;
		{	ScopeLock lock(mMutex);
			memcpy(p, Pscan_line_ofs, scan_line_len);
		}

		return result;
	}

	Pjpeg_decoder mDecoder;
	Stream* mStream;

	size_t mRowBytes;		// Number of byte per row of image data
	int mInternalFormat;
	int mExternalFormat;
	size_t mProcessedLines;	// The current number of processed scan line
};	// LoaderImpl

JpegLoader::JpegLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState JpegLoader::load(std::istream* is, const Path*)
{
	MCD_ASSUME(mImpl != nullptr);

	Mutex& mutex = mImpl->mMutex;
	ScopeLock lock(mutex);

	loadingState = is ? NotLoaded : Aborted;

	if(loadingState & Stopped)
		return loadingState;

	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);

	int result;
	{	ScopeUnlock unlock(mutex);
		result = impl->load(*is);
	}

	switch(result) {
	case JPGD_DONE:
		loadingState = Loaded;
		break;
	case JPGD_OKAY:
		++(impl->mProcessedLines);
		// Only report partial load every 1/4 of the scan lines are loaded
		if(impl->mProcessedLines % (impl->mHeight / 4) == 0)
			loadingState = PartialLoaded;
		break;
	case JPGD_FAILED:
	default:
		loadingState = Aborted;
	}

	return loadingState;
}

void JpegLoader::uploadData()
{
	MCD_ASSUME(mImpl != nullptr);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	MCD_ASSERT(mImpl->mMutex.isLocked());

	// NOTE: To compress texture on the fly, just pass GL_COMPRESSED_XXX_ARB as the internal format
	// Reference: www.oldunreal.com/editing/s3tc/ARB_texture_compression.pdf
	if(mImpl->mImageData)
		glTexImage2D(GL_TEXTURE_2D, 0, impl->mInternalFormat, impl->mWidth, impl->mHeight,
		0, impl->mExternalFormat, GL_UNSIGNED_BYTE, impl->mImageData);
}

}	// namespace MCD
