#ifndef __MCD_RENDER_SHADER__
#define __MCD_RENDER_SHADER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/Resource.h"

namespace MCD {

class MCD_RENDER_API Shader : public Resource, Noncopyable
{
public:
	explicit Shader(const Path& fileId);

	/*!	Create the shader.
		\param shaderType Type of the shader, can be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
		\note Previous created shader (if any) will be destroyed first.
	 */
	void create(uint shaderType);

	void destroy();

	//! Compile the source code.
	sal_checkreturn bool compile(sal_in_z const char* source);

	//! Compile from a list of source code.
	sal_checkreturn bool compile(const char** sourceList, size_t count);

	/*! Get the source code of this shader.
	 */
	void getSource(std::string& source);

	//! Get the information about the shader (including any compilation errors).
	void getLog(std::string& log);

	uint handle() const {
		return mHandle;
	}

	uint type() const {
		return mType;
	}

protected:
	sal_override ~Shader();

protected:
	uint mHandle;
	uint mType;
};	// Shader

typedef IntrusivePtr<Shader> ShaderPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_SHADER__
