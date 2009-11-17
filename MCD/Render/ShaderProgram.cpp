#include "Pch.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "../Core/System/Log.h"
#include "../../3Party/glew/glew.h"
#include <algorithm>
#include <map>
#include <string.h>	// For strcmp()
#include <vector>

namespace MCD {

class ShaderProgram::Impl
{
public:
	struct CStrKey
	{
		const char* key;
		CStrKey(const char* key_) : key(key_) {}
		
		bool operator < (const CStrKey& rhs) const { return strcmp(key, rhs.key) < 0; }
		bool operator == (const CStrKey& rhs) const { return strcmp(key, rhs.key) == 0; }
	};

	int getUniform(int program, const char* name)
	{
		CStrKey key(name);
		Uniforms::iterator i = mUniforms.find(key);

		if(i != mUniforms.end())
			return i->second;

		int loc = glGetUniformLocation(program, name);

		if(loc != -1)
			mUniforms.insert(std::make_pair(key, loc));
		else
		{
			// TODO: Log more information, becare of the char* wchar_t* issues
			Log::format(Log::Error, L"Shader uniform not found");
		}
		
		return loc;
	}

	typedef std::vector<ShaderPtr> Shaders;
	typedef std::map<CStrKey, int> Uniforms;

	Uniforms mUniforms;
	Shaders mShaders;
};	// Impl

ShaderProgram::ShaderProgram()
	: handle(0)
	, mImpl(*new Impl)
{}

ShaderProgram::~ShaderProgram()
{
	destroy();
	delete &mImpl;
}

ShaderProgram* ShaderProgram::mCurrent = nullptr;

ShaderProgram* ShaderProgram::current()
{
	return mCurrent;
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
	mImpl.mShaders.push_back(&shader);
}

void ShaderProgram::detach(Shader& shader)
{
	Impl::Shaders::const_iterator i = std::find(mImpl.mShaders.begin(), mImpl.mShaders.end(), &shader);
	if(i != mImpl.mShaders.end())
		glDetachShader(handle, shader.handle());
}

void ShaderProgram::detachAll()
{
	for(Impl::Shaders::const_iterator i=mImpl.mShaders.begin(); i!=mImpl.mShaders.end(); ++i)
		glDetachShader(handle, (*i)->handle());

	mImpl.mShaders.clear();
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
	mCurrent = this;
}

void ShaderProgram::unbind()
{
	glUseProgram(0);
	mCurrent = nullptr;
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
	glUniform1f(mImpl.getUniform(handle, name), v0);
}

void ShaderProgram::uniform2f(const char* name, float v0, float v1)
{
	glUniform2f(mImpl.getUniform(handle, name), v0, v1);
}

void ShaderProgram::uniform3f(const char* name, float v0, float v1, float v2)
{
	glUniform3f(mImpl.getUniform(handle, name), v0, v1, v2);
}

void ShaderProgram::uniform4f(const char* name, float v0, float v1, float v2, float v3)
{
	glUniform4f(mImpl.getUniform(handle, name), v0, v1, v2, v3);
}

void ShaderProgram::uniform1fv(const char* name, size_t count, const float* value)
{
	glUniform1fv(mImpl.getUniform(handle, name), count, value);
}

void ShaderProgram::uniform2fv(const char* name, size_t count, const float* value)
{
	glUniform2fv(mImpl.getUniform(handle, name), count, value);
}

void ShaderProgram::uniform3fv(const char* name, size_t count, const float* value)
{
	glUniform3fv(mImpl.getUniform(handle, name), count, value);
}

void ShaderProgram::uniform4fv(const char* name, size_t count, const float* value)
{
	glUniform4fv(mImpl.getUniform(handle, name), count, value);
}

void ShaderProgram::uniform1i(const char* name, int v0)
{
	glUniform1i(mImpl.getUniform(handle, name), v0);
}

void ShaderProgram::uniform2i(const char* name, int v0, int v1)
{
	glUniform2i(mImpl.getUniform(handle, name), v0, v1);
}

void ShaderProgram::uniform3i(const char* name, int v0, int v1, int v2)
{
	glUniform3i(mImpl.getUniform(handle, name), v0, v1, v2);
}

void ShaderProgram::uniform4i(const char* name, int v0, int v1, int v2, int v3)
{
	glUniform4i(mImpl.getUniform(handle, name), v0, v1, v2, v3);
}

void ShaderProgram::uniform1iv(const char* name, size_t count, const int* value)
{
	glUniform1iv(mImpl.getUniform(handle, name), count, value);
}

void ShaderProgram::uniform2iv(const char* name, size_t count, const int* value)
{
	glUniform2iv(mImpl.getUniform(handle, name), count, value);
}

void ShaderProgram::uniform3iv(const char* name, size_t count, const int* value)
{
	glUniform3iv(mImpl.getUniform(handle, name), count, value);
}

void ShaderProgram::uniform4iv(const char* name, size_t count, const int* value)
{
	glUniform4iv(mImpl.getUniform(handle, name), count, value);
}

void ShaderProgram::uniformMatrix2fv(const char* name, size_t count, bool transpose, const float* value)
{
	glUniformMatrix2fv(mImpl.getUniform(handle, name), count, transpose, value);
}

void ShaderProgram::uniformMatrix3fv(const char* name, size_t count, bool transpose, const float* value)
{
	glUniformMatrix3fv(mImpl.getUniform(handle, name), count, transpose, value);
}

void ShaderProgram::uniformMatrix4fv(const char* name, size_t count, bool transpose, const float* value)
{
	glUniformMatrix4fv(mImpl.getUniform(handle, name), count, transpose, value);
}

}	// namespace MCD
