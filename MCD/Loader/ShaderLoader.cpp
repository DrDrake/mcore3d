#include "Pch.h"
#include "ShaderLoader.h"
#include "../Render/Shader.h"
#include "../Core/System/Log.h"
#include <sstream>

namespace MCD {

ShaderLoader::ShaderLoader(int shaderType)
	: mShaderType(shaderType)
{
}

IResourceLoader::LoadingState ShaderLoader::load(std::istream* is, const Path*, const char*)
{
	mLoadingState = is ? NotLoaded : Aborted;

	if(mLoadingState & Stopped)
		return mLoadingState;

	MCD_ASSUME(is);
	{	// A simple way to grab all the content from the stream
		std::stringstream ss;
		*is >> ss.rdbuf();
		mSourceCode = ss.str();
	}

	mLoadingState = mSourceCode.empty() ? Aborted : Loaded;

	return mLoadingState;
}

void ShaderLoader::commit(Resource& resource)
{
	// Will throw exception if the resource is not of the type Shader
	Shader& shader = dynamic_cast<Shader&>(resource);

	shader.create(mShaderType);

	if(!shader.compile(mSourceCode.c_str())) {
		std::string log;
		shader.getLog(log);
		Log::format(Log::Error, "%s", log.c_str());
		return;
	}

	++resource.commitCount;
}

IResourceLoader::LoadingState ShaderLoader::getLoadingState() const
{
	return mLoadingState;
}

}	// namespace MCD
