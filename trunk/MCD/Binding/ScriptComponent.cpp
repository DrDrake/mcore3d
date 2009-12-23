#include "Pch.h"
#include "ScriptComponent.h"
#include "../Core/System/MemoryProfiler.h"

using namespace MCD;

namespace MCD {

ResourcePtr ScriptLoaderFactory::createResource(const Path& fileId)
{
	return nullptr;
}

IResourceLoader* ScriptLoaderFactory::createLoader()
{
	return nullptr;
}

class ScriptLoader::Impl
{
public:
	LoadingState load(std::istream* is, const Path* fileId)
	{
		return NotLoaded;
	}

	LoadingState getLoadingState() const
	{
		ScopeLock lock(mMutex);
		return mLoadingState;
	}

	volatile IResourceLoader::LoadingState mLoadingState;
	mutable Mutex mMutex;
};

ScriptLoader::ScriptLoader()
	: mImpl(*new Impl)
{
}

ScriptLoader::~ScriptLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState ScriptLoader::load(std::istream* is, const Path* fileId)
{
	MemoryProfiler::Scope scope("ScriptLoader::load");
	return mImpl.load(is, fileId);
}

void ScriptLoader::commit(Resource& resource)
{
}

IResourceLoader::LoadingState ScriptLoader::getLoadingState() const
{
	return mImpl.getLoadingState();
}

Script::Script(const Path& fileId)
	: Resource(fileId)
{
}

void ScriptComponent::update(float)
{
}

Component* ScriptComponent::clone() const
{
	return new ScriptComponent;
}

void registerScriptComponentBinding(script::VMCore* v)
{
	script::ClassTraits<ScriptComponent>::bind(v);
}

}	// namespace MCD

namespace script {

SCRIPT_CLASS_REGISTER(ScriptComponent)
	.declareClass<ScriptComponent, Component>(xSTRING("ScriptComponent"))
	.constructor()
;}

}	// namespace script
