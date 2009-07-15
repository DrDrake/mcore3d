#ifndef __MCD_COMPONENT_RENDER_ENTITYPROTOTYPE__
#define __MCD_COMPONENT_RENDER_ENTITYPROTOTYPE__

#include "../ShareLib.h"
#include "../../Core/System/Resource.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

class IResourceManager;

class MCD_COMPONENT_API EntityPrototype : public Resource, Noncopyable
{
public:
	explicit EntityPrototype(const Path& fileId);

	sal_override ~EntityPrototype();

	/*! When an EntityPrototype is loaded, mEntity is not be NULL */
	std::auto_ptr<Entity> mEntity;

};	// EntityPrototype

}	// namespace MCD

#endif	// __MCD_COMPONENT_RENDER_ENTITYPROTOTYPE__
