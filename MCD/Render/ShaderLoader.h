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
	/*!	Load data from stream.
		Block until the whole shader source file is loaded.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type Shader.
	 */
	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	std::string mSourceCode;
	LoadingState mLoadingState;
};	// ShaderLoader

}	// namespace MCD

#endif	// __MCD_RENDER_SHADERLOADER__
