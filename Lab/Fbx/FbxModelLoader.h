#ifndef __MCD_FBX_FBXMODELLOADER__
#define __MCD_FBX_FBXMODELLOADER__

#include "ShareLib.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Core/System/ResourceLoader.h"
#include "../../MCD/Core/System/ResourceManager.h"

namespace MCD
{

class MCD_FBX_API FbxModelLoader : public IResourceLoader, private Noncopyable
{
public:
	/*!	Constructor, take a ResourceManager as an optional parameter.
		Since the 3ds model may reference some texture files, therefore it needs
		a resource manager in order to load the textures. If null pointer is supplied
		as the manager,
	 */
	explicit FbxModelLoader(sal_maybenull IResourceManager* resourceManager = nullptr);

	sal_override ~FbxModelLoader();

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

private:
	class Impl;
	Impl* mImpl;
};	// FbxModelLoader

/*!	Load a model as a tree of Entity.
	In order to activate this factory, the string argument should having "loadAsEntity=true"
	\note Currently only *.3ds file is supported.
 */
class MCD_FBX_API FbxModelLoaderFactory : public ResourceManager::IFactory
{
public:
    FbxModelLoaderFactory(IResourceManager& resourceManager);
	sal_override ResourcePtr createResource(const Path& fileId, const wchar_t* args);
	sal_override IResourceLoader* createLoader();

private:
	/*!	This loader factory is going to be owned by the mResourceManager, so we can
		use mResourceManager freely during the life-time of the loader factory.
	 */
	IResourceManager& mResourceManager;
};	// FbxModelLoaderFactory

}	// namespace MCD

#endif	// __MCD_FBX_FBXMODELLOADER__
