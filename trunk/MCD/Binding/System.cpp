#include "Pch.h"
#include "System.h"
#include "Binding.h"

using namespace MCD;

namespace script {

SCRIPT_CLASS_REGISTER_NAME(Path, "Path")
	.constructor(L"create")
	.method(L"getString", &Path::getString)
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
	script::ClassTraits<RawFileSystem>::bind(v);
}

}	// namespace MCD
