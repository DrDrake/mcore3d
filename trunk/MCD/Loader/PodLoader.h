#ifndef __MCD_LOADER_PODLOADER__
#define __MCD_LOADER_PODLOADER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

class IResourceManager;

/*!	Loader for PowerVR's .pod file format.
	\sa http://www.imgtec.com/powervr/insider/powervr-pvrgeopod.asp

	\note The max pod exporter plugin didn't handle multi-material, therefore it is
		best to split the mesh with multi-material into multiple mesh first.
		There is a max script (for Max 9) to do that:
		http://www.scriptspot.com/3ds-max/scripts/detach-by-material-id
 */
class MCD_LOADER_API PodLoader : public IResourceLoader, private Noncopyable
{
public:
	PodLoader(sal_maybenull IResourceManager* resourceManager = nullptr);

	sal_override ~PodLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type Prefab.
	 */
	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	class Impl;
	Impl& mImpl;
};	// PodLoader

/*!	Load a model as a tree of Entity.
 */
class MCD_LOADER_API PodLoaderFactory : public ResourceManager::IFactory
{
public:
    PodLoaderFactory(IResourceManager& resourceManager);
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoader* createLoader();

private:
	/*!	This loader factory is going to be owned by the mResourceManager, so we can
		use mResourceManager freely during the life-time of the loader factory.
	 */
	IResourceManager& mResourceManager;
};	// PodLoaderFactory

}	// namespace MCD

#endif	// __MCD_LOADER_PODLOADER__
