#include "Pch.h"
#include "EntityPrototypeLoader.h"
#include "MeshComponent.h"
#include "../Prefab.h"
#include "../../Core/System/Log.h"
#include "../../Core/System/MemoryProfiler.h"
#include "../../Core/System/StrUtility.h"
#include "../../Render/Effect.h"
#include "../../Render/Mesh.h"
#include "../../Render/Model.h"

namespace MCD {

class EntityPrototypeLoader::Impl
{
public:
	Impl(IResourceManager* resourceManager)
		: mResourceManager(resourceManager)
	{
	}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const char* args);

	void commit(Resource& resource);

	IResourceLoader::LoadingState getLoadingState() const;

private:
	IResourceManager* mResourceManager;

// Actual Loaders
	mutable Mutex mMutex;
	IResourceLoaderPtr mConcreteLoader;
	ResourcePtr mConcreteResource;
};	// Impl

IResourceLoader::LoadingState EntityPrototypeLoader::Impl::load(std::istream* is, const Path* fileId, const char* args)
{
	if(!mConcreteLoader)
	{
		std::string newArgs;

		if(nullptr != args)
		{
			NvpParser parser(args);
			const char* name, *value;
			while(parser.next(name, value))
			{
				// skip the loadAsEntity arg
				if(strCaseCmp(name, "loadAsEntity") == 0)
					continue;

				newArgs += name;
				newArgs += "=";
				newArgs += value;
				newArgs += "; ";
			}
		}

		std::pair<IResourceLoaderPtr, ResourcePtr> r = mResourceManager->customLoad
			(*fileId
			, newArgs.empty() ? nullptr : newArgs.c_str());

		if(nullptr == r.first)
			return Aborted;

		ScopeLock lock(mMutex);
		mConcreteLoader = r.first;
		mConcreteResource = r.second;
	}

	return mConcreteLoader->load(is, fileId, args);
}

void EntityPrototypeLoader::Impl::commit(Resource& resource)
{
	MCD_ASSERT(mConcreteLoader.get());
	MCD_ASSERT(mConcreteResource.get());

	// Invoke the concrete loader to commit
	mConcreteLoader->commit(*mConcreteResource);

	Model* model = dynamic_cast<Model*>(mConcreteResource.get());
	MCD_ASSUME(model);

	// Convert Model to Prefab
	Prefab& ep = dynamic_cast<Prefab&>(resource);

	Entity* entRoot = new Entity;
	entRoot->name = "tmp root";

	for(Model::MeshAndMaterial* meshAndMat = model->mMeshes.begin()
		; meshAndMat != model->mMeshes.end()
		; meshAndMat = meshAndMat->next()
		)
	{
		std::auto_ptr<Entity> e(new Entity);
		e->name = meshAndMat->name;
		e->localTransform = Mat44f::cIdentity;

		MeshComponent* c = new MeshComponent;
		c->mesh = meshAndMat->mesh;
		c->effect = meshAndMat->effect;
		e->addComponent(c);

		e->asChildOf(entRoot);
		e.release();
	}

	ep.entity.reset(entRoot);
}

IResourceLoader::LoadingState EntityPrototypeLoader::Impl::getLoadingState() const
{
	{	ScopeLock lock(mMutex);

		if(!mConcreteLoader)
			return Aborted;
	}

	return mConcreteLoader->getLoadingState();
}

EntityPrototypeLoader::EntityPrototypeLoader(IResourceManager* resourceManager)
	: mImpl(*new Impl(resourceManager))
{
}

EntityPrototypeLoader::~EntityPrototypeLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState EntityPrototypeLoader::load(std::istream* is, const Path* fileId, const char* args)
{
	MemoryProfiler::Scope scope("EntityPrototypeLoader::load");
	return mImpl.load(is, fileId, args);
}

void EntityPrototypeLoader::commit(Resource& resource)
{
	MemoryProfiler::Scope scope("EntityPrototypeLoader::commit");
	mImpl.commit(resource);
}

IResourceLoader::LoadingState EntityPrototypeLoader::getLoadingState() const
{
	return mImpl.getLoadingState();
}

void EntityPrototypeLoader::LoadCallback::doCallback()
{
	Entity* addHere = addToHere.get();

	// The Entity that we want to insert at, may already destroyed.
	if(!addHere || !entityPrototype)
		return;

	Entity* e_ = entityPrototype->entity.get();
	if(!e_)	// Loading of the Entity failed
		return;
	std::auto_ptr<Entity> e(e_->clone());
	MCD_ASSERT(e.get());

	// Move all the node in the tmp root to the target.
	while(Entity* i=e->firstChild()) {
		i->unlink();
		i->asChildOf(addHere);
	}
}

PrefabPtr EntityPrototypeLoader::addEntityAfterLoad(
	const EntityPtr& addToHere, IResourceManager& manager,
	const char* filePath,
	LoadCallback* loadCallback,
	uint priority,
	const char* args)
{
	LoadCallback* callback = loadCallback;
	if(!callback)
		callback = new LoadCallback();

	callback->addToHere = addToHere;
	callback->entityPrototype = dynamic_cast<Prefab*>(manager.load(filePath, IResourceManager::NonBlock, priority, args).get());

	if(!callback->entityPrototype)
		Log::format(Log::Warn, "Fail to load \"%s\" as an Prefab", filePath);

	callback->setMajorDependency(filePath);
	manager.addCallback(callback);
	return callback->entityPrototype;
}

EntityPrototypeLoaderFactory::EntityPrototypeLoaderFactory(IResourceManager& resourceManager)
    : mResourceManager(resourceManager)
{
}

ResourcePtr EntityPrototypeLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(!args)
		return nullptr;

	// Detect the string option
	NvpParser parser(args);
	const char* name, *value;
	while(parser.next(name, value))
	{
		if(strCaseCmp(name, "loadAsEntity") == 0 && strCaseCmp(value, "true") == 0)
			return new Prefab(fileId);
	}

	return nullptr;
}

IResourceLoader* EntityPrototypeLoaderFactory::createLoader()
{
	return new EntityPrototypeLoader(&mResourceManager);
}

}	// namespace MCD
