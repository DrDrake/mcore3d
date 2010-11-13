#ifndef __MCD_RENDER_ANIMATEDTRANSFORM__
#define __MCD_RENDER_ANIMATEDTRANSFORM__

#include "Animation.h"

namespace MCD {

typedef IntrusiveWeakPtr<class Entity> EntityPtr;

/// Use the animation data to alter the Entity's localTransform.
class MCD_RENDER_API AnimatedTransform : public AnimatedComponent
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(AnimatedTransform);
	}

// Attributes
	size_t trackOffset;		///< Starting index to the animation's tracks
	size_t trackPerEntity;	///< 1 For position only, 2 for position + rotation, 3 for position + rotation + scaling
	AnimationComponentPtr animation;

	std::vector<EntityPtr> affectingEntities;

protected:
	sal_override void update();
};	// AnimatedTransform

typedef IntrusiveWeakPtr<AnimationComponent> AnimationComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_ANIMATEDTRANSFORM__
