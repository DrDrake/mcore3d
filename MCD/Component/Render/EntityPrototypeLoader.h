#ifndef __MCD_COMPONENT_RENDER_ENTITYPROTOTYPELOADER__
#define __MCD_COMPONENT_RENDER_ENTITYPROTOTYPELOADER__

#include "EntityPrototype.h"
#include "../../Core/System/ResourceLoader.h"
#include "../../Core/System/ResourceManager.h"

namespace MCD {

class IResourceManager;
typedef IntrusiveWeakPtr<class Entity> EntityPtr;

class MCD_COMPONENT_API EntityPrototypeLoader : public IResourceLoader, private Noncopyable
{
public:
	/*!	Constructor, take a ResourceManager as an optional parameter.
		Since the mesh file may reference some texture/material files, therefore it
		needs a resource manager in order to load them. If null pointer is supplied,
		no texture/material will be loaded.
	 */
	explicit EntityPrototypeLoader(sal_maybenull IResourceManager* resourceManager = nullptr);

	sal_override ~EntityPrototypeLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_maybenull const wchar_t* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type Model.
	 */
	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

	class LoadCallback;

	/*!	Loads a model file (currently only support *.3ds), creates a tree of entities
		and add them to a specific entity node after the load completes.

		If you know that the resource will load multiple times, you need to keep
		the returned EntityPrototypePtr. Otherwise the entity prototype will be
		destroyed once the load is completed.

		If the load operation failed, a EntityPrototype with it's \em entity equals to
		null will be returned.

		\param loadCallback Optional parameter for the user to get notification when the loads complete.
	 */
	static EntityPrototypePtr addEntityAfterLoad(
		const EntityPtr& addToHere, IResourceManager& manager,
		sal_in_z const wchar_t* filePath,
		sal_out_opt LoadCallback* loadCallback = nullptr,
		uint priority = 0,
		sal_in_z_opt const wchar_t* args = L"loadAsEntity=true");

private:
	class Impl;
	Impl& mImpl;
};	// EntityPrototypeLoader

/*!	Kind of ResourceManagerCallback to use with EntityPrototypeLoader::addEntityAfterLoad()
	User are suppose to override the doCallback() function to do their job when the load complete.
 */
class MCD_COMPONENT_API EntityPrototypeLoader::LoadCallback : public ResourceManagerCallback
{
public:
	sal_override void doCallback();

	//! Where the tree of loaded Entity will add to.
	EntityPtr addToHere;
	//! The entityPrototype that \em entityAdded clones from.
	EntityPrototypePtr entityPrototype;
};	// LoadCallback

/*!	Load a model as a tree of Entity.
	In order to activate this factory, the string argument should having "loadAsEntity=true"
	\note Currently only *.3ds file is supported.
 */
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
