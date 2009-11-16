#ifndef __MCD_RENDER_OGREMESHLOADER__
#define __MCD_RENDER_OGREMESHLOADER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/ResourceLoader.h"

namespace MCD {

class IResourceManager;

/*!	Loader for the Ogre 's .mesh file format.
 */
class MCD_RENDER_API OgreMeshLoader : public IResourceLoader, private Noncopyable
{
public:
	/*!	Constructor, take a ResourceManager as an optional parameter.
		Since the 3ds model may reference some texture files, therefore it needs
		a resource manager in order to load the textures. If null pointer is supplied
		as the manager,
	 */
	explicit OgreMeshLoader(sal_maybenull IResourceManager* resourceManager = nullptr);

	sal_override ~OgreMeshLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const wchar_t* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type Model.
	 */
	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	class Impl;
	Impl* mImpl;
};	// OgreMeshLoader

}	// namespace MCD

#endif	// __MCD_RENDER_OGREMESHLOADER__