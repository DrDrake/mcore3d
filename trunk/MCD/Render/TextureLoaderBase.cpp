#include "Pch.h"
#include "TextureLoaderBase.h"
#include "Texture.h"
#include "TextureLoaderBaseImpl.inc"
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
	delete[] mImageData;
}

TextureLoaderBase::TextureLoaderBase()
	: mImpl(nullptr), mLoadingState(NotLoaded)
{
}

TextureLoaderBase::~TextureLoaderBase()
{
	delete mImpl;
}

void TextureLoaderBase::setImpl(LoaderBaseImpl* impl)
{
	MCD_ASSERT(mImpl == nullptr);
	mImpl = impl;
}

template<>
class Texture::PrivateAccessor<TextureLoaderBase>
{
public:
	static uint& handle(Texture& texture) {
		return texture.mHandle;
	}
	static int& format(Texture& texture) {
		return texture.mFormat;
	}
	static size_t& width(Texture& texture) {
		return texture.mWidth;
	}
	static size_t& height(Texture& texture) {
		return texture.mHeight;
	}
};	// PrivateAccessor

void TextureLoaderBase::commit(Resource& resource)
{
	typedef Texture::PrivateAccessor<TextureLoaderBase> Accessor;

	if(!mImpl)
		return;

	ScopeLock lock(mImpl->mMutex);

	if(!(mLoadingState & CanCommit))
		return;

	// Will throw exception if the resource is not of the type Texture
	Texture& texture = dynamic_cast<Texture&>(resource);

	Accessor::width(texture) = mImpl->mWidth;
	Accessor::height(texture) = mImpl->mHeight;
	Accessor::format(texture) = mImpl->mFormat;

	GLuint* handle = reinterpret_cast<GLuint*>(&Accessor::handle(texture));

	if(*handle == 0)
		glGenTextures(1, handle);
	glBindTexture(GL_TEXTURE_2D, *handle);

	preUploadData();
	uploadData();
	postUploadData();

	if(mLoadingState == Loaded) {
		// The loader finish it's job, lets free up the resources
		lock.cancel();
		mImpl->mMutex.unlock();
		delete mImpl;
		mImpl = nullptr;
	} else {
		mLoadingState = Loading;
	}
}

IResourceLoader::LoadingState TextureLoaderBase::getLoadingState() const
{
	if(mImpl) {
		ScopeLock lock(mImpl->mMutex);
		return mLoadingState;
	}
	return mLoadingState;
}

void TextureLoaderBase::preUploadData()
{
	// TODO: Should set the filtering via option strings
	const bool generateMipMap = true;

	if(generateMipMap) {
		// Reference on comparision between gluBuild2DMipmaps / GL_GENERATE_MIPMAP and glGenerateMipmapEXT
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

}	// namespace MCD
