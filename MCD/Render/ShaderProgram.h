#ifndef __MCD_RENDER_SHADERPROGRAM__
#define __MCD_RENDER_SHADERPROGRAM__

#include "ShareLib.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/NonCopyable.h"
#include <vector>

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

    void uniform1f(const char* name, float v0);
    void uniform2f(const char* name, float v0, float v1);
    void uniform3f(const char* name, float v0, float v1, float v2);
    void uniform4f(const char* name, float v0, float v1, float v2, float v3);

    void uniform1fv(const char* name, size_t count, const float* value);
    void uniform2fv(const char* name, size_t count, const float* value);
    void uniform3fv(const char* name, size_t count, const float* value);
    void uniform4fv(const char* name, size_t count, const float* value);
    
    void uniform1i(const char* name, int v0);
    void uniform2i(const char* name, int v0, int v1);
    void uniform3i(const char* name, int v0, int v1, int v2);
    void uniform4i(const char* name, int v0, int v1, int v2, int v3);

    void uniform1iv(const char* name, size_t count, const int* value);
    void uniform2iv(const char* name, size_t count, const int* value);
    void uniform3iv(const char* name, size_t count, const int* value);
    void uniform4iv(const char* name, size_t count, const int* value);

    void uniformMatrix2fv(const char* name, size_t count, const float* value);
    void uniformMatrix3fv(const char* name, size_t count, const float* value);
    void uniformMatrix4fv(const char* name, size_t count, const float* value);

// Attributes
	// Get the information about the program (including any linking errors).
	void getLog(std::string& log);

	uint handle;

protected:
	typedef std::vector<ShaderPtr> Shaders;
	Shaders mShaders;
};	// ShaderProgram

}	// namespace MCD

#endif	// __MCD_RENDER_SHADERPROGRAM__
