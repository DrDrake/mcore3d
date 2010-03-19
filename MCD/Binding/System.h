#ifndef __MCD_BINDING_SYSTEM__
#define __MCD_BINDING_SYSTEM__

#include "ShareLib.h"
#include "../../3Party/jkbind/Declarator.h"

namespace MCD {

class IResourceManager;
class ResourceManagerCallback;
class Path;
class RawFileSystem;
class Resource;
class Timer;

}	// namespace MCD

namespace script {

SCRIPT_CLASS_DECLAR_EXPORT(MCD::IResourceManager, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::ResourceManagerCallback, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::Path, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::RawFileSystem, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::Resource, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::Timer, MCD_BINDING_API);

struct MCD_BINDING_API ResourceRefPolicy {
	static void addRef(MCD::Resource* resource);
	static void releaseRef(MCD::Resource* resource);
};	// ResourceRefPolicy

namespace types {

MCD_BINDING_API ClassID getClassIDFromObject(const MCD::Resource* obj, ClassID dummy);

}	// namespace types

}	// namespace script

#endif	// __MCD_BINDING_SYSTEM__
