#include "Pch.h"
#include "System.h"
#include "Binding.h"
#include "../Core/System/Timer.h"

using namespace MCD;

namespace script {

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
	.constructor<const Path&>(L"create")
	.method(L"setRoot", &RawFileSystem::setRoot)
;}

}	// namespace script

namespace MCD {

void registerSystemBinding(script::VMCore* v)
{
	script::ClassTraits<Path>::bind(v);
	script::ClassTraits<Timer>::bind(v);
	script::ClassTraits<RawFileSystem>::bind(v);
}

}	// namespace MCD
