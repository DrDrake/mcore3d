#include "Pch.h"
#include "PngLoader.h"
#include "Texture.h"
#include "TextureLoaderBaseImpl.inc"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"

#include "../../3Party/png/png.h"
#include "../../3Party/glew/glew.h"
#include <stdexcept>

#ifdef MCD_VC
#	pragma comment(lib, "png")
#	pragma comment(lib, "zlib")
#endif	// MCD_VC

// Reading of png files using the libpng
// The code is base on the example provided in libpng
// More information can be found in
// http://www.libpng.org/
// http://www.libpng.org/pub/png/libpng-1.2.5-manual.html

namespace MCD {

class PngLoader::LoaderImpl : public TextureLoaderBase::LoaderBaseImpl
{
public:
	LoaderImpl(PngLoader& loader)
		:
		LoaderBaseImpl(loader),
		png_ptr(nullptr), info_ptr(nullptr),
		mRowBytes(0),
		mCurrentPass(0), mHasError(false)
	{
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if(!png_ptr)
			throw std::runtime_error("png reader error");

		info_ptr = png_create_info_struct(png_ptr);
		if(!info_ptr) {
			png_destroy_read_struct(&png_ptr, nullptr, nullptr);
			throw std::runtime_error("png reader error");
		}

		// Setup the callback that will be called during the data processing.
		png_set_progressive_read_fn(png_ptr, (void*)this, info_callback, row_callback, end_callback);
	}

	sal_override ~LoaderImpl()
	{
		// libpng will handle for null input pointers
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)nullptr);
	}

	void onPngInfoReady()
	{
		// Query the info structure.
		int bit_depth, color_type, interlace_method;
		png_get_IHDR(png_ptr, info_ptr,
			(png_uint_32*)(&mWidth), (png_uint_32*)(&mHeight),
			&bit_depth, &color_type, &interlace_method, nullptr, nullptr);

		mRowBytes = info_ptr->rowbytes;

		switch(color_type) {
		case PNG_COLOR_TYPE_RGB:
			{
				mFormat = GL_RGB;
				mGpuFormat = GpuDataFormat::get("uintR8G8B8");
			}
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			{
				mFormat = GL_RGBA;
				mGpuFormat = GpuDataFormat::get("uintR8G8B8A8");
			}
			break;
		case PNG_COLOR_TYPE_GRAY:
			{
				MCD_ASSERT(false && "Not implemented");
				mFormat = GL_LUMINANCE;
//				mInternalFmt = GL_LUMINANCE;
			}
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			{
				MCD_ASSERT(false && "Not implemented");
				mFormat = GL_LUMINANCE_ALPHA;
//				mInternalFmt = GL_LUMINANCE_ALPHA;
			}
			break;
		case PNG_COLOR_TYPE_PALETTE:	// Color palette is not supported
			Log::write(Log::Error, "PngLoader: image using color palette is not supported, operation aborted");
		default:
			mHasError = true;
			break;
		}

		// We'll let libpng expand interlaced images.
		if(interlace_method == PNG_INTERLACE_ADAM7) {
			int number_of_passes = png_set_interlace_handling(png_ptr);
			(void)number_of_passes;
		}

		png_read_update_info(png_ptr, info_ptr);

		mImageData = new png_byte[mRowBytes * mHeight];
	}

	static void info_callback(png_structp png_ptr, png_infop)
	{
		LoaderImpl* impl = reinterpret_cast<LoaderImpl*>(png_get_progressive_ptr(png_ptr));
		MCD_ASSUME(impl != nullptr);
		impl->onPngInfoReady();
	}

	// This function is called for every pass when each row of image data is complete.
	static void row_callback(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass)
	{
		LoaderImpl* impl = reinterpret_cast<LoaderImpl*>(png_get_progressive_ptr(png_ptr));
		MCD_ASSUME(impl != nullptr);
		MCD_ASSUME(impl->mImageData != nullptr);
		MCD_ASSERT(impl->mMutex.isLocked());

		// Have libpng either combine the new row data with the existing row data
		// from previous passes (if interlaced) or else just copy the new row
		// into the main program's image buffer
		png_progressive_combine_row(png_ptr, impl->mImageData + row_num * impl->mRowBytes, new_row);

		// Only change the loading state after a pass is finished
		if(pass > impl->mCurrentPass) {
			impl->mCurrentPass = pass;
			impl->mLoader.loadingState = PartialLoaded;
		}
	}

	static void end_callback(png_structp png_ptr, png_infop)
	{
		LoaderImpl* impl = reinterpret_cast<LoaderImpl*>(png_get_progressive_ptr(png_ptr));
		MCD_ASSUME(impl != nullptr);

		impl->mLoader.loadingState = Loaded;
	}

	png_structp png_ptr;
	png_infop info_ptr;

	png_uint_32 mRowBytes;	// Number of byte per row of image data
	int mCurrentPass;		// For keep tracking when a new pass is loaded
	bool mHasError;
};	// LoaderImpl

PngLoader::PngLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState PngLoader::load(std::istream* is, const Path*, const char*)
{
	MemoryProfiler::Scope scope("PngLoader::load");
	MCD_ASSUME(mImpl != nullptr);

	Mutex& mutex = mImpl->mMutex;
	ScopeLock lock(mutex);

	if(!is)
		loadingState = Aborted;
	else if(loadingState == Aborted)
		loadingState = NotLoaded;

	if(loadingState & Stopped)
		return loadingState;

#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning(disable: 4611)
#endif
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	// Jump to here if any error occur in png_process_data
	if(impl->mHasError || setjmp(png_jmpbuf(impl->png_ptr)))
#ifdef MCD_VC
#	pragma warning(pop)
#endif
	{
		return (loadingState = Aborted);
	}

	// Process the data (used for progressive loading).
	char buff[1024*8];
	{	ScopeUnlock unlock(mutex);
		is->read(buff, sizeof(buff));
	}
	png_size_t readCount = static_cast<png_size_t>(is->gcount());

	png_process_data(impl->png_ptr, impl->info_ptr, (png_bytep)buff, readCount);

	if(readCount == 0)
		loadingState = Aborted;

	return loadingState;
}

void PngLoader::uploadData()
{
	MCD_ASSUME(mImpl != nullptr);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	MCD_ASSERT(mImpl->mMutex.isLocked());

	if(mImpl->mImageData)
		glTexImage2D(GL_TEXTURE_2D, 0, impl->mGpuFormat.format, impl->mWidth, impl->mHeight,
		0, impl->mFormat, GL_UNSIGNED_BYTE, impl->mImageData);
}

}	// namespace MCD
