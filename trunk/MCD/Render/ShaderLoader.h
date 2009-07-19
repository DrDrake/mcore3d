#ifndef __MCD_RENDER_SHADERLOADER__
#define __MCD_RENDER_SHADERLOADER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/ResourceLoader.h"
#include <string>

namespace MCD {

//! Shader loader
class MCD_RENDER_API ShaderLoader : public IResourceLoader, private Noncopyable
{
public:
	explicit ShaderLoader(int shaderType);

	/*!	Load data from stream.
		Block until the whole shader source file is loaded.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const wchar_t* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type Shader.
	 */
	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	int mShaderType;	//!< Can be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
	std::string mSourceCode;
	LoadingState mLoadingState;
};	// ShaderLoader

}	// namespace MCD

#endif	// __MCD_RENDER_SHADERLOADER__
