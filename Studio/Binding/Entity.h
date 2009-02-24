#pragma once

#include "../../MCD/Core/Entity/Entity.h"

using namespace System;
using namespace System::ComponentModel;

#using <System.Windows.Forms.dll>

namespace Binding {

ref class Vector3Converter;

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

	[CategoryAttribute("Entity"), DescriptionAttribute("Name of the Entity")]
	property String^ name {
		String^ get();
		void set(String^ value);
	}

	[BrowsableAttribute(false)]
	property Entity^ parent {
		Entity^ get();
	}

	[BrowsableAttribute(false)]
	property Entity^ firstChild {
		Entity^ get();
	}

	[BrowsableAttribute(false)]
	property Entity^ nextSlibing {
		Entity^ get();
	}

	[CategoryAttribute("Transform"), DescriptionAttribute("Local position"), TypeConverter(Vector3Converter::typeid)]
	property array<float>^ translation {
		array<float>^ get();
		void set(array<float>^ value);
	}

	[CategoryAttribute("Transform"), DescriptionAttribute("Local rotation in unity of degree"), TypeConverter(Vector3Converter::typeid)]
	property array<float>^ rotation {
		array<float>^ get();
		void set(array<float>^ value);
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
