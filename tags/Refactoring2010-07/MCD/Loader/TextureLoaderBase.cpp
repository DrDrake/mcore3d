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
	: loadingState(NotLoaded), mImpl(nullptr)
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

	if(!mImpl)
		return;

	ScopeLock lock(mImpl->mMutex);

	if(!(loadingState & CanCommit))
		return;

	// Will throw exception if the resource is not of the type Texture
	Texture& texture = dynamic_cast<Texture&>(resource);

	if(!isPowerOf2(mImpl->mWidth) || !isPowerOf2(mImpl->mHeight))
		Log::format(Log::Warn, "Texture:'%s' has non-power of 2 size, which may hurt performance",
			resource.fileId().getString().c_str());

	preUploadData();
	uploadData(texture);
	postUploadData();

	++resource.commitCount;

	// The invocation of postUploadData() may deleted mImpl
	if(!mImpl) {
		// The destruction of mImpl will release the mutex
		lock.cancel();

		// We are no-longer protected by the mutex, make a return statment
		// to foolproof any attempt to do anything.
		return;
	}
}

IResourceLoader::LoadingState TextureLoaderBase::getLoadingState() const
{
	// We don't have arithmetics on loadingState, so we don't need to lock on it
	return loadingState;
}

void TextureLoaderBase::onPartialLoaded(IPartialLoadContext& context, uint priority, const char* args)
{
	context.continueLoad(priority+1, args);
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

	if(loadingState == Loaded) {
		// The loader finish it's job, lets free up the resources
		// NOTE: We must use set mImpl = null before deleting mImpl,
		// otherwise the act of delete mImpl will unlock the mutex and
		// so another thread may have daling mImpl pointer.
		LoaderBaseImpl* tmp = mImpl;
		mImpl = nullptr;
		delete tmp;

		// We are no-longer protected by the mutex, make a return statment
		// to foolproof any attempt to do anything.
		return;
	} else {
		loadingState = Loading;
	}
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
