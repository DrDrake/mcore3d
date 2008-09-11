#ifndef __MCD_RENDER_MAX3DSLOADER__
#define __MCD_RENDER_MAX3DSLOADER__

#include "ShareLib.h"
#include "../Core/System/Noncopyable.h"
#include "../Core/System/ResourceLoader.h"

namespace MCD {

class ResourceManager;

/*!	Loader for the Autodesk 3D studio's 3ds file format.
	The loader will load vertex attributes including position and 1 texture coordinate,
	vertex normal is generated afterwards according the index information plus any smoothing
	group data presented in the 3ds. Material attributes including ambient, diffuse, specular
	and texture map are supported. Multi sub-object is also supported so that a single
	vertex buffer may be shared by multiple index buffers, where the index buffers are split-up
	according to the different materials presented in the same vertex buffer.

	\sa http://www.flipcode.com/archives/Another_3DS_LoaderViewer_Class.shtml
	\sa http://www.gamedev.net/community/forums/topic.asp?topic_id=382606
	\sa http://gpwiki.org/index.php/C:Load3DS
 */
class MCD_RENDER_API Max3dsLoader : public IResourceLoader, private Noncopyable
{
public:
	/*!	Constructor, take a ResourceManager as an optional parameter.
		Since the 3ds model may reference some texture files, therefore it needs
		a resource manager in order to load the textures. If null pointer is supplied
		as the manager,
	 */
	explicit Max3dsLoader(sal_maybenull ResourceManager* resourceManager = nullptr);

	sal_override ~Max3dsLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type Model.
	 */
	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	class Impl;
	Impl* mImpl;
};	// Max3dsLoader

}	// namespace MCD

#endif	// __MCD_RENDER_MAX3DSLOADER__
