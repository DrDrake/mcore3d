#include "stdafx.h"
#include "Entity.h"
#include "Utility.h"
#undef nullptr

namespace Binding {

Entity::Entity(MCD::Entity* entity)
{
	mImpl = entity;
}

Entity::Entity(IntPtr entity)
{
	mImpl = reinterpret_cast<MCD::Entity*>(entity.ToPointer());
}

void Entity::link(Entity^ parent)
{
	if(parent)
		mImpl->link(parent->mImpl);
}

void Entity::unlink()
{
	mImpl->unlink();
}

String^ Entity::name::get()
{
   return gcnew String(mImpl->name.c_str());
}

void Entity::name::set(String^ value)
{
	mImpl->name = Utility::toWString(value);
}

Entity^ Entity::parent::get()
{
	return mImpl->parent() ? gcnew Entity(mImpl->parent()) : nullptr;
}

Entity^ Entity::firstChild::get()
{
	return mImpl->firstChild() ? gcnew Entity(mImpl->firstChild()) : nullptr;
}

Entity^ Entity::nextSlibing::get()
{
	return mImpl->nextSlibing() ? gcnew Entity(mImpl->nextSlibing()) : nullptr;
}

}
