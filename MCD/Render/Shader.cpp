#include "Pch.h"
#include "Shader.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

Shader::Shader()
	: mHandle(0), mType(0)
{}

Shader::~Shader() {
	destroy();
}

void Shader::create(uint shaderType)
{
	destroy();
	mHandle = glCreateShader(shaderType);
}

void Shader::destroy()
{
	if(!mHandle)
		return;

	glDeleteShader(mHandle);
	mHandle = 0;
	mType = 0;
}

bool Shader::compile(sal_in_z const char* source)
{
	const char* strList[1];
	strList[0] = source;
	return compile(strList, 1);
}

bool Shader::compile(const char** sourceList, size_t count)
{
	MCD_ASSUME(mHandle != 0);
	if(!mHandle)
		return false;

	// We use null terminated strings, therefore the fourth parameter is null
	glShaderSource(mHandle, 1, sourceList, nullptr);

	GLint ok;
	glCompileShader(mHandle);
	glGetShaderiv(mHandle, GL_COMPILE_STATUS, &ok);
	return (ok == GL_TRUE);
}

void Shader::getSource(std::string& source)
{
	// Get the length of the source
	GLint len = 0;
	glGetShaderiv(mHandle, GL_SHADER_SOURCE_LENGTH, &len);
	source.resize(len);
	GLsizei written = 0;
	glGetShaderSource(mHandle, len, &written, &source[0]);
	MCD_ASSERT(written == len);
}

void Shader::getLog(std::string& log)
{
	GLint len;
	glGetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &len);
	if(len <= 0)
		return;
	log.resize(len);
	glGetShaderInfoLog(mHandle, len, nullptr, &log[0]);
}

}	// namespace MCD
