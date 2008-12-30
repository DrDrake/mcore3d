#include "Pch.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "../../3Party/glew/glew.h"
#include <algorithm>

namespace MCD {

ShaderProgram::ShaderProgram()
	: handle(0)
{}

ShaderProgram::~ShaderProgram()
{
	destroy();
}

void ShaderProgram::create()
{
	destroy();
	handle = glCreateProgram();
}

void ShaderProgram::destroy()
{
	if(!handle)
		return;

	detachAll();

	glDeleteProgram(handle);
	handle = 0;
}

void ShaderProgram::attach(Shader& shader)
{
	detach(shader);
	glAttachShader(handle, shader.handle());
	mShaders.push_back(&shader);
}

void ShaderProgram::detach(Shader& shader)
{
	Shaders::const_iterator i = std::find(mShaders.begin(), mShaders.end(), &shader);
	if(i != mShaders.end())
		glDetachShader(handle, shader.handle());
}

void ShaderProgram::detachAll()
{
	for(Shaders::const_iterator i=mShaders.begin(); i!=mShaders.end(); ++i)
		glDetachShader(handle, (*i)->handle());

	mShaders.clear();
}

bool ShaderProgram::link()
{
	glLinkProgram(handle);

	GLint result = 0;
	glGetProgramiv(handle, GL_LINK_STATUS, &result);
//	createUniformMap();
//	createAttributeMap();

	return (result == GL_TRUE);
}

void ShaderProgram::bind()
{
	glUseProgram(handle);
}

void ShaderProgram::unbind()
{
	glUseProgram(0);
}

void ShaderProgram::getLog(std::string& log)
{
	GLint len;
	glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &len);
	if(len <= 0)
		return;
	log.resize(len);
	glGetProgramInfoLog(handle, len, nullptr, &log[0]);
}

}	// namespace MCD
