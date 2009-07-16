#ifndef __MCD_COMPONENT_RENDER_ENTITYPROTOTYPELOADER__
#define __MCD_COMPONENT_RENDER_ENTITYPROTOTYPELOADER__

#include "../ShareLib.h"
#include "../../Core/System/NonCopyable.h"
#include "../../Core/System/ResourceLoader.h"
#include "../../Core/System/ResourceManager.h"

namespace MCD {

class IResourceManager;

class MCD_COMPONENT_API EntityPrototypeLoader : public IResourceLoader, private Noncopyable
{
public:
	/*!	Constructor, take a ResourceManager as an optional parameter.
		Since the 3ds model may reference some texture files, therefore it needs
		a resource manager in order to load the textures. If null pointer is supplied
		as the manager,
	 */
	explicit EntityPrototypeLoader(sal_maybenull IResourceManager* resourceManager = nullptr);

	sal_override ~EntityPrototypeLoader();

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

private:
	class Impl;
	Impl* mImpl;

};	// EntityPrototypeLoader

class MCD_COMPONENT_API EntityPrototypeLoaderFactory : public ResourceManager::IFactory
{
public:
    EntityPrototypeLoaderFactory(IResourceManager& resourceManager);
	sal_override ResourcePtr createResource(const Path& fileId, const wchar_t* args);
	sal_override IResourceLoader* createLoader();

private:
	/*!	This loader factory is going to be owned by the mResourceManager, so we can
		use mResourceManager freely during the life-time of the loader factory.
	 */
	IResourceManager& mResourceManager;
};	// EntityPrototypeLoaderFactory

}	// namespace MCD

#endif	// __MCD_COMPONENT_RENDER_ENTITYPROTOTYPELOADER__
