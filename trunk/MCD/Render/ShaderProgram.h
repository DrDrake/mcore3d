#ifndef __MCD_RENDER_SHADERPROGRAM__
#define __MCD_RENDER_SHADERPROGRAM__

#include "ShareLib.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/NonCopyable.h"

namespace MCD {

typedef IntrusivePtr<class Shader> ShaderPtr;

class MCD_RENDER_API ShaderProgram : Noncopyable
{
public:
	ShaderProgram();
	~ShaderProgram();

// Operations
	void create();

	void destroy();

	/*!	Attach a shader to the program.
		Multiple shader can be attached to the program and once all shaders are
		attached, you can call link().
	 */
	void attach(Shader& shader);

	void detach(Shader& shader);

	void detachAll();

	sal_checkreturn bool link();

	void bind();

	void unbind();

// Never use temporary c-string for the following methods!!
	/*!	Helper functions for binding shader uniforms.
		\sa http://www.opengl.org/sdk/docs/man/xhtml/glUniform.xml
	 */
	void uniform1f(sal_in_z const char* name, float v0);
	void uniform2f(sal_in_z const char* name, float v0, float v1);
	void uniform3f(sal_in_z const char* name, float v0, float v1, float v2);
	void uniform4f(sal_in_z const char* name, float v0, float v1, float v2, float v3);

	void uniform1fv(sal_in_z const char* name, size_t count, sal_in_ecount(count * 1) const float* value);
	void uniform2fv(sal_in_z const char* name, size_t count, sal_in_ecount(count * 2) const float* value);
	void uniform3fv(sal_in_z const char* name, size_t count, sal_in_ecount(count * 3) const float* value);
	void uniform4fv(sal_in_z const char* name, size_t count, sal_in_ecount(count * 4) const float* value);
	
	void uniform1i(sal_in_z const char* name, int v0);
	void uniform2i(sal_in_z const char* name, int v0, int v1);
	void uniform3i(sal_in_z const char* name, int v0, int v1, int v2);
	void uniform4i(sal_in_z const char* name, int v0, int v1, int v2, int v3);

	void uniform1iv(sal_in_z const char* name, size_t count, sal_in_ecount(count * 1) const int* value);
	void uniform2iv(sal_in_z const char* name, size_t count, sal_in_ecount(count * 2) const int* value);
	void uniform3iv(sal_in_z const char* name, size_t count, sal_in_ecount(count * 3) const int* value);
	void uniform4iv(sal_in_z const char* name, size_t count, sal_in_ecount(count * 4) const int* value);

	void uniformMatrix2fv(sal_in_z const char* name, size_t count, bool transpose, sal_in_ecount(count * 2*2) const float* value);
	void uniformMatrix3fv(sal_in_z const char* name, size_t count, bool transpose, sal_in_ecount(count * 3*3) const float* value);
	void uniformMatrix4fv(sal_in_z const char* name, size_t count, bool transpose, sal_in_ecount(count * 4*4) const float* value);

// Attributes
	// Get the information about the program (including any linking errors).
	void getLog(std::string& log);

	uint handle;

protected:
	class Impl;
	Impl* mImpl;
};	// ShaderProgram

}	// namespace MCD

#endif	// __MCD_RENDER_SHADERPROGRAM__
