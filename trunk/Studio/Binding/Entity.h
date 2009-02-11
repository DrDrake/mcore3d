#pragma once

#include "../../MCD/Core/Entity/Entity.h"

using namespace System;
#using <System.Windows.Forms.dll>

namespace Binding {

/*!	Will not take any ownership of the entity object.
 */
public ref class Entity
{
	// Actually it become private because of the native parameter.
	Entity(MCD::Entity* entity);

public:
	//! Create the Entity, all the cache/internal/tree-view node pointers will be initialized as well.
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

	System::Windows::Forms::TreeNode^ treeViewNode;

protected:
	MCD::Entity* mImpl;
	/*!	Cached value of the node pointers, these value will be refreshed
		every time the property get function is invoked.
	 */
	Entity^ mParent, ^mFirstChild, ^mNextSlibing;
};

}
