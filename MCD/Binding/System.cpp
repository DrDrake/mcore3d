#include "Pch.h"
#include "System.h"
#include "Binding.h"

using namespace MCD;

namespace script {

struct resourceRefPolicy {
	static void addRef(Resource* resource) {
		intrusivePtrAddRef(resource);
	}
	static void releaseRef(Resource* resource) {
		intrusivePtrAddRef(resource);
	}
};	// resourceRefPolicy

static void resourceManagerCallbackAddDependency(IResourceManagerCallback& self, const wchar_t* fileId) {
	self.addDependency(fileId);
}
SCRIPT_CLASS_REGISTER_NAME(IResourceManagerCallback, "ResuorceManagerCallback")
	.wrappedMethod(L"addDependency", &resourceManagerCallbackAddDependency)
;}

static Resource* resourceManagerLoad(IResourceManager& self, const wchar_t* fileId, bool block, uint priority) {
	return self.load(fileId, block, priority).get();
}
static void resourceManagerAddCallback(IResourceManager& self, GiveUpOwnership<IResourceManagerCallback*> callback) {
	self.addCallback(callback);
}
SCRIPT_CLASS_REGISTER_NAME(IResourceManager, "ResuorceManager")
	.wrappedMethod<objRefCount<resourceRefPolicy> >(L"load", &resourceManagerLoad)
	.wrappedMethod(L"addCallback", &resourceManagerAddCallback)
;}

SCRIPT_CLASS_REGISTER_NAME(Path, "Path")
	.constructor()
	.method(L"getString", &Path::getString)
;}

static float timerGet(Timer& timer) {
	return float(timer.get().asSecond());
}
static float timerReset(Timer& timer) {
	return float(timer.reset().asSecond());
}
SCRIPT_CLASS_REGISTER_NAME(Timer, "Timer")
	.constructor()
	.wrappedMethod(L"get", &timerGet)
	.wrappedMethod(L"reset", &timerReset)
;}

SCRIPT_CLASS_REGISTER_NAME(RawFileSystem, "RawFileSystem")
	.constructor<const Path&>()
	.method(L"setRoot", &RawFileSystem::setRoot)
;}

SCRIPT_CLASS_REGISTER_NAME(Resource, "Resource")
;}

}	// namespace script

namespace MCD {

void registerSystemBinding(script::VMCore* v)
{
	script::ClassTraits<IResourceManager>::bind(v);
	script::ClassTraits<IResourceManagerCallback>::bind(v);
	script::ClassTraits<Path>::bind(v);
	script::ClassTraits<Timer>::bind(v);
	script::ClassTraits<RawFileSystem>::bind(v);
	script::ClassTraits<Resource>::bind(v);
}

}	// namespace MCD
