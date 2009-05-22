#include "Pch.h"
#include "CubemapLoader.h"
#include "TextureLoaderBaseImpl.inc"
#include "BitmapLoader.h"
#include "PngLoader.h"
#include "JpegLoader.h"
#include "TgaLoader.h"
#include "Texture.h"
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

	void upload()
	{
		//glTexImage2D(GL_TEXTURE_2D, 0, mFormat, mWidth, mHeight,
		//	0, GL_BGR, GL_UNSIGNED_BYTE, &mImageData[0]);	// Note that the external format is GL_BGR but not GL_RGB

		size_t imageSize = mWidth * mWidth * Texture::bytePerPixel(mFormat);

		byte_t * buf = mImageData;

		for( int i=0; i<6; ++i )
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, mInternalFmt, mWidth, mWidth,
				0, mFormat, GL_UNSIGNED_BYTE, buf);

			buf += imageSize;
		}
	}

	std::auto_ptr<TextureLoaderBase> mLoaderDelegate;
};	// LoaderImpl

CubemapLoader::CubemapLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState CubemapLoader::load(std::istream* is, const Path* fileId)
{
	MCD_ASSUME(mImpl != nullptr);

	if(loadingState & Stopped)
		return loadingState;

	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);

	if(nullptr == impl->mLoaderDelegate.get())
	{
		std::wstring ext = fileId->getExtension();

		TextureLoaderBase* loader = nullptr;

		// dispatch to other image loaders
		if(wstrCaseCmp(ext.c_str(), L"bmp")==0)
		{
			loader = new BitmapLoader;
		}
		else if(wstrCaseCmp(ext.c_str(), L"jpg")==0)
		{
			loader = new JpegLoader;
		}
		else if(wstrCaseCmp(ext.c_str(), L"png")==0)
		{
			loader = new PngLoader;
		}
		else if(wstrCaseCmp(ext.c_str(), L"tga")==0)
		{
			loader = new TgaLoader;
		}

		ScopeLock lock(mImpl->mMutex);
		impl->mLoaderDelegate.reset(loader);
	}

	volatile LoadingState tmpState = impl->mLoaderDelegate->load(is, fileId);

	ScopeLock lock(mImpl->mMutex);
	loadingState = tmpState;
	if((CanCommit & loadingState))
	{
		impl->mLoaderDelegate->retriveData
			( &mImpl->mImageData
			, mImpl->mWidth
			, mImpl->mHeight
			, mImpl->mFormat
			, mImpl->mInternalFmt
			);

		if(mImpl->mHeight != 6 * mImpl->mWidth)
			loadingState = IResourceLoader::Aborted;
		else
			mImpl->mHeight = mImpl->mWidth;
	}

	return loadingState;
}

int CubemapLoader::textureType() const
{
	return GL_TEXTURE_CUBE_MAP;
}

void CubemapLoader::preUploadData()
{
	// TODO: Should set the filtering via option strings
	const bool generateMipMap = false;

	if(generateMipMap)
	{
		// Reference on comparison between gluBuild2DMipmaps / GL_GENERATE_MIPMAP and glGenerateMipmapEXT
		// http://www.gamedev.net/community/forums/topic.asp?topic_id=452780
		// http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=233955

		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glEnable(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_FALSE);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

void CubemapLoader::uploadData()
{
	MCD_ASSUME(mImpl != nullptr);
	static_cast<LoaderImpl*>(mImpl)->upload();
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
