#include "Pch.h"
#include "EntityPrototypeLoader.h"
#include "EntityPrototype.h"
#include "MeshComponent.h"
#include "../../Core/System/Log.h"
#include "../../Core/System/MemoryProfiler.h"
#include "../../Core/System/StrUtility.h"
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

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const wchar_t* args);

	void commit(Resource& resource);

	IResourceLoader::LoadingState getLoadingState() const;

private:
	IResourceManager* mResourceManager;

// Actual Loaders
	mutable Mutex mMutex;
	std::auto_ptr<IResourceLoader> mConcreteLoader;
	ResourcePtr mConcreteResource;
};	// Impl

EntityPrototypeLoader::Impl::Impl(IResourceManager* resourceManager)
	: mResourceManager(resourceManager)
{
}

EntityPrototypeLoader::Impl::~Impl()
{
}

IResourceLoader::LoadingState EntityPrototypeLoader::Impl::load(std::istream* is, const Path* fileId, const wchar_t* args)
{
	{
		ScopeLock lock(mMutex);

		if(nullptr == mConcreteLoader.get())
		{
			std::wstring newArgs;

			if(nullptr != args)
			{
				NvpParser parser(args);
				const wchar_t* name, *value;
				while(parser.next(name, value))
				{
					// skip the loadAsEntity arg
					if(wstrCaseCmp(name, L"loadAsEntity") == 0)
						continue;

					newArgs += name;
					newArgs += L"=";
					newArgs += value;
					newArgs += L"; ";
				}
			}

			std::pair<IResourceLoader*, ResourcePtr> r = mResourceManager->customLoad
				(*fileId
				, newArgs.empty() ? nullptr : newArgs.c_str());

			if(nullptr == r.first)
				return Aborted;

			mConcreteLoader.reset(r.first);
			mConcreteResource = r.second;
		}
	}

	return mConcreteLoader->load(is, fileId, args);
}

void EntityPrototypeLoader::Impl::commit(Resource& resource)
{
	MCD_ASSUME(mConcreteLoader.get());
	MCD_ASSUME(mConcreteResource.get());

	// invoke the concrete loader to commit
	mConcreteLoader->commit(*mConcreteResource);

	Model* model = dynamic_cast<Model*>(mConcreteResource.get());
	MCD_ASSUME(model);

	// Convert Model to EntityPrototype
	EntityPrototype& ep = dynamic_cast<EntityPrototype&>(resource);

	Entity* entRoot = new Entity;

	for(Model::MeshAndMaterial* meshAndMat = model->mMeshes.begin()
		; meshAndMat != model->mMeshes.end()
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
	{	ScopeLock lock(mMutex);

		if(nullptr == mConcreteLoader.get())
			return Aborted;
	}

	return mConcreteLoader->getLoadingState();
}

EntityPrototypeLoader::EntityPrototypeLoader(IResourceManager* resourceManager)
{
	mImpl = new Impl(resourceManager);
}

EntityPrototypeLoader::~EntityPrototypeLoader()
{
	delete mImpl;
}

IResourceLoader::LoadingState EntityPrototypeLoader::load(std::istream* is, const Path* fileId, const wchar_t* args)
{
	MemoryProfiler::Scope scope("EntityPrototypeLoader::load");
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->load(is, fileId, args);
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

void EntityPrototypeLoader::LoadCallback::doCallback()
{
	Entity* addHere = addToHere.get();

	// The Entity that we want to insert at, may already destroyed.
	if(!addHere || !entityPrototype)
		return;

	Entity* e = entityPrototype->entity.get();
	if(!e)	// Loading of the Entity failed
		return;
	e = e->clone();
	MCD_ASSUME(e);

	e->asChildOf(addHere);
	entityAdded = e;
}

EntityPrototypePtr EntityPrototypeLoader::addEntityAfterLoad(
	const EntityPtr& addToHere, IResourceManager& manager,
	const wchar_t* filePath,
	LoadCallback* loadCallback,
	uint priority,
	const wchar_t* args)
{
	LoadCallback* callback = loadCallback;
	if(!callback)
		callback = new LoadCallback();

	callback->addToHere = addToHere;
	callback->entityPrototype = dynamic_cast<EntityPrototype*>(manager.load(filePath, false, priority, args).get());

	if(!callback->entityPrototype)
		Log::format(Log::Warn, L"Fail to load \"%s\" as an EntityPrototype", filePath);

	callback->setMajorDependency(filePath);
	manager.addCallback(callback);
	return callback->entityPrototype;
}

EntityPrototypeLoaderFactory::EntityPrototypeLoaderFactory(IResourceManager& resourceManager)
    : mResourceManager(resourceManager)
{
}

ResourcePtr EntityPrototypeLoaderFactory::createResource(const Path& fileId, const wchar_t* args)
{
	if(wstrCaseCmp(fileId.getExtension().c_str(), L"3ds") != 0)
		return nullptr;

	if(!args)
		return nullptr;

	// Detect the string option
	NvpParser parser(args);
	const wchar_t* name, *value;
	while(parser.next(name, value))
	{
		if(wstrCaseCmp(name, L"loadAsEntity") == 0 && wstrCaseCmp(value, L"true") == 0)
			return new EntityPrototype(fileId);
	}

	return nullptr;
}

IResourceLoader* EntityPrototypeLoaderFactory::createLoader()
{
	return new EntityPrototypeLoader(&mResourceManager);
}

}	// namespace MCD
