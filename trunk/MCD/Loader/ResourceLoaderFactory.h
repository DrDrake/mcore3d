#ifndef __MCD_LOADER_RESOURCELOADERFACTORY__
#define __MCD_LOADER_RESOURCELOADERFACTORY__

#include "ShareLib.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

class MCD_LOADER_API AnimationTrackLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoader* createLoader();
};	// AnimationTrackLoaderFactory

class MCD_LOADER_API CubemapLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoader* createLoader();
};	// CubemapLoaderFactory

class MCD_LOADER_API EffectLoaderFactory : public ResourceManager::IFactory
{
public:
	EffectLoaderFactory(IResourceManager& resourceManager);
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoader* createLoader();

private:
	IResourceManager& mResourceManager;
};	// EffectLoaderFactory

class MCD_LOADER_API FntLoaderFactory : public ResourceManager::IFactory
{
public:
	FntLoaderFactory(IResourceManager& resourceManager);
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoader* createLoader();

private:
	IResourceManager& mResourceManager;
};	// FntLoaderFactory

class MCD_LOADER_API Max3dsLoaderFactory : public ResourceManager::IFactory
{
public:
	Max3dsLoaderFactory(IResourceManager& resourceManager);
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoader* createLoader();

private:
	/*!	This loader factory is going to be owned by the mResourceManager, so we can
		use mResourceManager freely during the life-time of the loader factory.
	 */
	IResourceManager& mResourceManager;
};	// Max3dsLoaderFactory

class MCD_LOADER_API PixelShaderLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoader* createLoader();
};	// PixelShaderLoaderFactory

class MCD_LOADER_API SkeletonLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoader* createLoader();
};	// SkeletonLoaderFactory

class MCD_LOADER_API VertexShaderLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoader* createLoader();
};	// VertexShaderLoaderFactory

}	// namespace MCD

#endif	// __MCD_LOADER_RESOURCELOADERFACTORY__
