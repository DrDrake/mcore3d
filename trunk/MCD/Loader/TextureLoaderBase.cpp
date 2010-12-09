#include "Pch.h"
#include "TextureLoaderBase.h"
#include "TextureLoaderBaseImpl.inc"
#include "../Render/Texture.h"
#include "../Core/Math/BasicFunction.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/ResourceManager.h"
#include <memory.h>	// For memcpy

namespace MCD {

TextureLoaderBase::LoaderBaseImpl::LoaderBaseImpl(TextureLoaderBase& loader)
	: mLoader(loader)
	, mWidth(0), mHeight(0)
	, mMipLevels(1)
{
}

TextureLoaderBase::LoaderBaseImpl::~LoaderBaseImpl()
{
	MCD_ASSERT(mMutex.isLocked());
	mImageData.clear();
	mMutex.unlock();
}

void TextureLoaderBase::LoaderBaseImpl::genMipmap()
{
	if(mImageData.size() == 0) return;

	size_t levels = 0;
	size_t s = mWidth < mHeight ? mWidth : mHeight;
	for(;s >= 1; s /= 2) ++levels;
	mMipLevels = levels;

	MCD_ASSUME(levels > 0);

	const size_t oldSize = mImageData.size();
	const size_t newSize = size_t(oldSize * 1.34);	// There is a little extra room
	const size_t bytePerPixel = mSrcFormat.sizeInByte();
	mImageData.mImageData = (char*)realloc(mImageData.mImageData, newSize);

	size_t w1 = mWidth, h1 = mHeight;
	byte_t* p1 = (byte_t*)mImageData.mImageData, *p2 = p1 + w1 * h1 * bytePerPixel;

	for(size_t l=1; l<levels; ++l)
	{
		const size_t w2 = w1/2;
		const size_t h2 = h1/2;
		const size_t stride1 = w1 * bytePerPixel;

		for(size_t i=0; i<h2; ++i) for(size_t j=0; j<w2; ++j) {
			byte_t* v1 = p1 + i*2*stride1 + j*2*bytePerPixel + bytePerPixel*0;
			byte_t* v2 = p1 + i*2*stride1 + j*2*bytePerPixel + bytePerPixel*1;
			byte_t* v3 = p1 + i*2*stride1 + stride1 + j*2*bytePerPixel + bytePerPixel*0;
			byte_t* v4 = p1 + i*2*stride1 + stride1 + j*2*bytePerPixel + bytePerPixel*1;

			for(size_t b=0; b<bytePerPixel; ++b) {
				p2[b] = char((int(v1[b]) + int(v2[b]) + int(v3[b]) + int(v4[b])) / 4);
			}

			p2 += bytePerPixel;
		}

		p1 += w1 * h1 * bytePerPixel;
		w1 = w2;
		h1 = h2;
	}

	mImageData.mSize = p2 - (byte_t*)mImageData.mImageData;
}

TextureLoaderBase::TextureLoaderBase()
	: mImpl(nullptr)
{
}

TextureLoaderBase::~TextureLoaderBase()
{
	if(mImpl)
		mImpl->mMutex.lock();
	delete mImpl;
}

void TextureLoaderBase::setImpl(LoaderBaseImpl* impl)
{
	MCD_ASSERT(mImpl == nullptr);
	mImpl = impl;
}

void TextureLoaderBase::commit(Resource& resource)
{
	MemoryProfiler::Scope scope("TextureLoaderBase::commit");

	MCD_ASSUME(mImpl);
	ScopeLock lock(mImpl->mMutex);

	// Will throw exception if the resource is not of the type Texture
	Texture& texture = dynamic_cast<Texture&>(resource);

	if(!isPowerOf2(mImpl->mWidth) || !isPowerOf2(mImpl->mHeight))
		Log::format(Log::Warn, "Texture:'%s' has non-power of 2 size, which may hurt performance",
			resource.fileId().c_str());

	preUploadData();
	uploadData(texture);
	postUploadData();

	// Release the memory once the image is fully loaded and uploaded
	if(loadingState() & Stopped)
		mImpl->mImageData.clear();
}

void TextureLoaderBase::preUploadData()
{
}

void TextureLoaderBase::postUploadData()
{
	// Seems that glGenerateMipmapEXT didn't work for normal texture rather than FBO
	// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=495747
//	if(glGenerateMipmapEXT)
//		glGenerateMipmapEXT(GL_TEXTURE_2D);
}

void TextureLoaderBase::retriveData(const char*& imageData, size_t& imageDataSize, size_t& width, size_t& height, GpuDataFormat& srcFormat, GpuDataFormat& gpuFormat)
{
	if(!mImpl) return;

	imageData = mImpl->mImageData;
	imageDataSize = mImpl->mImageData.size();
	width = mImpl->mWidth;
	height = mImpl->mHeight;
	srcFormat = mImpl->mSrcFormat;
	gpuFormat = mImpl->mGpuFormat;
}

}	// namespace MCD
