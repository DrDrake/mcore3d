#include "Pch.h"
#include "EntityPrototype.h"

namespace MCD {

EntityPrototype::EntityPrototype(const Path& fileId)
	: Resource(fileId)
{
}

EntityPrototype::~EntityPrototype()
{
}

Component* EntityPrototypeComponent::clone() const
{
	EntityPrototypeComponent* c = new EntityPrototypeComponent;
	c->entityPrototype = entityPrototype;
	return c;
}

}	// namespace MCD
