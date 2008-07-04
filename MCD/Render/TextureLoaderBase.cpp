#include "Pch.h"
#include "TextureLoaderBase.h"
#include "Texture.h"
#include "TextureLoaderBaseImpl.inc"
#include "../../3Party/glew/glew.h"

namespace SGE {

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
	SGE_ASSERT(mImpl == nullptr);
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

	Texture& texture = dynamic_cast<Texture&>(resource);

	Accessor::width(texture) = mImpl->mWidth;
	Accessor::height(texture) = mImpl->mHeight;
	Accessor::format(texture) = mImpl->mFormat;

	GLuint* handle = reinterpret_cast<GLuint*>(&Accessor::handle(texture));

	if(*handle == 0)
		glGenTextures(1, handle);
	glBindTexture(GL_TEXTURE_2D, *handle);

	uploadData();

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

}	// namespace SGE
