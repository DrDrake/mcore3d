#include "Pch.h"
#include "DdsLoader.h"
#include "Texture.h"
#include "TextureLoaderBaseImpl.inc"
#include "../Core/System/Log.h"
#include "../../3Party/glew/glew.h"

// http://www.mindcontrol.org/~hplus/graphics/dds-info/

namespace MCD {

// Little-endian, of course
#define DDS_MAGIC 0x20534444

// DDS_header.dwFlags
#define DDSD_CAPS					0x00000001
#define DDSD_HEIGHT					0x00000002
#define DDSD_WIDTH					0x00000004
#define DDSD_PITCH					0x00000008
#define DDSD_PIXELFORMAT			0x00001000
#define DDSD_MIPMAPCOUNT			0x00020000
#define DDSD_LINEARSIZE				0x00080000
#define DDSD_DEPTH					0x00800000

// DDS_header.sPixelFormat.dwFlags
#define DDPF_ALPHAPIXELS			0x00000001
#define DDPF_FOURCC					0x00000004
#define DDPF_INDEXED				0x00000020
#define DDPF_RGB					0x00000040

// DDS_header.sCaps.dwCaps1
#define DDSCAPS_COMPLEX				0x00000008
#define DDSCAPS_TEXTURE				0x00001000
#define DDSCAPS_MIPMAP				0x00400000

// DDS_header.sCaps.dwCaps2
#define DDSCAPS2_CUBEMAP			0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX	0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX	0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY	0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY	0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ	0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ	0x00008000
#define DDSCAPS2_VOLUME				0x00200000

#define D3DFMT_DXT1 "DXT1"	//  DXT1 compression texture format
#define D3DFMT_DXT2 "DXT2"	//  DXT2 compression texture format
#define D3DFMT_DXT3 "DXT3"	//  DXT3 compression texture format
#define D3DFMT_DXT4 "DXT4"	//  DXT4 compression texture format
#define D3DFMT_DXT5 "DXT5"	//  DXT5 compression texture format

#define PF_IS_DXT1(pf) \
	((pf.dwFlags & DDPF_FOURCC) && \
	(pf.dwFourCC == *((uint*)D3DFMT_DXT1)))

#define PF_IS_DXT3(pf) \
	((pf.dwFlags & DDPF_FOURCC) && \
	(pf.dwFourCC == *(uint*)D3DFMT_DXT3))

#define PF_IS_DXT5(pf) \
	((pf.dwFlags & DDPF_FOURCC) && \
	(pf.dwFourCC == *(uint*)D3DFMT_DXT5))

#define PF_IS_BGRA8(pf) \
	((pf.dwFlags & DDPF_RGB) && \
	(pf.dwFlags & DDPF_ALPHAPIXELS) && \
	(pf.dwRGBBitCount == 32) && \
	(pf.dwRBitMask == 0xff0000) && \
	(pf.dwGBitMask == 0xff00) && \
	(pf.dwBBitMask == 0xff) && \
	(pf.dwAlphaBitMask == 0xff000000U))

#define PF_IS_BGR8(pf) \
	((pf.dwFlags & DDPF_ALPHAPIXELS) && \
	!(pf.dwFlags & DDPF_ALPHAPIXELS) && \
	(pf.dwRGBBitCount == 24) && \
	(pf.dwRBitMask == 0xff0000) && \
	(pf.dwGBitMask == 0xff00) && \
	(pf.dwBBitMask == 0xff))

#define PF_IS_BGR5A1(pf) \
	((pf.dwFlags & DDPF_RGB) && \
	(pf.dwFlags & DDPF_ALPHAPIXELS) && \
	(pf.dwRGBBitCount == 16) && \
	(pf.dwRBitMask == 0x00007c00) && \
	(pf.dwGBitMask == 0x000003e0) && \
	(pf.dwBBitMask == 0x0000001f) && \
	(pf.dwAlphaBitMask == 0x00008000))

#define PF_IS_BGR565(pf) \
	((pf.dwFlags & DDPF_RGB) && \
	!(pf.dwFlags & DDPF_ALPHAPIXELS) && \
	(pf.dwRGBBitCount == 16) && \
	(pf.dwRBitMask == 0x0000f800) && \
	(pf.dwGBitMask == 0x000007e0) && \
	(pf.dwBBitMask == 0x0000001f))

#define PF_IS_INDEX8(pf) \
	((pf.dwFlags & DDPF_INDEXED) && \
	(pf.dwRGBBitCount == 8))

union DDS_header
{
	struct
	{
		uint dwMagic;
		uint dwSize;
		uint dwFlags;
		uint dwHeight;
		uint dwWidth;
		uint dwPitchOrLinearSize;
		uint dwDepth;
		uint dwMipMapCount;
		uint dwReserved1[ 11 ];

		// DDPIXELFORMAT
		struct
		{
			uint dwSize;
			uint dwFlags;
			uint dwFourCC;
			uint dwRGBBitCount;
			uint dwRBitMask;
			uint dwGBitMask;
			uint dwBBitMask;
			uint dwAlphaBitMask;
		} sPixelFormat;

		// DDCAPS2
		struct
		{
			uint dwCaps1;
			uint dwCaps2;
			uint dwDDSX;
			uint dwReserved;
		} sCaps;
		uint dwReserved2;
	};
	char data[128];
};	// DDS_header

struct DdsLoadInfo {
	bool compressed;
	bool swap;
	bool palette;
	uint divSize;
	uint blockBytes;
	GLenum internalFormat;
	GLenum externalFormat;
	GLenum type;
};

DdsLoadInfo loadInfoDXT1 = {
	true, false, false, 4, 8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GLenum(-1), GLenum(-1)
};
DdsLoadInfo loadInfoDXT3 = {
	true, false, false, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GLenum(-1), GLenum(-1)
};
DdsLoadInfo loadInfoDXT5 = {
	true, false, false, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GLenum(-1), GLenum(-1)
};
DdsLoadInfo loadInfoBGRA8 = {
	false, false, false, 1, 4, GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE
};
DdsLoadInfo loadInfoBGR8 = {
	false, false, false, 1, 3, GL_RGB8, GL_BGR, GL_UNSIGNED_BYTE
};
DdsLoadInfo loadInfoBGR5A1 = {
	false, true, false, 1, 2, GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV
};
DdsLoadInfo loadInfoBGR565 = {
	false, true, false, 1, 2, GL_RGB5, GL_RGB, GL_UNSIGNED_SHORT_5_6_5
};
DdsLoadInfo loadInfoIndex8 = {
	false, false, true, 1, 1, GL_RGB8, GL_BGRA, GL_UNSIGNED_BYTE
};

uint max(uint a, uint b)
{
	return a > b ? a : b;
}

//! Get the memory size of the given dds info and the image dimension
size_t getSize(uint w, uint h, DdsLoadInfo* li)
{
	if(li->compressed)
		return max(li->divSize, w) / li->divSize * max(li->divSize, h) / li->divSize * li->blockBytes;
	else
		return w * h * li->blockBytes;
}

class DdsLoader::LoaderImpl : public TextureLoaderBase::LoaderBaseImpl
{
public:
	LoaderImpl(DdsLoader& loader)
		:
		LoaderBaseImpl(loader),
		mMipMapCount(0),
		mLoadInfo(nullptr)
	{
	}

	sal_override ~LoaderImpl()
	{
	}

	int load(std::istream& is)
	{
		DDS_header hdr;
		is.read((char*)&hdr, sizeof(hdr));
		if(is.gcount() != sizeof(hdr)) return -1;
		if(hdr.dwMagic != DDS_MAGIC) return -1;
		if(hdr.dwSize != 124) return -1;

		if( hdr.dwMagic != DDS_MAGIC || hdr.dwSize != 124 ||
			!(hdr.dwFlags & DDSD_PIXELFORMAT) || !(hdr.dwFlags & DDSD_CAPS) )
		{
			return -1;
		}

		mWidth = hdr.dwWidth;
		mHeight = hdr.dwHeight;

		// The size of a dds image must be power of 2
		if(mWidth & (mWidth - 1))
			return -1;
		if(mHeight & (mHeight - 1))
			return -1;

		mMipMapCount = (hdr.dwFlags & DDSD_MIPMAPCOUNT) ? hdr.dwMipMapCount : 1;

		DdsLoadInfo* li;

		if(PF_IS_DXT1(hdr.sPixelFormat))
			li = &loadInfoDXT1;
		else if(PF_IS_DXT3(hdr.sPixelFormat))
			li = &loadInfoDXT3;
		else if(PF_IS_DXT5(hdr.sPixelFormat))
			li = &loadInfoDXT5;
		else if(PF_IS_BGRA8(hdr.sPixelFormat))
			li = &loadInfoBGRA8;
		else if(PF_IS_BGR8(hdr.sPixelFormat))
			li = &loadInfoBGR8;
		else if(PF_IS_BGR5A1(hdr.sPixelFormat))
			li = &loadInfoBGR5A1;
		else if(PF_IS_BGR565(hdr.sPixelFormat))
			li = &loadInfoBGR565;
		else if(PF_IS_INDEX8(hdr.sPixelFormat)) {
			li = &loadInfoIndex8;
			MCD_ASSERT(false && "Not implemented");
		}
		else
			return -1;

		mLoadInfo = li;
		mFormat = li->internalFormat;

		size_t size = 0;
		uint x = hdr.dwWidth;
		uint y = hdr.dwHeight;

		for(size_t i = 0; i < mMipMapCount; ++i) {
			size += getSize(x, y, li);
			x = (x + 1) >> 1;
			y = (y + 1) >> 1;
		}

		mImageData = ImageData(size);
		is.read(mImageData, size);
		if(size_t(is.gcount()) != size)
			return -1;

		return 0;
	}

	void upload()
	{
		DdsLoader& loader = static_cast<DdsLoader&>(mLoader);
		MCD_ASSERT(loader.loadingState == Loaded);
		(void)loader;

		glewInit();
		if(!glCompressedTexImage2D) {
			Log::write(Log::Error, "DdsLoader: glCompressedTexImage2D is not supported, operation aborted");
			return;
		}

		// Missing mipmaps won't be a problem anymore
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mMipMapCount - 1);

		DdsLoadInfo* li = mLoadInfo;
		char* p = mImageData;
		uint x = mWidth;
		uint y = mHeight;

		for(size_t i = 0; i < mMipMapCount; ++i) {
			size_t size = getSize(x, y, li);

			// TODO: Check opengl error
			if(li->compressed)
				glCompressedTexImage2D(GL_TEXTURE_2D, i, li->internalFormat, x, y, 0, size, p);
			else
				glTexImage2D(GL_TEXTURE_2D, i, li->internalFormat, x, y, 0, li->externalFormat, li->type, p);

			p += size;
			x = (x + 1) >> 1;
			y = (y + 1) >> 1;
		}

		// Restore to default
		// Reference: http://developer.apple.com/documentation/Darwin/Reference/ManPages/man3/glTexParameter.3.html
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
	}

	size_t mMipMapCount;
	DdsLoadInfo* mLoadInfo;
};	// LoaderImpl

DdsLoader::DdsLoader()
	: TextureLoaderBase()
{
	setImpl(new LoaderImpl(*this));
}

IResourceLoader::LoadingState DdsLoader::load(std::istream* is, const Path*, const char*)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mMutex);

	if(!is)
		loadingState = Aborted;
	else if(loadingState == Aborted)
		loadingState = NotLoaded;

	if(loadingState & Stopped)
		return loadingState;

	int result;
	{	// There is no need to do a mutex lock during loading, since
		// no body can access the mImageData if the loading isn't finished.
		ScopeUnlock unlock(mImpl->mMutex);
		result = static_cast<LoaderImpl*>(mImpl)->load(*is);
	}

	return (loadingState = (result == 0) ? Loaded : Aborted);
}

void DdsLoader::uploadData()
{
	MCD_ASSUME(mImpl != nullptr);
	LoaderImpl* impl = static_cast<LoaderImpl*>(mImpl);
	impl->upload();
}

}	// namespace MCD
