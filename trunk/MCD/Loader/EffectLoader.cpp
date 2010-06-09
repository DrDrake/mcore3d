#include "Pch.h"
#include "EffectLoader.h"
#include "../Render/Effect.h"
#include "../Render/Material.h"
#include "../Render/Shader.h"
#include "../Render/ShaderProgram.h"
#include "../Render/Texture.h"
#include "../Core/System/Log.h"
#include "../Core/System/PtrVector.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/Utility.h"	// for MCD_FOREACH
#include "../Core/System/XmlParser.h"
#include <sstream>

// TODO: Remove the dependency on graphics API
#include "../../3Party/glew/glew.h"

namespace MCD {

namespace {

/*!Parse Color4f, returns true if succeed. */
static bool parseColor4f(const char* str, ColorRGBAf& color)
{
	if(!str)
		return true;

	// %*s means ignoring any non-numeric characters
	return sscanf(str, "%f%*s%f%*s%f%*s%f", &color.r, &color.g, &color.b, &color.a) == 4;
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
					Log::format(Log::Error, "Fail to bind texture uniform '%s'", texture.shaderName.c_str());
			}

			shaderProgram->unbind();
		}

		ptr_vector<TextureProperty> textures;
		SharedPtr<ShaderProgram> shaderProgram;
	};	// Callback

public:
	PassLoader();

	sal_override const char* name() const {
		return "pass";
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context);

	sal_override void newPass()
	{
	}

	typedef ptr_vector<ILoader> Loaders;
	Loaders mLoaders;

	std::auto_ptr<Callback> mBindTextureUniformCallback;
};	// PassLoader

class StandardLoader : public EffectLoader::ILoader
{
public:
	sal_override const char* name() const {
		return "standard";
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
	{
		ColorRGBAf ambient(0, 0);
		if(!parseColor4f(parser.attributeValueIgnoreCase("ambient"), ambient)) return false;
		ColorRGBAf diffuse(0, 0);
		if(!parseColor4f(parser.attributeValueIgnoreCase("diffuse"), diffuse)) return false;
		ColorRGBAf specular(0, 0);
		if(!parseColor4f(parser.attributeValueIgnoreCase("specular"), specular)) return false;

		float shininess = parser.attributeValueAsFloatIgnoreCase("shininess", 0.0f);

		// TODO: Remove the hard coded ColorOperation
		material.addProperty(
			new StandardProperty(
				ambient, diffuse, specular, ColorProperty::ColorOperation::Replace, shininess
			), context.pass
		);

		return true;
	}

	sal_override void newPass()
	{
	}
};	// StandardLoader

class TextureLoader : public EffectLoader::ILoader
{
public:
	TextureLoader(PassLoader& passLoader) : mPassLoader(passLoader), mTextureUnit(0) {}

	sal_override const char* name() const {
		return "texture";
	}

	static void parseFilterOptions(const char* options, int& minFilter, int& magFilter)
	{
		if(0 == strCaseCmp(options, "point_point_none"))
		{
			minFilter = GL_NEAREST; magFilter = GL_NEAREST;
		}
		else if(0 == strCaseCmp(options, "point_point_point"))
		{
			minFilter = GL_NEAREST_MIPMAP_NEAREST; magFilter = GL_NEAREST;
		}
		else if(0 == strCaseCmp(options, "point_point_linear"))
		{
			minFilter = GL_NEAREST_MIPMAP_LINEAR; magFilter = GL_NEAREST;
		}
		else if(0 == strCaseCmp(options, "linear_linear_none"))
		{
			minFilter = GL_LINEAR; magFilter = GL_LINEAR;
		}
		else if(0 == strCaseCmp(options, "linear_linear_point"))
		{
			minFilter = GL_LINEAR_MIPMAP_NEAREST; magFilter = GL_LINEAR;
		}
		else if(0 == strCaseCmp(options, "linear_linear_linear"))
		{
			minFilter = GL_LINEAR_MIPMAP_LINEAR; magFilter = GL_LINEAR;
		}
		else
		{
			Log::format(Log::Warn, "EffectLoader: unknown filtering options %s.", options);
		}
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
	{
		// file attribute
		const char* file = parser.attributeValueIgnoreCase("file");
		if(!file || file[0] == '\0')
			return false;

		// Adjust the path to load the texture relative to the xml or relative to the root folder.
		Path path(file);
		path = path.hasRootDirectory() ? path : context.basePath / path;
		TexturePtr texture = dynamic_cast<Texture*>(context.resourceManager.load(path).get());

		// filter attribute
		int minFilter = GL_LINEAR;
		int magFilter = GL_LINEAR;
		const char* filter = parser.attributeValueIgnoreCase("filter");
		if(filter)
			parseFilterOptions(filter, minFilter, magFilter);

		std::auto_ptr<TextureProperty> textureProperty(new TextureProperty(texture.get(), mTextureUnit, minFilter, magFilter));

		// shaderName attribute
		if(const char* shaderName = parser.attributeValueIgnoreCase("shaderName"))
			textureProperty->shaderName = shaderName;

		material.addProperty(textureProperty.get(), context.pass);
		mPassLoader.mBindTextureUniformCallback->textures.push_back(dynamic_cast<TextureProperty*>(textureProperty->clone()));
		textureProperty.release();
		++mTextureUnit;

		return true;
	}

	sal_override void newPass()
	{
		mTextureUnit = 0;
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

		void initAsInt(const char* name, const char* value) {
			mName = name;
			mFloatValues = nullptr;
			mIntValues = strToIntArray(value, mCount);
		}

		void initAsFloat(const char* name, const char* value) {
			mName = name;
			mIntValues = nullptr;
			mFloatValues = strToFloatArray(value, mCount);
		}

	protected:
		Param() : mIntValues(nullptr), mFloatValues(nullptr), mCount(0) {}

		int getUniformLocation(uint shaderProgramHandle) const {
			int location = glGetUniformLocation(shaderProgramHandle, mName.c_str());
			if(location < 0)
				Log::format(Log::Error, "Fail to bind uniform '%s'", mName.c_str());
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
		bool createInlineShader(const char* sourceCode, uint shaderType)
		{
			if(!sourceCode)
				return false;

			InlineSource source = { sourceCode, shaderType };
			inlineSources.push_back(source);
			return true;
		}

		sal_override void doCallback()
		{
			if(!program->handle)
				program->create();

			program->detachAll();

			// Compile and attach all inline shaders
			MCD_FOREACH(const InlineSource& source, inlineSources) {
				ShaderPtr shader = new Shader("");	// This resource is not going to put in the manager, no name is needed
				shader->create(source.type);
				if(shader->compile(source.code.c_str()))
					program->attach(*shader);
				else {
					std::string log;
					shader->getLog(log);
					Log::format(Log::Error, "%s", log.c_str());
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
				Log::write(Log::Error, log.c_str());
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

		static Param* create(const char* name, const char* value) {
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

		static Param* create(const char* name, const char* value) {
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
				Log::format(Log::Warn, "Number of float in shader parameter '%s' do not match the type vec2", mName.c_str());
			if(location >= 0 && mCount > 0)
				glUniform2fv(location, mCount/2, mFloatValues);
		}

		static Param* create(const char* name, const char* value) {
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
				Log::format(Log::Warn, "Number of float in shader parameter '%s' do not match the type vec3", mName.c_str());
			if(location >= 0 && mCount > 0)
				glUniform3fv(location, mCount/3, mFloatValues);
		}

		static Param* create(const char* name, const char* value) {
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
				Log::format(Log::Warn, "Number of float in shader parameter '%s' do not match the type vec4", mName.c_str());
			if(location >= 0 && mCount > 0)
				glUniform4fv(location, mCount/4, mFloatValues);
		}

		static Param* create(const char* name, const char* value) {
			std::auto_ptr<Param> param(new Vec4Param);
			param->initAsFloat(name, value);
			return param.release();
		}
	};	// Vec4Param

	// Creates different kinds of shader param according to it's type
	void createParam(const char* name, const char* type, const char* value, Callback& callback)
	{
		if(!name || !type || !value || value[0] == '\0')
			return;

		if(strCaseCmp(type, "int") == 0)
			return callback.shaderParams.push_back(IntParam::create(name, value));
		else if(strCaseCmp(type, "float") == 0)
			return callback.shaderParams.push_back(FloatParam::create(name, value));
		else if(strCaseCmp(type, "vec2") == 0)
			return callback.shaderParams.push_back(Vec2Param::create(name, value));
		else if(strCaseCmp(type, "vec3") == 0)
			return callback.shaderParams.push_back(Vec3Param::create(name, value));
	}

public:
	ShaderLoader(PassLoader& passLoader) : mPassLoader(passLoader) {}

	sal_override const char* name() const {
		return "shader";
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
	{
		if(parser.isEmptyElement())
			return true;

		typedef XmlParser::Event Event;
		const char* shaderFile = nullptr;

		std::auto_ptr<ShaderProperty> shaderProperty(new ShaderProperty(new ShaderProgram));
		std::auto_ptr<Callback> callback(new Callback);
		mPassLoader.mBindTextureUniformCallback->shaderProgram = callback->program = shaderProperty->shaderProgram;

		// Indicate the currect shader type
		uint shaderType = 0;

		bool done = false;
		while(!done) switch(parser.nextEvent())
		{
		case Event::BeginElement:
			if(strCaseCmp(parser.elementName(), "vertex") == 0)
				shaderType = GL_VERTEX_SHADER;
			else if(strCaseCmp(parser.elementName(), "fragment") == 0)
				shaderType = GL_FRAGMENT_SHADER;

			if(shaderType == 0)
				return false;

			if((shaderFile = parser.attributeValueIgnoreCase("file")) != nullptr) {
				// Adjust the path to load the shader relative to the xml or relative to the root folder.
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
			if(strCaseCmp(parser.elementName(), "parameter") == 0)
				createParam(
					parser.attributeValueIgnoreCase("name"),
					parser.attributeValueIgnoreCase("type"),
					parser.textData(),
					*callback
				);
			break;

		case Event::CData:
			if(!callback->createInlineShader(parser.textData(), shaderType))
				return false;
			break;

		case Event::EndElement:
			if(strCaseCmp(parser.elementName(), "shader") == 0)
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

	sal_override void newPass()
	{
	}

	PassLoader& mPassLoader;
};	// ShaderLoader

class BlendingLoader : public EffectLoader::ILoader
{
public:
	sal_override const char* name() const {
		return "blending";
	}

	static void parseGLBlendFactor(const char* str, int defaultVal, int& result)
	{
		result = defaultVal;

		if(NULL != str)
		{
			if(0 == strCaseCmp(str, "zero"))
				result = GL_ZERO;

			else if(0 == strCaseCmp(str, "one"))
				result = GL_ONE;

			else if(0 == strCaseCmp(str, "srcColor"))
				result = GL_SRC_COLOR;

			else if(0 == strCaseCmp(str, "oneMinusSrcColor"))
				result = GL_ONE_MINUS_SRC_COLOR;

			else if(0 == strCaseCmp(str, "dstColor"))
				result = GL_DST_COLOR;

			else if(0 == strCaseCmp(str, "oneMinusDstColor"))
				result = GL_ONE_MINUS_DST_COLOR;

			else if(0 == strCaseCmp(str, "srcAlpha"))
				result = GL_SRC_ALPHA;

			else if(0 == strCaseCmp(str, "oneMinusSrcAlpha"))
				result = GL_ONE_MINUS_SRC_ALPHA;

			else if(0 == strCaseCmp(str, "dstAlpha"))
				result = GL_DST_ALPHA;

			else if(0 == strCaseCmp(str, "oneMinusDstAlpha"))
				result = GL_ONE_MINUS_DST_ALPHA;

			else if(0 == strCaseCmp(str, "constantColor"))
				result = GL_CONSTANT_COLOR;

			else if(0 == strCaseCmp(str, "oneMinusConstantColor"))
				result = GL_ONE_MINUS_CONSTANT_COLOR;

			else if(0 == strCaseCmp(str, "constantAlpha"))
				result = GL_CONSTANT_ALPHA;

			else if(0 == strCaseCmp(str, "oneMinusConstantAlpha"))
				result = GL_ONE_MINUS_CONSTANT_ALPHA;
		}
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
	{
		BlendingProperty* prop = new BlendingProperty();

		// color blending
		prop->blendEnable = parser.attributeValueAsBoolIgnoreCase("colorBlend", false);
		parseGLBlendFactor(parser.attributeValueIgnoreCase("srcFactor"), GL_ONE, prop->sfactor);
		parseGLBlendFactor(parser.attributeValueIgnoreCase("dstFactor"), GL_ZERO, prop->dfactor);

		// separate color blending
		prop->blendEnableSep = parser.attributeValueAsBoolIgnoreCase("colorBlendSep", false);
		parseGLBlendFactor(parser.attributeValueIgnoreCase("srcFactorSep"), GL_ONE, prop->sfactorSep);
		parseGLBlendFactor(parser.attributeValueIgnoreCase("dstFactorSep"), GL_ZERO, prop->dfactorSep);

		// blendcolor
		parseColor4f(parser.attributeValueIgnoreCase("blendConstant"), prop->blendColor);

		material.addProperty(prop, context.pass);

		return true;
	}

	sal_override void newPass()
	{
	}

};	// BlendingLoader

class DepthStencilLoader : public EffectLoader::ILoader
{
public:
	sal_override const char* name() const {
		return "depthStencil";
	}

	static void parseGLCmpFunc(const char* str, int defaultVal, int& result)
	{
		result = defaultVal;

		if(NULL != str)
		{
			if(0 == strCaseCmp(str, "never"))
				result = GL_NEVER;

			else if(0 == strCaseCmp(str, "less"))
				result = GL_LESS;

			else if(0 == strCaseCmp(str, "equal"))
				result = GL_EQUAL;

			else if(0 == strCaseCmp(str, "lessEqual"))
				result = GL_LEQUAL;

			else if(0 == strCaseCmp(str, "greater"))
				result = GL_GREATER;

			else if(0 == strCaseCmp(str, "notEqual"))
				result = GL_NOTEQUAL;

			else if(0 == strCaseCmp(str, "greaterEqual"))
				result = GL_GEQUAL;

			else if(0 == strCaseCmp(str, "always"))
				result = GL_ALWAYS;
		}
	}

	sal_override bool load(XmlParser& parser, IMaterial& material, Context& context)
	{
		DepthStencilProperty* prop = new DepthStencilProperty();

		// depth test
		prop->depthTestEnable = parser.attributeValueAsBoolIgnoreCase("depthTest", true);
		prop->depthWriteEnable = parser.attributeValueAsBoolIgnoreCase("depthWrite", true);
		parseGLCmpFunc(parser.attributeValueIgnoreCase("depthFunc"), GL_LESS, prop->depthFunc);

		// stencil test

		material.addProperty(prop, context.pass);

		return true;
	}

	sal_override void newPass()
	{
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

	for(Loaders::iterator i=mLoaders.begin(); i!=mLoaders.end(); ++i) {
		i->newPass();
	}

	// If the pass is disabled, skip the whole element.
	if(!parser.attributeValueAsBoolIgnoreCase("enable", true/*By default a pass is enabled*/))
	{
		if(parser.isEmptyElement())
			return true;
		while(true) switch(parser.nextEvent()) {
		case Event::EndElement:
			if(strCaseCmp(parser.elementName(), "pass") == 0)
				return true;
			break;

		case Event::Error:
		case Event::EndDocument:
			return false;

		default:
			break;
		}
	}

	const char* attributeValue = nullptr;
	// Parse the attributes of "pass"
	if(parser.attributeValueAsBoolIgnoreCase("drawLine", false))
		material.addProperty(new LineDrawingProperty, context.pass);

	if(parser.attributeValueIgnoreCase("lineWidth"))
		material.addProperty(new LineWidthProperty(parser.attributeValueAsFloatIgnoreCase("lineWidth", 1.0f)), context.pass);

	if((attributeValue = parser.attributeValueIgnoreCase("cullMode")) != nullptr) {
		if(strCaseCmp(attributeValue, "none") == 0)
			material.addProperty(new DisableStateProperty(GL_CULL_FACE), context.pass);
		else if(strCaseCmp(attributeValue, "front") == 0)
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
			if(strCaseCmp(parser.elementName(), i->name()) != 0)
				continue;
			if(!i->load(parser, material, context))
				return false;
			break;
		}
		break;

	case Event::EndElement:
		if(strCaseCmp(parser.elementName(), "pass") == 0) {
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

		std::string xmlString;
		{	// A simple way to grab all the content from the stream
			std::stringstream ss;
			*is >> ss.rdbuf();
			xmlString = ss.str();
			mLoadingState = xmlString.empty() ? Aborted : mLoadingState;
		}

		if(mLoadingState & Stopped)
			return mLoadingState;

		ILoader::Context context = { 0, *fileId, fileId->getBranchPath(), nullptr, mResourceManager };

		// Parse the xml
		typedef XmlParser::Event Event;
		XmlParser parser;
		parser.parse(const_cast<char*>(xmlString.c_str()));	// NOTE: parser.parse() will modify the string content in the stream
		mLoadingState = Loaded;

		while(true) switch(parser.nextEvent())
		{
		case Event::BeginElement:
			// Search for a loader that will response with this xml element
			for(Loaders::iterator i=mLoaders.begin(); i!=mLoaders.end(); ++i) {
				if(strCaseCmp(parser.elementName(), i->name()) != 0)
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
		effect.material.reset(static_cast<Material*>(mMaterial.clone()));
	}

	IResourceLoader::LoadingState mLoadingState;

	typedef ptr_vector<ILoader> Loaders;
	Loaders mLoaders;

	Material mMaterial;

	IResourceManager& mResourceManager;
};	// Impl

EffectLoader::EffectLoader(IResourceManager& resourceManager)
	: mImpl(*new Impl(resourceManager))
{
}

EffectLoader::~EffectLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState EffectLoader::load(std::istream* is, const Path* fileId, const char*)
{
	return mImpl.load(is, fileId);
}

void EffectLoader::commit(Resource& resource)
{
	mImpl.commit(resource);
}

IResourceLoader::LoadingState EffectLoader::getLoadingState() const
{
	return mImpl.mLoadingState;
}

}	// namespace MCD
