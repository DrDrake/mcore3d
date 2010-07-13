#include "Pch.h"
#include "ResourceLoaderFactory.h"
#include "BitmapLoader.h"
#include "CubemapLoader.h"
#include "DdsLoader.h"
#include "EffectLoader.h"
#include "FntLoader.h"
#include "JpegLoader.h"
#include "Max3dsLoader.h"
#include "PngLoader.h"
#include "ShaderLoader.h"
#include "TgaLoader.h"
#include "../Render/Effect.h"
#include "../Render/Font.h"
#include "../Render/Mesh.h"
#include "../Render/Model.h"
#include "../Render/Shader.h"
#include "../Render/Texture.h"
#include "../Core/Math/AnimationTrack.h"
#include "../Core/Math/AnimationTrackLoader.h"
#include "../Core/Math/Skeleton.h"
#include "../Core/Math/SkeletonLoader.h"
#include "../Core/System/Path.h"
#include "../Core/System/StrUtility.h"

// TODO: Remove the dependency on graphics API
#include "../../3Party/glew/glew.h"

namespace MCD {

ResourcePtr AnimationTrackLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "anim") == 0)
		return new AnimationTrack(fileId);
	return nullptr;
}

IResourceLoader* AnimationTrackLoaderFactory::createLoader()
{
	return new AnimationTrackLoader;
}

ResourcePtr BitmapLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "bmp") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* BitmapLoaderFactory::createLoader()
{
	return new BitmapLoader;
}

ResourcePtr CubemapLoaderFactory::createResource(const Path& fileId, const char* args)
{
	Path p(fileId);
    
    p.removeExtension();
    if(strCaseCmp(p.getExtension().c_str(), "cubemap") == 0)
		return new Texture(fileId);

	return nullptr;
}

IResourceLoader* CubemapLoaderFactory::createLoader()
{
	return new CubemapLoader;
}

ResourcePtr DdsLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "dds") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* DdsLoaderFactory::createLoader()
{
	return new DdsLoader;
}

EffectLoaderFactory::EffectLoaderFactory(IResourceManager& resourceManager)
	: mResourceManager(resourceManager)
{
}

ResourcePtr EffectLoaderFactory::createResource(const Path& fileId, const char* args)
{
	// We try to detect the fileId haveing a ".fx.xml" or not.
	if(strCaseCmp(fileId.getExtension().c_str(), "xml") != 0)
		return nullptr;

	Path p = fileId;
	p.removeExtension();
	if(strCaseCmp(p.getExtension().c_str(), "fx") != 0)
		return nullptr;

	return new Effect(fileId);
}

IResourceLoader* EffectLoaderFactory::createLoader()
{
	return new EffectLoader(mResourceManager);
}

FntLoaderFactory::FntLoaderFactory(IResourceManager& resourceManager)
	: mResourceManager(resourceManager)
{
}

ResourcePtr FntLoaderFactory::createResource(const Path& fileId, const char* args)
{
	std::string extStr = fileId.getExtension();
	const char* ext = extStr.c_str();
	if(strCaseCmp(ext, "fnt") == 0)
		return new BmpFont(fileId);
	return nullptr;
}

IResourceLoader* FntLoaderFactory::createLoader()
{
	return new FntLoader(&mResourceManager);
}

ResourcePtr JpegLoaderFactory::createResource(const Path& fileId, const char* args)
{
	std::string extStr = fileId.getExtension();
	const char* ext = extStr.c_str();
	if(strCaseCmp(ext, "jpg") == 0 || strCaseCmp(ext, "jpeg") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* JpegLoaderFactory::createLoader()
{
	return new JpegLoader;
}

Max3dsLoaderFactory::Max3dsLoaderFactory(IResourceManager& resourceManager)
	: mResourceManager(resourceManager)
{
}

ResourcePtr Max3dsLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "3ds") == 0)
		return new Model(fileId);
	return nullptr;
}

IResourceLoader* Max3dsLoaderFactory::createLoader()
{
	return new Max3dsLoader(&mResourceManager);
}

ResourcePtr PixelShaderLoaderFactory::createResource(const Path& fileId, const char* args)
{
	ShaderPtr shader;
	if(strCaseCmp(fileId.getExtension().c_str(), "glps") == 0)
		shader = new Shader(fileId);
	return shader;
}

IResourceLoader* PixelShaderLoaderFactory::createLoader()
{
	return new ShaderLoader(GL_FRAGMENT_SHADER);
}

ResourcePtr PngLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "png") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* PngLoaderFactory::createLoader()
{
	return new PngLoader;
}

ResourcePtr SkeletonLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "skt") == 0)
		return new Skeleton(fileId);
	return nullptr;
}

IResourceLoader* SkeletonLoaderFactory::createLoader()
{
	return new SkeletonLoader;
}

ResourcePtr TgaLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "tga") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* TgaLoaderFactory::createLoader()
{
	return new TgaLoader;
}

ResourcePtr VertexShaderLoaderFactory::createResource(const Path& fileId, const char* args)
{
	ShaderPtr shader;
	if(strCaseCmp(fileId.getExtension().c_str(), "glvs") == 0)
		shader = new Shader(fileId);
	return shader;
}

IResourceLoader* VertexShaderLoaderFactory::createLoader()
{
	return new ShaderLoader(GL_VERTEX_SHADER);
}

}	// namespace MCD
