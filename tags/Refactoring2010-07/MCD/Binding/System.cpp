#include "Pch.h"
#include "System.h"
#include "Binding.h"
#include "../Core/System/RawFileSystem.h"
#include "../Core/System/Resource.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/Timer.h"

using namespace MCD;

namespace script {

void ResourceRefPolicy::addRef(MCD::Resource* resource) {
	if(resource)
		intrusivePtrAddRef(resource);
}
void ResourceRefPolicy::releaseRef(MCD::Resource* resource) {
	if(resource)
		intrusivePtrRelease(resource);
}

namespace types {

ClassID getClassIDFromObject(const Resource* obj, ClassID original) {
	return getClassIDFromTypeInfo(typeid(*obj), original);
}

}	// namespace types

static void resourceManagerCallbackAddDependency(ResourceManagerCallback& self, const char* fileId) {
	self.addDependency(fileId);
}
static void resourceManagerCallbackSetMajorDependency(ResourceManagerCallback& self, const char* fileId) {
	self.setMajorDependency(fileId);
}
SCRIPT_CLASS_REGISTER_NAME(ResourceManagerCallback, "ResuorceManagerCallback")
	.enableGetset()
	.wrappedMethod("addDependency", &resourceManagerCallbackAddDependency)
	.wrappedMethod("_setmajorDependency", &resourceManagerCallbackSetMajorDependency)
;}

static Resource* resourceManagerLoad(IResourceManager& self, const char* fileId, bool block, uint priority, const char* args) {
	// TODO: Fix the blocking option
	return self.load(fileId, IResourceManager::NonBlock, priority, args).get();
}
static void resourceManagerAddCallback(IResourceManager& self, GiveUpOwnership<ResourceManagerCallback*> callback) {
	self.addCallback(callback);
}
SCRIPT_CLASS_REGISTER_NAME(IResourceManager, "ResuorceManager")
	.wrappedMethod<objRefCount<ResourceRefPolicy> >("load", &resourceManagerLoad)
	.wrappedMethod("addCallback", &resourceManagerAddCallback)
;}

SCRIPT_CLASS_REGISTER_NAME(Path, "Path")
	.constructor()
	.method("getString", &Path::getString)
;}

static float timerGet(Timer& timer) {
	return float(timer.get().asSecond());
}
static float timerReset(Timer& timer) {
	return float(timer.reset().asSecond());
}
SCRIPT_CLASS_REGISTER_NAME(Timer, "Timer")
	.constructor()
	.wrappedMethod("get", &timerGet)
	.wrappedMethod("reset", &timerReset)
;}

SCRIPT_CLASS_REGISTER_NAME(RawFileSystem, "RawFileSystem")
	.constructor<const Path&>()
	.method("setRoot", &RawFileSystem::setRoot)
;}

SCRIPT_CLASS_REGISTER_NAME(Resource, "Resource")
;}

// TODO: Handle endian problem
static const char* floatToHex(float f) {
	static char buf[64];	// NOTE: Will have multi-thread problem if the VM support multi-thread later.
	MCD_ASSERT(sizeof(float) == sizeof(int));
	if(f == 0)
		::sprintf(buf, "%s", "00000000");
	else
		::sprintf(buf, "%X", *((int*)(&f)));
	MCD_ASSERT(::strlen(buf) == sizeof(float) * 2);
	return buf;
}

static float floatFromHex(const char* s) {
	MCD_ASSERT(::strlen(s) == sizeof(float) * 2);
	float ret;
	MCD_VERIFY(::sscanf(s, "%X", (int*)(&ret)) == 1);
	return ret;
}

}	// namespace script

namespace MCD {

void registerSystemBinding(script::VMCore* v)
{
	script::ClassTraits<IResourceManager>::bind(v);
	script::ClassTraits<ResourceManagerCallback>::bind(v);
	script::ClassTraits<Path>::bind(v);
	script::ClassTraits<Timer>::bind(v);
	script::ClassTraits<RawFileSystem>::bind(v);
	script::ClassTraits<Resource>::bind(v);

	script::RootDeclarator root(v);
	root.declareFunction("floatToHex", &script::floatToHex);
	root.declareFunction("floatFromHex", &script::floatFromHex);
}

}	// namespace MCD
