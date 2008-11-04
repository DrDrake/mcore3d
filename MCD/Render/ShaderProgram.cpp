#include "Pch.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "../../3Party/glew/glew.h"
#include <algorithm>

namespace MCD {

ShaderProgram::ShaderProgram()
	: mHandle(0)
{}

ShaderProgram::~ShaderProgram()
{
	destroy();
}

void ShaderProgram::create()
{
	destroy();
	mHandle = glCreateProgram();
}

void ShaderProgram::destroy()
{
	if(!mHandle)
		return;

	detachAll();

	glDeleteProgram(mHandle);
	mHandle = 0;
}

void ShaderProgram::attach(Shader& shader)
{
	detach(shader);
	glAttachShader(mHandle, shader.handle());
	mShaders.push_back(&shader);
}

void ShaderProgram::detach(Shader& shader)
{
	Shaders::const_iterator i = std::find(mShaders.begin(), mShaders.end(), &shader);
	if(i != mShaders.end())
		glDetachShader(mHandle, shader.handle());
}

void ShaderProgram::detachAll()
{
	for(Shaders::const_iterator i=mShaders.begin(); i!=mShaders.end(); ++i)
		glDetachShader(mHandle, (*i)->handle());

	mShaders.clear();
}

bool ShaderProgram::link()
{
	glLinkProgram(mHandle);

	GLint result = 0;
	glGetProgramiv(mHandle, GL_LINK_STATUS, &result);
//	createUniformMap();
//	createAttributeMap();

	return (result == GL_TRUE);
}

void ShaderProgram::bind()
{
	glUseProgram(mHandle);
}

void ShaderProgram::unbind()
{
	glUseProgram(0);
}

void ShaderProgram::getLog(std::string& log)
{
	GLint len;
	glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &len);
	if(len <= 0)
		return;
	log.resize(len);
	glGetProgramInfoLog(mHandle, len, nullptr, &log[0]);
}

}	// namespace MCD
