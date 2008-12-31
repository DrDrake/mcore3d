#include "Pch.h"
#include "EffectLoader.h"
#include "Effect.h"
#include "Material.h"
#include "Shader.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "../Core/System/Log.h"
#include "../Core/System/PtrVector.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/Utility.h"	// for MCD_FOREACH
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

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
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

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
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

class ShaderLoader : public EffectLoader::ILoader
{
	class Callback : public ResourceManagerCallback
	{
	public:
		//! Create shader from the shader code right inside the effect xml
		bool createInlineShader(const wchar_t* sourceCode, uint shaderType)
		{
			if(!sourceCode)
				return false;

			InlineSource source;
			if(wStrToStr(sourceCode, source.code)) {
				source.type = shaderType;
				inlineSources.push_back(source);
				return true;
			}
			else {
				Log::write(Log::Error, L"Fail to convert inline shader source");
				return false;
			}
		}

		sal_override void doCallback()
		{
			if(!program->handle)
				program->create();

			program->detachAll();

			// Compile and attach all inline shaders
			MCD_FOREACH(const InlineSource& source, inlineSources) {
				ShaderPtr shader = new Shader(L"");	// This resource is not going to put in the manager, no name is needed
				shader->create(source.type);
				if(shader->compile(source.code.c_str()))
					program->attach(*shader);
				else {
					std::string log;
					shader->getLog(log);
					// TODO: Not using %S for every platform
					Log::format(Log::Error, L"%S", log.c_str());
				}
			}

			// Attach all external shaders
			MCD_FOREACH(const ShaderPtr& shader, shaders) {
				if(shader)
					program->attach(*shader);
			}

			if(!program->link()) {
				std::string log;
				program->getLog(log);
				Log::write(Log::Error, strToWStr(log).c_str());
			}
		}

		SharedPtr<ShaderProgram> program;
		//! The shaders that the ShaderProgram depends, excluding inlineShaders
		std::vector<ShaderPtr> shaders;

		//! Source code for the inline shaders
		struct InlineSource { std::string code; uint type; };
		std::vector<InlineSource> inlineSources;
	};	// Callback

public:
	sal_override const wchar_t* name() const {
		return L"shader";
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
	{
		if(parser.isEmptyElement())
			return true;

		typedef XmlParser::Event Event;
		const wchar_t* shaderFile = nullptr;

		std::auto_ptr<ShaderProperty> shaderProperty(new ShaderProperty(new ShaderProgram));
		std::auto_ptr<Callback> callback(new Callback);
		callback->program = shaderProperty->shaderProgram;

		// Indicate the currect shader type
		uint shaderType = 0;

		bool done = false;
		while(!done) switch(parser.nextEvent())
		{
		case Event::BeginElement:
			if(wstrCaseCmp(parser.elementName(), L"vertex") == 0)
				shaderType = GL_VERTEX_SHADER;
			else if(wstrCaseCmp(parser.elementName(), L"fragment") == 0)
				shaderType = GL_FRAGMENT_SHADER;

			if(shaderType == 0)
				return false;

			if((shaderFile = parser.attributeValueIgnoreCase(L"file")) != nullptr) {
				Path path(shaderFile);
				path = path.hasRootDirectory() ? path : context.basePath / path;

				ShaderPtr shader = dynamic_cast<Shader*>(context.resourceManager.load(path).get());
				callback->addDependency(path);
				callback->shaders.push_back(shader);
			}
			break;

		case Event::CData:
			if(!callback->createInlineShader(parser.textData(), shaderType))
				return false;
			break;

		case Event::EndElement:
			if(wstrCaseCmp(parser.elementName(), L"shader") == 0)
				done = true;
			break;

		case Event::Error:
		case Event::EndDocument:
			return false;

		default:
			break;
		}

		context.resourceManager.addCallback(callback.get());
		callback.release();

		material.addProperty(shaderProperty.get(), context.pass);
		shaderProperty.release();

		return true;
	}
};	// ShaderLoader

class PassLoader : public EffectLoader::ILoader
{
public:
	PassLoader()
	{
		mLoaders.push_back(new StandardLoader);
		mLoaders.push_back(new TextureLoader);
		mLoaders.push_back(new ShaderLoader);
	}

	sal_override const wchar_t* name() const {
		return L"pass";
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
	{
		typedef XmlParser::Event Event;

		// If the pass is disabled, skip the whole element.
		if(!parser.attributeValueAsBoolIgnoreCase(L"enable", true/*By default a pass is enabled*/))
		{
			if(parser.isEmptyElement())
				return true;
			while(true) switch(parser.nextEvent()) {
			case Event::EndElement:
				if(wstrCaseCmp(parser.elementName(), L"pass") == 0)
					return true;
				break;

			case Event::Error:
			case Event::EndDocument:
				return false;

			default:
				break;
			}
		}

		const wchar_t* attributeValue = nullptr;
		// Parse the attributes of "pass"
		if(parser.attributeValueAsBoolIgnoreCase(L"drawLine", false))
			material.addProperty(new LineDrawingProperty, context.pass);

		if(parser.attributeValueIgnoreCase(L"lineWidth"))
			material.addProperty(new LineWidthProperty(parser.attributeValueAsFloatIgnoreCase(L"lineWidth", 1.0f)), context.pass);

		if((attributeValue = parser.attributeValueIgnoreCase(L"cullMode")) != nullptr) {
			if(wstrCaseCmp(attributeValue, L"none") == 0)
				material.addProperty(new DisableStateProperty(GL_CULL_FACE), context.pass);
			else if(wstrCaseCmp(attributeValue, L"front") == 0)
				material.addProperty(new FrontCullingProperty, context.pass);
		}

		if(parser.isEmptyElement()) {
			++context.pass;
			return true;
		}

		while(true) switch(parser.nextEvent())
		{
		case Event::BeginElement:
			// Search for a loader that will response with this xml element
			for(Loaders::iterator i=mLoaders.begin(); i!=mLoaders.end(); ++i) {
				if(wstrCaseCmp(parser.elementName(), i->name()) != 0)
					continue;
				if(!i->load(parser, material, context))
					return false;
				break;
			}
			break;

		case Event::EndElement:
			if(wstrCaseCmp(parser.elementName(), L"pass") == 0) {
				++context.pass;
				return true;
			}
			break;

		case Event::Error:
		case Event::EndDocument:
			return false;

		default:
			break;
		}

		return true;
	}

	typedef ptr_vector<ILoader> Loaders;
	Loaders mLoaders;
};	// PassLoader

}	// namespace

class EffectLoader::Impl
{
public:
	Impl(IResourceManager& resourceManager)
		: mResourceManager(resourceManager)
	{
		mLoaders.push_back(new PassLoader);
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
		XmlParser parser;
		parser.parse(const_cast<wchar_t*>(xmlString.c_str()));
		mLoadingState = Loaded;

		while(true) switch(parser.nextEvent())
		{
		case Event::BeginElement:
			// Search for a loader that will response with this xml element
			for(Loaders::iterator i=mLoaders.begin(); i!=mLoaders.end(); ++i) {
				if(wstrCaseCmp(parser.elementName(), i->name()) != 0)
					continue;
				if(!i->load(parser, mMaterial, context))
					return mLoadingState = Aborted;
			}
			break;

		case Event::Error:
			mLoadingState = Aborted;
		case Event::EndDocument:
			return mLoadingState;

		default:
			break;
		}

		return mLoadingState;
	}

	void commit(Resource& resource)
	{
		Effect& effect = dynamic_cast<Effect&>(resource);
		effect.material.reset(static_cast<Material2*>(mMaterial.clone()));
	}

	IResourceLoader::LoadingState mLoadingState;

	typedef ptr_vector<ILoader> Loaders;
	Loaders mLoaders;

	Material2 mMaterial;

	IResourceManager& mResourceManager;
};	// Impl

EffectLoader::EffectLoader(IResourceManager& resourceManager)
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
