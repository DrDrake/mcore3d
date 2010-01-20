#include "Pch.h"
#include "System.h"
#include "Binding.h"
#include "Binding.h"

using namespace MCD;

namespace script {

namespace types {

ClassID getClassIDFromObject(const Resource* obj, ClassID original) {
	return getClassIDFromTypeInfo(typeid(*obj), original);
}

}	// namespace types

static void resourceManagerCallbackAddDependency(ResourceManagerCallback& self, const wchar_t* fileId) {
	self.addDependency(fileId);
}
static void resourceManagerCallbackSetMajorDependency(ResourceManagerCallback& self, const wchar_t* fileId) {
	self.setMajorDependency(fileId);
}
SCRIPT_CLASS_REGISTER_NAME(ResourceManagerCallback, "ResuorceManagerCallback")
	.enableGetset()
	.wrappedMethod(xSTRING("addDependency"), &resourceManagerCallbackAddDependency)
	.wrappedMethod(xSTRING("_setmajorDependency"), &resourceManagerCallbackSetMajorDependency)
;}

static Resource* resourceManagerLoad(IResourceManager& self, const wchar_t* fileId, bool block, uint priority, const wchar_t* args) {
	// TODO: Fix the blocking option
	return self.load(fileId, IResourceManager::NonBlock, priority, args).get();
}
static void resourceManagerAddCallback(IResourceManager& self, GiveUpOwnership<ResourceManagerCallback*> callback) {
	self.addCallback(callback);
}
SCRIPT_CLASS_REGISTER_NAME(IResourceManager, "ResuorceManager")
	.wrappedMethod<objRefCount<ResourceRefPolicy> >(xSTRING("load"), &resourceManagerLoad)
	.wrappedMethod(xSTRING("addCallback"), &resourceManagerAddCallback)
;}

SCRIPT_CLASS_REGISTER_NAME(Path, "Path")
	.constructor()
	.method(xSTRING("getString"), &Path::getString)
;}

static float timerGet(Timer& timer) {
	return float(timer.get().asSecond());
}
static float timerReset(Timer& timer) {
	return float(timer.reset().asSecond());
}
SCRIPT_CLASS_REGISTER_NAME(Timer, "Timer")
	.constructor()
	.wrappedMethod(xSTRING("get"), &timerGet)
	.wrappedMethod(xSTRING("reset"), &timerReset)
;}

SCRIPT_CLASS_REGISTER_NAME(RawFileSystem, "RawFileSystem")
	.constructor<const Path&>()
	.method(xSTRING("setRoot"), &RawFileSystem::setRoot)
;}

SCRIPT_CLASS_REGISTER_NAME(Resource, "Resource")
;}

// TODO: Handle endian problem
static const wchar_t* floatToHex(float f) {
	static wchar_t buf[64];	// NOTE: Will have multi-thread problem if the VM support multi-thread later.
	MCD_ASSERT(sizeof(float) == sizeof(int));
	if(f == 0)
		::swprintf(buf, sizeof(buf)/sizeof(wchar_t), L"%s", L"00000000");
	else
		::swprintf(buf, sizeof(buf)/sizeof(wchar_t), L"%X", *((int*)(&f)));
	MCD_ASSERT(::wcslen(buf) == sizeof(float) * 2);
	return buf;
}

static float floatFromHex(const wchar_t* s) {
	MCD_ASSERT(::wcslen(s) == sizeof(float) * 2);
	float ret;
	MCD_VERIFY(::swscanf(s, L"%X", (int*)(&ret)) == 1);
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
	root.declareFunction(xSTRING("floatToHex"), &script::floatToHex);
	root.declareFunction(xSTRING("floatFromHex"), &script::floatFromHex);
}

}	// namespace MCD
