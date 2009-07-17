#ifndef __MCD_BINDING_SCRIPTCOMPONENT__
#define __MCD_BINDING_SCRIPTCOMPONENT__

#include "ShareLib.h"
#include "../Core/Entity/BehaviourComponent.h"
#include "../Core/System/Resource.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/ResourceLoader.h"

namespace MCD {

class MCD_BINDING_API ScriptLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId);
	sal_override IResourceLoader* createLoader();
};	// ScriptLoaderFactory

class MCD_BINDING_API ScriptLoader : public IResourceLoader, Noncopyable
{
public:
	ScriptLoader();

	sal_override ~ScriptLoader();

	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr);

	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

private:
	class Impl;
	Impl* mImpl;
};	// ScriptLoader

class MCD_BINDING_API Script : public Resource
{
public:
	explicit Script(const Path& fileId);

};	// Script

class MCD_BINDING_API ScriptComponent : public BehaviourComponent
{
public:

// Cloning
	sal_override bool cloneable() const { return false; }
	sal_override Component* clone() const { return nullptr; }

// Operations
	//! The derived components should override this function for defining behaviour.
	sal_override void update();
};	// ScriptComponent

}	// namespace MCD

namespace script {

SCRIPT_CLASS_DECLAR_EXPORT(MCD::ScriptComponent, MCD_BINDING_API);

}	// namespace script


#endif	// __MCD_BINDING_SCRIPTCOMPONENT__
