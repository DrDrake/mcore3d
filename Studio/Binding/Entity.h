#pragma once

#include "../../MCD/Core/Entity/Entity.h"

using namespace System;

namespace Binding {

/*!	Will not take any ownership of the entity object.
 */
public ref class Entity
{
public:
	// Seems it become private by default.
	Entity(MCD::Entity* entity);

	Entity(IntPtr entity);

	void link(Entity^ parent);

	void unlink();

	property String^ name {
      String^ get();
      void set(String^ value);
   }

	property Entity^ parent {
      Entity^ get();
   }

	property Entity^ firstChild {
      Entity^ get();
   }

	property Entity^ nextSlibing {
      Entity^ get();
   }

protected:
	MCD::Entity* mImpl;
};

}
