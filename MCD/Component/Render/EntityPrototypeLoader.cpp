#include "Pch.h"
#include "EntityPrototypeLoader.h"
#include "EntityPrototype.h"
#include "MeshComponent.h"
#include "../../Core/System/MemoryProfiler.h"
#include "../../Core/System/StrUtility.h"
#include "../../Render/Max3dsLoader.h"
#include "../../Render/Model.h"
#include "../../Render/Effect.h"
#include "../../Render/Mesh.h"

namespace MCD {

/*! EntityPrototypeLoader */
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
	Model model(resource.fileId());
	m3dsLoader.commit(model);

	// convert Model to EntityPrototype
	EntityPrototype& ep = dynamic_cast<EntityPrototype&>(resource);

	Entity* entRoot = new Entity;

	for(Model::MeshAndMaterial* meshAndMat = model.mMeshes.begin()
		; meshAndMat != model.mMeshes.end()
		; meshAndMat = meshAndMat->next()
		)
	{
		std::auto_ptr<Entity> e(new Entity);
		e->localTransform = Mat44f::cIdentity;

		MeshComponent* c = new MeshComponent;
		c->mesh = meshAndMat->mesh;
        c->effect = new Effect(Path(L""));
		c->effect->material.reset(meshAndMat->material->clone());
		e->addComponent(c);

		e->asChildOf(entRoot);
		e.release();
	}

	ep.entity.reset(entRoot);
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

/*! EntityPrototypeLoaderFactory */
EntityPrototypeLoaderFactory::EntityPrototypeLoaderFactory(IResourceManager& resourceManager)
    : mResourceManager(resourceManager)
{
}

ResourcePtr EntityPrototypeLoaderFactory::createResource(const Path& fileId, const wchar_t* args)
{
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"3ds") == 0)
		return new EntityPrototype(fileId);
	return nullptr;
}

IResourceLoader* EntityPrototypeLoaderFactory::createLoader()
{
	return new EntityPrototypeLoader(&mResourceManager);
}

}	// namespace MCD
