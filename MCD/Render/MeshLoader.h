#ifndef __MCD_RENDER_MESHLOADER__
#define __MCD_RENDER_MESHLOADER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/ResourceLoader.h"

namespace MCD {

class IResourceManager;

/*!	Loader for MCore's .msh file format.
 */
class MCD_RENDER_API MeshLoader : public IResourceLoader, private Noncopyable
{
public:
	MeshLoader();

	sal_override ~MeshLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type Model.
	 */
	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	class Impl;
	Impl& mImpl;
};	// MeshLoader

}	// namespace MCD

#endif	// __MCD_RENDER_MESHLOADER__
