#include "Pch.h"
#include "Entity.h"
#include "Binding.h"
#include "Math.h"

using namespace MCD;

namespace script {

SCRIPT_CLASS_REGISTER_NAME(Component, "Component")
//	.method(L"familyType", &Component::familyType)
	.method(L"entity", &Component::entity)
;}

SCRIPT_CLASS_REGISTER_NAME(Entity, "Entity")
	.constructor(L"create")
	.method(L"asChildOf", &Entity::asChildOf)
	.method(L"insertBefore", &Entity::insertBefore)
	.method(L"insertAfter", &Entity::insertAfter)
	.method(L"unlink", &Entity::unlink)
	.getset(L"enabled", &Entity::enabled)
	.getset(L"name", &Entity::name)
//	.getset(L"localTransform", &Entity::localTransform)
	.method<objNoCare>(L"_getparent", &Entity::parent)			// The node's life time is controled by the
	.method<objNoCare>(L"_getfirstChild", &Entity::firstChild)	// node tree's root node, therefore we use
	.method<objNoCare>(L"_getnextSlibing", &Entity::nextSlibing)// objNoCare as the return policy.
;}

}	// namespace script

namespace MCD {

void registerEntityBinding(script::VMCore* v)
{
	script::ClassTraits<Component>::bind(v);
	script::ClassTraits<Entity>::bind(v);
}

}	// namespace MCD
