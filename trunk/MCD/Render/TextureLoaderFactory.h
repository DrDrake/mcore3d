#ifndef __MCD_RENDER_TEXTURELOADERFACTORY__
#define __MCD_RENDER_TEXTURELOADERFACTORY__

#include "ShareLib.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

class MCD_RENDER_API BitmapLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();
};	// BitmapLoaderFactory

class MCD_RENDER_API DdsLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();
};	// DdsLoaderFactory

class MCD_RENDER_API JpegLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();
};	// JpegLoaderFactory

class MCD_RENDER_API PngLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();
};	// PngLoaderFactory

}	// namespace MCD

#endif	// __MCD_RENDER_TEXTURELOADERFACTORY__
