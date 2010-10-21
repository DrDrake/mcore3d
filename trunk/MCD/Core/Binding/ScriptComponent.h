#ifndef __MCD_CORE_BINDING_SCRIPTCOMPONENT__
#define __MCD_CORE_BINDING_SCRIPTCOMPONENT__

#include "../Entity/BehaviourComponent.h"

namespace MCD {
namespace Binding {

/// To be sub-classed in script
class ScriptComponent : public BehaviourComponent
{
public:
	sal_override void update(float dt);
};	// ScriptComponent

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_CORE_BINDING_SCRIPTCOMPONENT__
