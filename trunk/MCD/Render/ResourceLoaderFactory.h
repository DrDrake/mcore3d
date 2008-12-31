#ifndef __MCD_RENDER_RESOURCELOADERFACTORY__
#define __MCD_RENDER_RESOURCELOADERFACTORY__

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

class MCD_RENDER_API TgaLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();
};	// TgaLoaderFactory

class MCD_RENDER_API VertexShaderLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();
};	// VertexShaderLoaderFactory

class MCD_RENDER_API PixelShaderLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();
};	// PixelShaderLoaderFactory

class MCD_RENDER_API Max3dsLoaderFactory : public ResourceManager::IFactory
{
public:
	Max3dsLoaderFactory(ResourceManager& resourceManager);
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();

private:
	/*!	This loader factory is going to be owned by the mResourceManager, so we can
		use mResourceManager freely during the life-time of the loader factory.
	 */
	ResourceManager& mResourceManager;
};	// Max3dsLoaderFactory
/*
class MCD_RENDER_API PodLoaderFactory : public ResourceManager::IFactory
{
public:
	PodLoaderFactory(ResourceManager& resourceManager);
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();

private:
	ResourceManager& mResourceManager;
};	// PodLoaderFactory*/

class MCD_RENDER_API EffectLoaderFactory : public ResourceManager::IFactory
{
public:
	EffectLoaderFactory(ResourceManager& resourceManager);
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();

private:
	ResourceManager& mResourceManager;
};	// EffectLoaderFactory

}	// namespace MCD

#endif	// __MCD_RENDER_RESOURCELOADERFACTORY__