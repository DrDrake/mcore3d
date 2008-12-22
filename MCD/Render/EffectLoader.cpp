#include "Pch.h"
#include "EffectLoader.h"
#include "Effect.h"
#include "Material.h"
#include "Texture.h"
#include "../Core/System/PtrVector.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/XmlParser.h"
#include <sstream>

namespace MCD {

namespace {

static bool parseColor4f(const wchar_t* str, ColorRGBAf& color)
{
	if(!str)
		return true;

	// %*s means ignoring any non-numeric characters
	return swscanf(str, L"%f%*s%f%*s%f%*s%f", &color.r, &color.g, &color.b, &color.a) == 4;
}

class StandardLoader : public EffectLoader::ILoader
{
public:
	sal_override const wchar_t* name() const {
		return L"standard";
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, const Context& context)
	{
		ColorRGBAf ambient(0, 0);
		if(!parseColor4f(parser.attributeValueIgnoreCase(L"ambient"), ambient)) return false;
		ColorRGBAf diffuse(0, 0);
		if(!parseColor4f(parser.attributeValueIgnoreCase(L"diffuse"), diffuse)) return false;
		ColorRGBAf specular(0, 0);
		if(!parseColor4f(parser.attributeValueIgnoreCase(L"specular"), specular)) return false;

		float shininess = parser.attributeValueAsFloatIgnoreCase(L"shininess", 0.0f);

		// TODO: Remove the hard coded ColorOperation
		material.addProperty(
			new StandardProperty(
				ambient, diffuse, specular, ColorProperty::ColorOperation::Replace, shininess
			), context.pass
		);

		return true;
	}
};	// StandardLoader

class TextureLoader : public EffectLoader::ILoader
{
public:
	TextureLoader() : mTextureUnit(0) {}

	sal_override const wchar_t* name() const {
		return L"texture";
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, const Context& context)
	{
		const wchar_t* file = parser.attributeValueIgnoreCase(L"file");
		if(!file || file[0] == L'\0')
			return false;

		Path path(file);
		path = path.hasRootDirectory() ? path : context.basePath / path;
		TexturePtr texture = dynamic_cast<Texture*>(context.resourceManager.load(path, false).get());

		std::auto_ptr<TextureProperty> textureProperty(new TextureProperty(texture.get(), mTextureUnit));

		const wchar_t* shaderName = parser.attributeValueIgnoreCase(L"shaderName");
		if(shaderName)
			if(!wStrToStr(shaderName, textureProperty->shaderName))
				textureProperty->shaderName.clear();

		material.addProperty(textureProperty.get(), context.pass);
		textureProperty.release();
		++mTextureUnit;

		return true;
	}

	size_t mTextureUnit;	//! This variable will keep increasing every time a load operation is performed
};	// TextureLoader

class ModeLoader : public EffectLoader::ILoader
{
public:
	sal_override const wchar_t* name() const {
		return L"mode";
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, const Context& context)
	{
		if(parser.attributeValueAsBoolIgnoreCase(L"drawLine", false))
			material.addProperty(new LineDrawingProperty, context.pass);

		return true;
	}
};	// ModeLoader

}	// namespace

class EffectLoader::Impl
{
public:
	Impl(ResourceManager& resourceManager)
		: mIsCurrentPassEnabled(true), mResourceManager(resourceManager)
	{
		mLoaders.push_back(new StandardLoader);
		mLoaders.push_back(new TextureLoader);
		mLoaders.push_back(new ModeLoader);
	}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId)
	{
		mLoadingState = is && fileId ? NotLoaded : Aborted;

		if(mLoadingState & Stopped)
			return mLoadingState;

		MCD_ASSUME(is);
		MCD_ASSUME(fileId);

		std::wstring xmlString;
		{	// A simple way to grab all the content from the stream
			std::stringstream ss;
			*is >> ss.rdbuf();
			if(!utf8ToWStr(ss.str().c_str(), xmlString))
				xmlString.clear();
			mLoadingState = xmlString.empty() ? Aborted : mLoadingState;
		}

		if(mLoadingState & Stopped)
			return mLoadingState;

		ILoader::Context context = { 0, fileId->getBranchPath(), nullptr, mResourceManager };

		// Parse the xml
		typedef XmlParser::Event Event;
		bool ended = false;
		XmlParser parser;
		parser.parse(const_cast<wchar_t*>(xmlString.c_str()));

		while(!ended)
		{
			Event::Enum e = parser.nextEvent();

			switch(e)
			{
			case Event::BeginElement:
				if(!mIsCurrentPassEnabled)
					break;

				if(wstrCaseCmp(parser.elementName(), L"pass") == 0)
					mIsCurrentPassEnabled = parser.attributeValueAsBoolIgnoreCase(L"enable", true);

				// Search for a loader that will response with this xml element
				for(ptr_vector<ILoader>::iterator i=mLoaders.begin(); i!=mLoaders.end(); ++i) {
					if(wstrCaseCmp(parser.elementName(), i->name()) != 0)
						continue;
					if(!i->load(parser, mMaterial, context)) {
						mLoadingState = Aborted;
						ended = true;
					}
					break;
				}
				break;

			case Event::EndElement:
				if(wstrCaseCmp(parser.elementName(), L"pass") == 0) {
					mIsCurrentPassEnabled = true;
					++context.pass;
				}
				break;

			case Event::Error:
			case Event::EndDocument:
				ended = true;
				break;

			default:
				break;
			}
		}

		mLoadingState = Loaded;

		return mLoadingState;
	}

	void commit(Resource& resource)
	{
		Effect& effect = dynamic_cast<Effect&>(resource);
		effect.material = static_cast<Material2*>(mMaterial.clone());
	}

	IResourceLoader::LoadingState mLoadingState;

	ptr_vector<ILoader> mLoaders;
	Material2 mMaterial;

	bool mIsCurrentPassEnabled;

	ResourceManager& mResourceManager;
};	// Impl

EffectLoader::EffectLoader(ResourceManager& resourceManager)
{
	mImpl = new Impl(resourceManager);
}

EffectLoader::~EffectLoader()
{
	delete mImpl;
}

IResourceLoader::LoadingState EffectLoader::load(std::istream* is, const Path* fileId)
{
	MCD_ASSUME(mImpl);
	return mImpl->load(is, fileId);
}

void EffectLoader::commit(Resource& resource)
{
	MCD_ASSUME(mImpl);
	mImpl->commit(resource);
}

IResourceLoader::LoadingState EffectLoader::getLoadingState() const
{
	MCD_ASSUME(mImpl);
	return mImpl->mLoadingState;
}

}	// namespace MCD
