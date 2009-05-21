#include "Pch.h"
#include "TextureLoaderBase.h"
#include "Texture.h"
#include "TextureLoaderBaseImpl.inc"
#include "../Core/Math/BasicFunction.h"
#include "../Core/System/Log.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

TextureLoaderBase::LoaderBaseImpl::LoaderBaseImpl(TextureLoaderBase& loader)
	:
	mLoader(loader), mImageData(nullptr),
	mWidth(0), mHeight(0), mFormat(-1)
{
}

TextureLoaderBase::LoaderBaseImpl::~LoaderBaseImpl()
{
	MCD_ASSERT(mMutex.isLocked());
	delete[] mImageData;
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
	if(!mImpl)
		return;

	ScopeLock lock(mImpl->mMutex);

	if(!(loadingState & CanCommit))
		return;

	// Will throw exception if the resource is not of the type Texture
	Texture& texture = dynamic_cast<Texture&>(resource);

	texture.width = mImpl->mWidth;
	texture.height = mImpl->mHeight;
	texture.format = mImpl->mFormat;
	texture.type = textureType();	// Currently only support the loading of 2D texture

	if(!isPowerOf2(mImpl->mWidth) || !isPowerOf2(mImpl->mHeight))
		Log::format(Log::Warn, L"Texture:'%s' has non-power of 2 size, which may hurt performance",
			resource.fileId().getString().c_str());

	GLuint* handle = reinterpret_cast<GLuint*>(&texture.handle);

	if(*handle == 0)
		glGenTextures(1, handle);
	glBindTexture(texture.type, *handle);

	preUploadData();
	uploadData();
	postUploadData();

	if(loadingState == Loaded) {
		// The loader finish it's job, lets free up the resources
		delete mImpl;
		mImpl = nullptr;

		// The destruction of mImpl will release the mutex
		lock.cancel();

		// We are no-longer protected by the mutex, make a return statment
		// to foolproof any attempt to do anything.
		return;
	} else {
		loadingState = Loading;
	}
}

IResourceLoader::LoadingState TextureLoaderBase::getLoadingState() const
{
	// We don't have arithmetics on loadingState, so we don't need to lock on it
	return loadingState;
}

void TextureLoaderBase::preUploadData()
{
	// TODO: Should set the filtering via option strings
	const bool generateMipMap = false;

	if(generateMipMap) {
		// Reference on comparison between gluBuild2DMipmaps / GL_GENERATE_MIPMAP and glGenerateMipmapEXT
		// http://www.gamedev.net/community/forums/topic.asp?topic_id=452780
		// http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=233955
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

void TextureLoaderBase::postUploadData()
{
	// Seems that glGenerateMipmapEXT didn't work for normal texture rather than FBO
	// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=495747
//	if(glGenerateMipmapEXT)
//		glGenerateMipmapEXT(GL_TEXTURE_2D);
}

int TextureLoaderBase::textureType() const
{
    return GL_TEXTURE_2D;
}

void TextureLoaderBase::retriveData( byte_t** imageData, size_t& width, size_t& height, int& format, int& internalFmt )
{
	if(!mImpl)
		return;

	width = mImpl->mWidth;
	height = mImpl->mHeight;
	format = mImpl->mFormat;
	internalFmt = mImpl->mInternalFmt;

    size_t allocSize = width * height * Texture::bytePerPixel(format);

    *imageData = new byte_t[allocSize];
	memcpy(*imageData, mImpl->mImageData, allocSize);
}

}	// namespace MCD
