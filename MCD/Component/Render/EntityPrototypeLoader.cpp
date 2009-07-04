#include "Pch.h"
#include "EntityPrototypeLoader.h"
#include "../../Core/System/MemoryProfiler.h"
#include "../../Render/Max3dsLoader.h"
#include "../../Render/Model.h"

namespace MCD {

class EntityPrototypeLoader::Impl
{
public:
	Impl(IResourceManager* resourceManager);

	~Impl();

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId);

	void commit(Resource& resource);

	IResourceLoader::LoadingState getLoadingState() const;

private:
	IResourceManager* mResourceManager;

// Actual Loaders
	Max3dsLoader m3dsLoader;
};	// Impl

EntityPrototypeLoader::Impl::Impl(IResourceManager* resourceManager)
	: m3dsLoader(resourceManager)
{
}

EntityPrototypeLoader::Impl::~Impl()
{
}

IResourceLoader::LoadingState EntityPrototypeLoader::Impl::load(std::istream* is, const Path* fileId)
{
	return m3dsLoader.load(is, fileId);
}

void EntityPrototypeLoader::Impl::commit(Resource& resource)
{
	return m3dsLoader.commit(resource);
}

IResourceLoader::LoadingState EntityPrototypeLoader::Impl::getLoadingState() const
{
	return m3dsLoader.getLoadingState();
}

EntityPrototypeLoader::EntityPrototypeLoader(IResourceManager* resourceManager)
{
	mImpl = new Impl(resourceManager);
}

EntityPrototypeLoader::~EntityPrototypeLoader()
{
	delete mImpl;
}

IResourceLoader::LoadingState EntityPrototypeLoader::load(std::istream* is, const Path* fileId)
{
	MemoryProfiler::Scope scope("EntityPrototypeLoader::load");
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->load(is, fileId);
}

void EntityPrototypeLoader::commit(Resource& resource)
{
	MemoryProfiler::Scope scope("EntityPrototypeLoader::commit");
	MCD_ASSUME(mImpl != nullptr);
	mImpl->commit(resource);
}

IResourceLoader::LoadingState EntityPrototypeLoader::getLoadingState() const
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->getLoadingState();
}

}	// namespace MCD
