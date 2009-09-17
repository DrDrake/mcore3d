#pragma once

#include "../../MCD/Core/System/ResourceManager.h"
#undef nullptr

namespace Binding {

/*!	A place for everyone to access
 */
public ref class ResourceManager
{
public:
	ResourceManager();

protected:
	~ResourceManager();
	!ResourceManager();

	MCD::IResourceManager* mImpl;
};	// ResourceManager

}	// namespace Binding
