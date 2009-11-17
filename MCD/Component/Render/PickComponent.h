#ifndef __MCD_COMPONENT_RENDER_PICKINGCOMPONENT__
#define __MCD_COMPONENT_RENDER_PICKINGCOMPONENT__

#include "../ShareLib.h"
#include "../../Core/Entity/BehaviourComponent.h"

namespace MCD {

typedef WeakPtr<class Entity> EntityPtr;

/*!	Detect which entity (with mesh component) is being packed
	within certain screen rectangle area.

	\note
		The opengl projection and view port settings will be
		used when update() is being invoked.
	\note
		Ensure the update() function is invoked before you try to query
		the picking result to ensure the information are up todate.

	\ref: http://www.opengl.org/resources/faq/technical/selection.htm
 */
class MCD_COMPONENT_API PickComponent : public BehaviourComponent
{
	class Impl;
	Impl& mImpl;

public:
	PickComponent();

	sal_override ~PickComponent();

// Operations
	//! Preform the picking detection.
	sal_override void update();

	/*!	Restrict the area of picking detection.
		It's a retangular region of width and height centered at x, y.
		The unit is in screen pixel and the origin start at top left corner.
	 */
	void setPickRegion(size_t x, size_t y, size_t width=1, size_t height=1);

	//! Get the number of hit.
	size_t hitCount() const;

	/*!	Get the n-th picked entity, return null if index out of bound.
		\note The returned pointer will also be null if the Entity itself is deleted.
	 */
	EntityPtr hitAtIndex(size_t index);

	void clearResult();

// Attrubutes
	/*!	The tree of entities that we want to test the picking against with.
		EntityPtr (a WeakPtr) is used since the entiy can be destroyed at any time.
	 */
	EntityPtr entityToPick;
};	// MeshComponent

}	// namespace MCD

#endif	// __MCD_COMPONENT_RENDER_PICKINGCOMPONENT__
