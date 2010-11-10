#ifndef __MCD_CORE_BINDING_SCRIPTCOMPONENT__
#define __MCD_CORE_BINDING_SCRIPTCOMPONENT__

#include "../Entity/BehaviourComponent.h"

typedef ssize_t SQInteger;
typedef struct SQVM* HSQUIRRELVM;

namespace MCD {
namespace Binding {

class VMCore;

/// To be sub-classed in script
class MCD_CORE_API ScriptComponent : public BehaviourComponent
{
public:
	ScriptComponent();

	sal_override void update(float dt);

protected:
	friend SQInteger wakeup_ScriptComponent(HSQUIRRELVM vm);
	friend class ScriptManagerComponent;
	void* mThreadVM;
	bool mSuspended;
};	// ScriptComponent

class MCD_CORE_API ScriptManagerComponent : public ComponentUpdater
{
public:
	explicit ScriptManagerComponent(VMCore* vmcore);

protected:
	/// Will wakeup up sleeped script at the right time
	sal_override void end(float dt);

	VMCore* mVMCore;
};	// ScriptManagerComponent

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_CORE_BINDING_SCRIPTCOMPONENT__
