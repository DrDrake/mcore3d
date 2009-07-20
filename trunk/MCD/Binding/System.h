#ifndef __MCD_BINDING_SYSTEM__
#define __MCD_BINDING_SYSTEM__

#include "ShareLib.h"
#include "../Core/System/IncludeAll.h"
#include "../../3Party/jkbind/Declarator.h"

namespace script {

SCRIPT_CLASS_DECLAR_EXPORT(MCD::IResourceManager, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::IResourceManagerCallback, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::Path, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::Timer, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::RawFileSystem, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::Resource, MCD_BINDING_API);

}	// namespace script

#endif	// __MCD_BINDING_SYSTEM__
