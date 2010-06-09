#include "Pch.h"
#include "CubemapLoader.h"
#include "TextureLoaderBaseImpl.inc"
#include "BitmapLoader.h"
#include "PngLoader.h"
#include "JpegLoader.h"
#include "TgaLoader.h"
#include "../Render/Texture.h"
#include "../Core/System/Log.h"
#include "../Core/System/Path.h"
#include "../Core/System/StrUtility.h"

#include <memory>

namespace MCD {

class CubemapLoader::LoaderImpl : public TextureLoaderBase::LoaderBaseImpl
{
public:
	LoaderImpl(CubemapLoader& loader)
		:
		LoaderBaseImpl(loader)
	{
	}

	std::auto_ptr<TextureLoaderBase> mLoaderDelegate;
};	// LoaderImpl

CubemapLoader::CubemapLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState CubemapLoader::load(std::istream* is, const Path* fileId, const char*)
{
	MCD_ASSUME(mImpl != nullptr);

	ScopeLock lock(mImpl->mMutex);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);

	if(nullptr == impl->mLoaderDelegate.get())
	{
		if(!fileId)
			return (loadingState = IResourceLoader::Aborted);

		std::string ext = fileId->getExtension();

		TextureLoaderBase* loader = nullptr;

		// Dispatch to other image loaders
		if(strCaseCmp(ext.c_str(), "bmp") == 0)
			loader = new BitmapLoader;
		else if(strCaseCmp(ext.c_str(), "jpg") == 0)
			loader = new JpegLoader;
		else if(strCaseCmp(ext.c_str(), "png") == 0)
			loader = new PngLoader;
		else if(strCaseCmp(ext.c_str(), "tga") == 0)
			loader = new TgaLoader;

		impl->mLoaderDelegate.reset(loader);
	}

	volatile LoadingState tmpState;
	{	ScopeUnlock unlock(mImpl->mMutex);
		tmpState = impl->mLoaderDelegate->load(is, fileId);
	}
	loadingState = tmpState;

	if((CanCommit & loadingState))
	{
		const char* data;
		size_t size;
		impl->mLoaderDelegate->retriveData(
			data, size
			, mImpl->mWidth, mImpl->mHeight
			, mImpl->mSrcFormat, mImpl->mGpuFormat
		);

		if(impl->mImageData.size() != size)
			impl->mImageData = TextureLoaderBase::LoaderBaseImpl::ImageData(size);

		::memcpy(mImpl->mImageData, data, size);

		if(mImpl->mHeight != 6 * mImpl->mWidth)
			loadingState = IResourceLoader::Aborted;
		else
			// Shut up the warning of non-power of 2 texture in the delegate loader.
			mImpl->mHeight = mImpl->mWidth;
	}

	return loadingState;
}

void CubemapLoader::uploadData(Texture& texture)
{
	MCD_ASSUME(mImpl != nullptr);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	MCD_VERIFY(texture.create(
		impl->mGpuFormat, impl->mSrcFormat,
		impl->mWidth, impl->mHeight,
		6, 1,
		impl->mImageData, impl->mImageData.size())
	);
}

void CubemapLoader::postUploadData()
{
	TextureLoaderBase::postUploadData();

	// NOTE: mImpl may become null, after TextureLoaderBase::postUploadData() is called.
	if(!mImpl)
		return;

	// Update the delegated loader's loading state as this one,
	// after postUploadData() is invoked
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	impl->mLoaderDelegate->loadingState = loadingState;
}

}	// namespace MCD
