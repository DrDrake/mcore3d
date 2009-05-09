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

/*!Parse Color4f, returns true if succeed. */
static bool parseColor4f(const wchar_t* str, ColorRGBAf& color)
{
	if(!str)
		return true;

	// %*s means ignoring any non-numeric characters
	return swscanf(str, L"%f%*s%f%*s%f%*s%f", &color.r, &color.g, &color.b, &color.a) == 4;
}

class PassLoader : public EffectLoader::ILoader
{
	/*!	Callback that bind attributes to ShaderProgram
		The dependence system can make sure all ShaderProgram is linked before invoking this callback
	 */
	class Callback : public ResourceManagerCallback
	{
	public:
		// Bind texture to the shader's uniform
		sal_override void doCallback()
		{
			MCD_ASSERT(shaderProgram);

			shaderProgram->bind();

			MCD_FOREACH(const TextureProperty& texture, textures) {
				if(texture.shaderName.empty())
					continue;
				GLint location = glGetUniformLocation(shaderProgram->handle, texture.shaderName.c_str());
				if(location >= 0)
					glUniform1i(location, texture.unit);
				else
					Log::format(Log::Error, L"Fail to bind texture uniform '%s'", strToWStr(texture.shaderName).c_str());
			}

			shaderProgram->unbind();
		}

		ptr_vector<TextureProperty> textures;
		SharedPtr<ShaderProgram> shaderProgram;
	};	// Callback

public:
	PassLoader();

	sal_override const wchar_t* name() const {
		return L"pass";
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context);

	typedef ptr_vector<ILoader> Loaders;
	Loaders mLoaders;

	std::auto_ptr<Callback> mBindTextureUniformCallback;
};	// PassLoader

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
	TextureLoader(PassLoader& passLoader) : mPassLoader(passLoader), mTextureUnit(0) {}

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
		if(shaderName) {
			if(!wStrToStr(shaderName, textureProperty->shaderName))
				textureProperty->shaderName.clear();
		}

		material.addProperty(textureProperty.get(), context.pass);
		mPassLoader.mBindTextureUniformCallback->textures.push_back(dynamic_cast<TextureProperty*>(textureProperty->clone()));
		textureProperty.release();
		++mTextureUnit;

		return true;
	}

	size_t mTextureUnit;	//! This variable will keep increasing every time a load operation is performed
	PassLoader& mPassLoader;
};	// TextureLoader

class ShaderLoader : public EffectLoader::ILoader
{
	class MCD_ABSTRACT_CLASS Param
	{
	public:
		virtual ~Param() {
			delete[] mIntValues;
			delete[] mFloatValues;
		}

		virtual void bind(uint shaderProgramHandle) const = 0;

		void initAsInt(const char* name, const wchar_t* value) {
			mName = name;
			mFloatValues = nullptr;
			mIntValues = wStrToIntArray(value, mCount);
		}

		void initAsFloat(const char* name, const wchar_t* value) {
			mName = name;
			mIntValues = nullptr;
			mFloatValues = wStrToFloatArray(value, mCount);
		}

	protected:
		Param() : mIntValues(nullptr), mFloatValues(nullptr), mCount(0) {}

		int getUniformLocation(uint shaderProgramHandle) const {
			int location = glGetUniformLocation(shaderProgramHandle, mName.c_str());
			if(location < 0)
				Log::format(Log::Error, L"Fail to bind uniform '%s'", strToWStr(mName).c_str());
			return location;
		}

		std::string mName;
		int* mIntValues;
		float* mFloatValues;
		size_t mCount;	//!< Number of float
	};	// Param

	//! Callback that attach shader to shader program and linking them together
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
				return;
			}

			// Bind shader parameters
			program->bind();
			MCD_FOREACH(const Param& param, shaderParams) {
				param.bind(program->handle);
			}
			program->unbind();
		}

		SharedPtr<ShaderProgram> program;
		//! The shaders that the ShaderProgram depends, excluding inlineShaders
		std::vector<ShaderPtr> shaders;

		//! Source code for the inline shaders
		struct InlineSource { std::string code; uint type; };
		std::vector<InlineSource> inlineSources;

		//! Shader parameters
		ptr_vector<Param> shaderParams;
	};	// Callback

	class IntParam : public Param
	{
	public:
		sal_override void bind(uint shaderProgramHandle) const {
			int location = getUniformLocation(shaderProgramHandle);
			if(location >= 0 && mCount > 0)
				glUniform1iv(location, mCount, mIntValues);
		}

		static Param* create(const char* name, const wchar_t* value) {
			std::auto_ptr<Param> param(new IntParam);
			param->initAsInt(name, value);
			return param.release();
		}
	};	// IntParam

	class FloatParam : public Param
	{
	public:
		sal_override void bind(uint shaderProgramHandle) const {
			int location = getUniformLocation(shaderProgramHandle);
			if(location >= 0 && mCount > 0)
				glUniform1fv(location, mCount, mFloatValues);
		}

		static Param* create(const char* name, const wchar_t* value) {
			std::auto_ptr<Param> param(new FloatParam);
			param->initAsFloat(name, value);
			return param.release();
		}
	};	// FloatParam

	class Vec2Param : public Param
	{
	public:
		sal_override void bind(uint shaderProgramHandle) const {
			int location = getUniformLocation(shaderProgramHandle);
			if(mCount % 2 != 0)
				Log::format(Log::Warn, L"Number of float in shader parameter '%S' do not match the type vec2", mName.c_str());
			if(location >= 0 && mCount > 0)
				glUniform2fv(location, mCount/2, mFloatValues);
		}

		static Param* create(const char* name, const wchar_t* value) {
			std::auto_ptr<Param> param(new Vec2Param);
			param->initAsFloat(name, value);
			return param.release();
		}
	};	// Vec2Param

	class Vec3Param : public Param
	{
	public:
		sal_override void bind(uint shaderProgramHandle) const {
			int location = getUniformLocation(shaderProgramHandle);
			if(mCount % 3 != 0)
				Log::format(Log::Warn, L"Number of float in shader parameter '%S' do not match the type vec3", mName.c_str());
			if(location >= 0 && mCount > 0)
				glUniform3fv(location, mCount/3, mFloatValues);
		}

		static Param* create(const char* name, const wchar_t* value) {
			std::auto_ptr<Param> param(new Vec3Param);
			param->initAsFloat(name, value);
			return param.release();
		}
	};	// Vec3Param

	class Vec4Param : public Param
	{
	public:
		sal_override void bind(uint shaderProgramHandle) const {
			int location = getUniformLocation(shaderProgramHandle);
			if(mCount % 4 != 0)
				Log::format(Log::Warn, L"Number of float in shader parameter '%S' do not match the type vec4", mName.c_str());
			if(location >= 0 && mCount > 0)
				glUniform4fv(location, mCount/4, mFloatValues);
		}

		static Param* create(const char* name, const wchar_t* value) {
			std::auto_ptr<Param> param(new Vec4Param);
			param->initAsFloat(name, value);
			return param.release();
		}
	};	// Vec4Param

	// Creates different kinds of shader param according to it's type
	void createParam(const wchar_t* name_, const wchar_t* type, const wchar_t* value, Callback& callback)
	{
		if(!name_ || !type || !value || value[0] == L'\0')
			return;

		std::string name;
		if(!wStrToStr(name_, name))
			return;

		if(wstrCaseCmp(type, L"int") == 0)
			return callback.shaderParams.push_back(IntParam::create(name.c_str(), value));
		else if(wstrCaseCmp(type, L"float") == 0)
			return callback.shaderParams.push_back(FloatParam::create(name.c_str(), value));
		else if(wstrCaseCmp(type, L"vec2") == 0)
			return callback.shaderParams.push_back(Vec2Param::create(name.c_str(), value));
		else if(wstrCaseCmp(type, L"vec3") == 0)
			return callback.shaderParams.push_back(Vec3Param::create(name.c_str(), value));
	}

public:
	ShaderLoader(PassLoader& passLoader) : mPassLoader(passLoader) {}

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
		mPassLoader.mBindTextureUniformCallback->shaderProgram = callback->program = shaderProperty->shaderProgram;

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

				// Make sure the uniform binding is done after the shader program is commited
				mPassLoader.mBindTextureUniformCallback->addDependency(path);
			}
			break;

		case Event::Text:
			// Parse the value of a shader parameter
			if(wstrCaseCmp(parser.elementName(), L"parameter") == 0)
				createParam(
					parser.attributeValueIgnoreCase(L"name"),
					parser.attributeValueIgnoreCase(L"type"),
					parser.textData(),
					*callback
				);
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

	PassLoader& mPassLoader;
};	// ShaderLoader

class BlendingLoader : public EffectLoader::ILoader
{
public:
	sal_override const wchar_t* name() const {
		return L"blending";
	}

	static void parseGLBlendFactor(const wchar_t* str, int defaultVal, int& result)
	{
		result = defaultVal;

		if(NULL != str)
		{
			if(0 == wstrCaseCmp(str, L"zero"))
				result = GL_ZERO;

			else if(0 == wstrCaseCmp(str, L"one"))
				result = GL_ONE;

			else if(0 == wstrCaseCmp(str, L"src_color"))
				result = GL_SRC_COLOR;

			else if(0 == wstrCaseCmp(str, L"one_minus_src_color"))
				result = GL_ONE_MINUS_SRC_COLOR;

			else if(0 == wstrCaseCmp(str, L"dst_color"))
				result = GL_DST_COLOR;

			else if(0 == wstrCaseCmp(str, L"one_minus_dst_color"))
				result = GL_ONE_MINUS_DST_COLOR;

			else if(0 == wstrCaseCmp(str, L"src_alpha"))
				result = GL_SRC_ALPHA;

			else if(0 == wstrCaseCmp(str, L"one_minus_src_alpha"))
				result = GL_ONE_MINUS_SRC_ALPHA;

			else if(0 == wstrCaseCmp(str, L"dst_alpha"))
				result = GL_DST_ALPHA;

			else if(0 == wstrCaseCmp(str, L"one_minus_dst_alpha"))
				result = GL_ONE_MINUS_DST_ALPHA;

			else if(0 == wstrCaseCmp(str, L"constant_color"))
				result = GL_CONSTANT_COLOR;

			else if(0 == wstrCaseCmp(str, L"one_minus_constant_color"))
				result = GL_ONE_MINUS_CONSTANT_COLOR;

			else if(0 == wstrCaseCmp(str, L"constant_alpha"))
				result = GL_CONSTANT_ALPHA;

			else if(0 == wstrCaseCmp(str, L"one_minus_constant_alpha"))
				result = GL_ONE_MINUS_CONSTANT_ALPHA;
		}
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
	{
		BlendingProperty* prop = new BlendingProperty();

		// color blending
		prop->blendEnable = parser.attributeValueAsBoolIgnoreCase(L"colorBlend", false);
		parseGLBlendFactor(parser.attributeValueIgnoreCase(L"srcFactor"), GL_ONE, prop->sfactor);
		parseGLBlendFactor(parser.attributeValueIgnoreCase(L"dstFactor"), GL_ZERO, prop->dfactor);

		// separate color blending
		prop->blendEnableSep = parser.attributeValueAsBoolIgnoreCase(L"colorBlendSep", false);
		parseGLBlendFactor(parser.attributeValueIgnoreCase(L"srcFactorSep"), GL_ONE, prop->sfactorSep);
		parseGLBlendFactor(parser.attributeValueIgnoreCase(L"dstFactorSep"), GL_ZERO, prop->dfactorSep);

		// blendcolor
		parseColor4f(parser.attributeValueIgnoreCase(L"blendColor"), prop->blendColor);

		material.addProperty(prop, context.pass);

		return true;
	}
};	// BlendingLoader

class DepthStencilLoader : public EffectLoader::ILoader
{
public:
	sal_override const wchar_t* name() const {
		return L"depthStencil";
	}

	static void parseGLCmpFunc(const wchar_t* str, int defaultVal, int& result)
	{
		result = defaultVal;

		if(NULL != str)
		{
			if(0 == wstrCaseCmp(str, L"never"))
				result = GL_NEVER;

			else if(0 == wstrCaseCmp(str, L"less"))
				result = GL_LESS;

			else if(0 == wstrCaseCmp(str, L"equal"))
				result = GL_EQUAL;

			else if(0 == wstrCaseCmp(str, L"less_equal"))
				result = GL_LEQUAL;

			else if(0 == wstrCaseCmp(str, L"greater"))
				result = GL_GREATER;

			else if(0 == wstrCaseCmp(str, L"not_equal"))
				result = GL_NOTEQUAL;

			else if(0 == wstrCaseCmp(str, L"greater_equal"))
				result = GL_GEQUAL;

			else if(0 == wstrCaseCmp(str, L"always"))
				result = GL_ALWAYS;
		}
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
	{
		DepthStencilProperty* prop = new DepthStencilProperty();

		// depth test
		prop->depthTestEnable = parser.attributeValueAsBoolIgnoreCase(L"depthTest", true);
		prop->depthWriteEnable = parser.attributeValueAsBoolIgnoreCase(L"depthWrite", true);
		parseGLCmpFunc(parser.attributeValueIgnoreCase(L"depthFunc"), GL_LESS, prop->depthFunc);

		// stencil test

		material.addProperty(prop, context.pass);

		return true;
	}
};	// DepthStencilLoader

PassLoader::PassLoader()
{
	mLoaders.push_back(new StandardLoader);
	mLoaders.push_back(new TextureLoader(*this));
	mLoaders.push_back(new ShaderLoader(*this));
	mLoaders.push_back(new BlendingLoader);
	mLoaders.push_back(new DepthStencilLoader);
}

bool PassLoader::load(XmlParser& parser, IMaterial& material, Context& context)
{
	typedef XmlParser::Event Event;

	mBindTextureUniformCallback.reset(new Callback);

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

	bool done = false;
	while(!done) switch(parser.nextEvent())
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
			done = true;
		}
		break;

	case Event::Error:
	case Event::EndDocument:
		return false;

	default:
		break;
	}

	// Add callback to bind texture to shader, if needed
	if(mBindTextureUniformCallback->shaderProgram && !mBindTextureUniformCallback->textures.empty()) {
		mBindTextureUniformCallback->addDependency(context.effectPath);
		context.resourceManager.addCallback(mBindTextureUniformCallback.get());
		mBindTextureUniformCallback.release();
	}

	return true;
}

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
		// FileId must not be null, otherwise abort
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

		ILoader::Context context = { 0, *fileId, fileId->getBranchPath(), nullptr, mResourceManager };

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
