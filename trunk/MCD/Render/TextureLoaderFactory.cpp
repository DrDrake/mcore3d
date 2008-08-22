#include "Pch.h"
#include "TextureLoaderFactory.h"
#include "BitmapLoader.h"
#include "DdsLoader.h"
#include "JpegLoader.h"
#include "PngLoader.h"
#include "Texture.h"
#include "../Core/System/Path.h"
#include "../Core/System/StrUtility.h"

namespace MCD {

ResourcePtr BitmapLoaderFactory::createResource(const Path& fileId)
{
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"bmp") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* BitmapLoaderFactory::createLoader()
{
	return new BitmapLoader;
}

ResourcePtr DdsLoaderFactory::createResource(const Path& fileId)
{
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"dds") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* DdsLoaderFactory::createLoader()
{
	return new DdsLoader;
}

ResourcePtr JpegLoaderFactory::createResource(const Path& fileId)
{
	std::wstring extStr = fileId.getExtension();
	const wchar_t* ext = extStr.c_str();
	if(wstrCaseCmp(ext, L"jpg") == 0 || wstrCaseCmp(ext, L"jpeg") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* JpegLoaderFactory::createLoader()
{
	return new JpegLoader;
}

ResourcePtr PngLoaderFactory::createResource(const Path& fileId)
{
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"png") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* PngLoaderFactory::createLoader()
{
	return new PngLoader;
}

}	// namespace MCD
