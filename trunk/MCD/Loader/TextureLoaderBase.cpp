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
{
}

TextureLoaderBase::LoaderBaseImpl::~LoaderBaseImpl()
{
	MCD_ASSERT(mMutex.isLocked());
	mImageData.clear();
	mMutex.unlock();
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
			resource.fileId().getString().c_str());

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
