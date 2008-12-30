#include "Pch.h"
#include "ResourceLoaderFactory.h"
#include "BitmapLoader.h"
#include "DdsLoader.h"
#include "Effect.h"
#include "EffectLoader.h"
#include "JpegLoader.h"
#include "Max3dsLoader.h"
#include "Model.h"
#include "PngLoader.h"
#include "Shader.h"
#include "ShaderLoader.h"
#include "Texture.h"
#include "TgaLoader.h"
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

ResourcePtr TgaLoaderFactory::createResource(const Path& fileId)
{
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"tga") == 0)
		return new Texture(fileId);
	return nullptr;
}

IResourceLoader* TgaLoaderFactory::createLoader()
{
	return new TgaLoader;
}

ResourcePtr VertexShaderLoaderFactory::createResource(const Path& fileId)
{
	ShaderPtr shader;
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"glvs") == 0)
		shader = new Shader(fileId);
	return shader;
}

IResourceLoader* VertexShaderLoaderFactory::createLoader()
{
	return new ShaderLoader(GL_VERTEX_SHADER);
}

ResourcePtr PixelShaderLoaderFactory::createResource(const Path& fileId)
{
	ShaderPtr shader;
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"glps") == 0)
		shader = new Shader(fileId);
	return shader;
}

IResourceLoader* PixelShaderLoaderFactory::createLoader()
{
	return new ShaderLoader(GL_FRAGMENT_SHADER);
}

Max3dsLoaderFactory::Max3dsLoaderFactory(ResourceManager& resourceManager)
	: mResourceManager(resourceManager)
{
}

ResourcePtr Max3dsLoaderFactory::createResource(const Path& fileId)
{
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"3ds") == 0)
		return new Model(fileId);
	return nullptr;
}

IResourceLoader* Max3dsLoaderFactory::createLoader()
{
	return new Max3dsLoader(&mResourceManager);
}

EffectLoaderFactory::EffectLoaderFactory(ResourceManager& resourceManager)
	: mResourceManager(resourceManager)
{
}

ResourcePtr EffectLoaderFactory::createResource(const Path& fileId)
{
	// We try to detect the fileId haveing a ".fx.xml" or not.
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"xml") != 0)
		return nullptr;

	Path p = fileId;
	p.removeExtension();
	if(wstrCaseCmp(p.getExtension().c_str(), L"fx") != 0)
		return nullptr;

	return new Effect(fileId);
}

IResourceLoader* EffectLoaderFactory::createLoader()
{
	return new EffectLoader(mResourceManager);
}

}	// namespace MCD
