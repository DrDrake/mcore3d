#include "Pch.h"
#include "TextureLoaderBase.h"
#include "Texture.h"
#include "TextureLoaderBaseImpl.inc"
#include "../Core/Math/BasicFunction.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/ResourceManager.h"
#include "../../3Party/glew/glew.h"
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

	texture.width = mImpl->mWidth;
	texture.height = mImpl->mHeight;
	texture.format = mImpl->mGpuFormat;
	texture.type = textureType();	// Currently only support the loading of 2D texture

	if(!isPowerOf2(mImpl->mWidth) || !isPowerOf2(mImpl->mHeight))
		Log::format(Log::Warn, "Texture:'%s' has non-power of 2 size, which may hurt performance",
			resource.fileId().getString().c_str());

	GLuint* handle = reinterpret_cast<GLuint*>(&texture.handle);

	if(*handle == 0)
		glGenTextures(1, handle);
	glBindTexture(texture.type, *handle);

	preUploadData();
	uploadData(texture);
	postUploadData();

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
	// TODO: Should set the filtering via option strings
	const bool generateMipMap = Texture::autoGenMipmapEnabled();

	if(generateMipMap) {
		// Reference on comparison between gluBuild2DMipmaps / GL_GENERATE_MIPMAP and glGenerateMipmapEXT
		// http://www.gamedev.net/community/forums/topic.asp?topic_id=452780
		// http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=233955
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
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

int TextureLoaderBase::textureType() const
{
	return GL_TEXTURE_2D;
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
