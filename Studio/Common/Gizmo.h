#ifndef __MCD_STUDIO_GIZMO__
#define __MCD_STUDIO_GIZMO__

#include "../../MCD/Core/Entity/Entity.h"

namespace MCD {
class ResourceManager;
}

//! An entity that representing the Gizmo
class Gizmo : public MCD::Entity
{
public:
	Gizmo(MCD::ResourceManager& resourceManager);

	// TODO: Select multiple Entity
	void setSelectedEntity(const MCD::EntityPtr& selectedEntity);

	const MCD::EntityPtr& selectedEntity() const;
};	// Gizmo

#endif	// __MCD_STUDIO_GIZMO__
