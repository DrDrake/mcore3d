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
		char* buf = mImageData;
		const size_t imageSize = mImageData.size() / 6;

		for( int i=0; i<6; ++i )
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, mGpuFormat.format, mWidth, mWidth,
				0, mSrcFormat.components, mGpuFormat.dataType, buf);

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

int CubemapLoader::textureType() const
{
	return GL_TEXTURE_CUBE_MAP;
}

void CubemapLoader::preUploadData()
{
	// TODO: Should set the filtering via option strings
	const bool generateMipMap = Texture::autoGenMipmapEnabled();

	if(generateMipMap)
	{
		// Reference on comparison between gluBuild2DMipmaps / GL_GENERATE_MIPMAP and glGenerateMipmapEXT
		// http://www.gamedev.net/community/forums/topic.asp?topic_id=452780
		// http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=233955

		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glEnable(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_FALSE);

		// NOTE: Use GL_CLAMP_TO_EDGE, otherwise there will be seams when using GL_LINEAR
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

void CubemapLoader::uploadData(Texture& texture)
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
