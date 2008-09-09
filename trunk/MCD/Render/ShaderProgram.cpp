#include "Pch.h"
#include "ShaderProgram.h"
#include "../../3Party/glew/glew.h"
#include <vector>

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

	glDeleteProgram(mHandle);
	mHandle = 0;
}

void ShaderProgram::attach(uint shaderHandle)
{
	glAttachShader(mHandle, shaderHandle);
}

void ShaderProgram::detach(uint shaderHandle)
{
	glDetachShader(mHandle, shaderHandle);
}

void ShaderProgram::detachAll()
{
	// Get the number of attached shaders
	GLint num = 0;
	glGetProgramiv(mHandle, GL_ATTACHED_SHADERS, &num);

	if(num <= 0)
		return;

	std::vector<GLuint> shaders(num);
	GLsizei count = 0;
	glGetAttachedShaders(mHandle, num, &count, &shaders[0]);
	for(GLsizei i = 0; i < count; ++i)
		detach(shaders[i]);
}

bool ShaderProgram::link()
{
	glLinkProgram(mHandle);

	GLint error = 0;
	glGetProgramiv(mHandle, GL_LINK_STATUS, &error);
//	createUniformMap();
//	createAttributeMap();

	return error == GL_TRUE;
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
