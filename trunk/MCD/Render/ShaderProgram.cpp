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

void ShaderProgram::uniform1f(const char* name, float v0)
{
}

void ShaderProgram::uniform2f(const char* name, float v0, float v1)
{
}

void ShaderProgram::uniform3f(const char* name, float v0, float v1, float v2)
{
}

void ShaderProgram::uniform4f(const char* name, float v0, float v1, float v2, float v3)
{
}

void ShaderProgram::uniform1fv(const char* name, size_t count, const float* value)
{
}

void ShaderProgram::uniform2fv(const char* name, size_t count, const float* value)
{
}

void ShaderProgram::uniform3fv(const char* name, size_t count, const float* value)
{
}

void ShaderProgram::uniform4fv(const char* name, size_t count, const float* value)
{
}

void ShaderProgram::uniform1i(const char* name, int v0)
{
}

void ShaderProgram::uniform2i(const char* name, int v0, int v1)
{
}

void ShaderProgram::uniform3i(const char* name, int v0, int v1, int v2)
{
}

void ShaderProgram::uniform4i(const char* name, int v0, int v1, int v2, int v3)
{
}

void ShaderProgram::uniform1iv(const char* name, size_t count, const int* value)
{
}

void ShaderProgram::uniform2iv(const char* name, size_t count, const int* value)
{
}

void ShaderProgram::uniform3iv(const char* name, size_t count, const int* value)
{
}

void ShaderProgram::uniform4iv(const char* name, size_t count, const int* value)
{
}

void ShaderProgram::uniformMatrix2fv(const char* name, size_t count, const float* value)
{
}

void ShaderProgram::uniformMatrix3fv(const char* name, size_t count, const float* value)
{
}

void ShaderProgram::uniformMatrix4fv(const char* name, size_t count, const float* value)
{
}

}	// namespace MCD
