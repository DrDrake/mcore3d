#ifndef __MCD_RENDER_SHADERPROGRAM__
#define __MCD_RENDER_SHADERPROGRAM__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"

namespace MCD {

class MCD_RENDER_API ShaderProgram : Noncopyable
{
public:
	ShaderProgram();
	~ShaderProgram();

	void create();

	void destroy();

	/*!	Attach a shader to the program.
		Multiple shader can be attached to the program and once all shaders are
		attached, you can call link().
	 */
	void attach(uint shaderHandle);

	void detach(uint shaderHandle);

	void detachAll();

	sal_checkreturn bool link();

	void bind();

	void unbind();

	// Get the information about the program (including any linking errors).
	void getLog(std::string& log);

	uint handle() const {
		return mHandle;
	}

protected:
	uint mHandle;
};	// ShaderProgram

}	// namespace MCD

#endif	// __MCD_RENDER_SHADERPROGRAM__
