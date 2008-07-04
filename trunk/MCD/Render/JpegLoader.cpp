#include "Pch.h"
#include "JpegLoader.h"
#include "Texture.h"
#include "TextureLoaderBaseImpl.inc"

#include "../../3Party/glew/glew.h"
#include "../../3Party/SmallJpeg/jpegdecoder.h"

#ifdef SGE_VC
#	pragma comment(lib, "SmallJpeg")
#endif	// SGE_VC

// Reading of jpeg files using the small jpeg decoder.
// It's able to load jpeg in progressive format, but not able
// to display it progressively.
// http://www.users.voicenet.com/~richgel/

namespace SGE {

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

			if(mDecoder->get_error_code() != JPGD_OKAY)
				return Aborted;

			if(mDecoder->begin() != JPGD_OKAY)
				return Aborted;

			int c = mDecoder->get_num_components();
			if(c == 1)
				mInternalFormat = mExternalFormat = GL_LUMINANCE;
			else if(c == 3) {
				mInternalFormat = GL_RGB;
				mExternalFormat = GL_RGBA;
			}
			else {
				// TODO: Log error message
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

IResourceLoader::LoadingState JpegLoader::load(std::istream* is)
{
	SGE_ASSUME(mImpl != nullptr);

	Mutex& mutex = mImpl->mMutex;
	ScopeLock lock(mutex);

	if(mLoadingState & Stopped || !is)
		return mLoadingState;

	int result;
	{	ScopeUnlock unlock(mutex);
		result = static_cast<LoaderImpl*>(mImpl)->load(*is);
	}

	switch(result) {
	case JPGD_DONE:
		mLoadingState = Loaded;
		break;
	case JPGD_OKAY:
		++(static_cast<LoaderImpl*>(mImpl)->mProcessedLines);
		mLoadingState = PartialLoaded;
		break;
	case JPGD_FAILED:
	default:
		mLoadingState = Aborted;
	}

	return mLoadingState;
}

void JpegLoader::uploadData()
{
	SGE_ASSUME(mImpl != nullptr);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);

	if(mImpl->mImageData)
		glTexImage2D(GL_TEXTURE_2D, 0, impl->mInternalFormat, impl->mWidth, impl->mHeight,
		0, impl->mExternalFormat, GL_UNSIGNED_BYTE, impl->mImageData);
}

}	// namespace SGE
