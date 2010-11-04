#ifndef __MCD_LOADER_MAX3DSLOADER__
#define __MCD_LOADER_MAX3DSLOADER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

/*!	Loader for the Autodesk 3D studio's 3ds file format.
	The loader will load vertex attributes including position and 1 texture coordinate,
	vertex normal is generated afterwards according the index information plus any smoothing
	group data presented in the 3ds. Material attributes including ambient, diffuse, specular
	and texture map are supported. Multi sub-object is also supported so that a single
	vertex buffer may be shared by multiple index buffers, where the index buffers are split-up
	according to the different materials presented in the same vertex buffer.

	Loading args:
	- Name: tangents
		- Desc: Compute tangent space for loaded meshes.
		- Value: true/false
		- Default: false

	\sa http://www.flipcode.com/archives/Another_3DS_LoaderViewer_Class.shtml
	\sa http://www.gamedev.net/community/forums/topic.asp?topic_id=382606
	\sa http://gpwiki.org/index.php/C:Load3DS
 */
class MCD_LOADER_API Max3dsLoader : public IResourceLoader
{
public:
	Max3dsLoader();

	sal_override ~Max3dsLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type Model.
	 */
	sal_override void commit(Resource& resource);

protected:
	class Impl;
	Impl& mImpl;
};	// Max3dsLoader

class MCD_LOADER_API Max3dsLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoaderPtr createLoader();
};	// Max3dsLoaderFactory

}	// namespace MCD

#endif	// __MCD_LOADER_MAX3DSLOADER__
