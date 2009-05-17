#include "Pch.h"
#include "CubemapLoader.h"
#include "TextureLoaderBaseImpl.inc"
#include "BitmapLoader.h"
#include "PngLoader.h"
#include "JpegLoader.h"
#include "TgaLoader.h"
#include "../Core/System/Log.h"
#include "../Core/System/Path.h"
#include "../Core/System/StrUtility.h"

namespace MCD {

class CubemapLoader::LoaderImpl : public TextureLoaderBase::LoaderBaseImpl
{
public:
	LoaderImpl(CubemapLoader& loader)
		:
		LoaderBaseImpl(loader)
	{
	}

	void upload()
	{
		glTexImage2D(GL_TEXTURE_2D, 0, mFormat, mWidth, mHeight,
			0, GL_BGR, GL_UNSIGNED_BYTE, &mImageData[0]);	// Note that the external format is GL_BGR but not GL_RGB
	}
};	// LoaderImpl

CubemapLoader::CubemapLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState CubemapLoader::load(std::istream* is, const Path* fileId)
{
	MCD_ASSUME(mImpl != nullptr);

	loadingState = is ? NotLoaded : Aborted;

	if(loadingState & Stopped)
		return loadingState;

	std::wstring ext = fileId->getExtension();

	// initialize
	mImpl->mImageData = nullptr;
	mImpl->mWidth = mImpl->mHeight = mImpl->mFormat = 0;

	// dispatch to other image loaders
	if(wstrCaseCmp(ext.c_str(), L"bmp")==0)
	{
		BitmapLoader loader;
		loadingState = loader.load(is, fileId);

		loader.retriveData(&mImpl->mImageData, mImpl->mWidth, mImpl->mHeight, mImpl->mFormat);
	}
	else if(wstrCaseCmp(ext.c_str(), L"jpg")==0)
	{
		JpegLoader loader;
		loadingState = loader.load(is, fileId);

		loader.retriveData(&mImpl->mImageData, mImpl->mWidth, mImpl->mHeight, mImpl->mFormat);
	}
	else if(wstrCaseCmp(ext.c_str(), L"png")==0)
	{
		PngLoader loader;
		loadingState = loader.load(is, fileId);

		loader.retriveData(&mImpl->mImageData, mImpl->mWidth, mImpl->mHeight, mImpl->mFormat);
	}
	else if(wstrCaseCmp(ext.c_str(), L"tga")==0)
	{
		TgaLoader loader;
		loadingState = loader.load(is, fileId);

		loader.retriveData(&mImpl->mImageData, mImpl->mWidth, mImpl->mHeight, mImpl->mFormat);
	}

	// There is no need to do a mutex lock during loading, since
	// no body can access the mImageData if the loading isn't finished.
	ScopeLock lock(mImpl->mMutex);

	if(mImpl->mImageData == nullptr)	
		loadingState = IResourceLoader::Aborted;

	return loadingState;
}

void CubemapLoader::uploadData()
{
	MCD_ASSUME(mImpl != nullptr);

	static_cast<LoaderImpl*>(mImpl)->upload();
}

}	// namespace MCD
