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
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, mFormat, mWidth, mWidth,
				0, GL_BGR, GL_UNSIGNED_BYTE, buf);

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

	loadingState = is ? NotLoaded : Aborted;

	if(loadingState & Stopped)
		return loadingState;

	std::wstring ext = fileId->getExtension();

	// initialize
	mImpl->mImageData = nullptr;
	mImpl->mWidth = mImpl->mHeight = mImpl->mFormat = 0;

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

    if(nullptr != loader)
    {
        loadingState = loader->load(is, fileId);
        loader->retriveData
            ( &mImpl->mImageData
            , mImpl->mWidth
            , mImpl->mHeight
            , mImpl->mFormat);

		// mLoaderDelegate will be destroyed(accessed) in destructor,
		// so we must also lock all operations that involve mLoaderDelegate
		ScopeLock lock(mImpl->mMutex);
        static_cast<LoaderImpl*>(mImpl)->mLoaderDelegate.reset(loader);

        if(mImpl->mHeight != 6 * mImpl->mWidth)
        {
            loadingState = IResourceLoader::Aborted;
        }

		mImpl->mHeight = mImpl->mWidth;
    }
    else
    {
		ScopeLock lock(mImpl->mMutex);
        loadingState = IResourceLoader::Aborted;
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

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
    {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_FALSE);
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

void CubemapLoader::postUploadData()
{
	// Seems that glGenerateMipmapEXT didn't work for normal texture rather than FBO
	// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=495747
//	if(glGenerateMipmapEXT)
//		glGenerateMipmapEXT(GL_TEXTURE_2D);
}

void CubemapLoader::uploadData()
{
	MCD_ASSUME(mImpl != nullptr);
	static_cast<LoaderImpl*>(mImpl)->upload();
}

}	// namespace MCD
