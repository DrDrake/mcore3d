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
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const wchar_t* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource can be of type Mesh or EntityPrototype.
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
	sal_override ResourcePtr createResource(const Path& fileId, const wchar_t* args);
	sal_override IResourceLoader* createLoader();

private:
	/*!	This loader factory is going to be owned by the mResourceManager, so we can
		use mResourceManager freely during the life-time of the loader factory.
	 */
	IResourceManager& mResourceManager;
};	// PodLoaderFactory

}	// namespace MCD

#endif	// __MCD_LOADER_PODLOADER__
