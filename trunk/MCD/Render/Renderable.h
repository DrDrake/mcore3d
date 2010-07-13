#ifndef __MCD_RENDER_RENDERABLE__
#define __MCD_RENDER_RENDERABLE__

#include "ShareLib.h"
#include "../Core/System/Platform.h"
#include "../Core/Entity/Component.h"

namespace MCD {

class MCD_ABSTRACT_CLASS IRenderable
{
public:
	virtual ~IRenderable() {}

	virtual void draw() = 0;
};	// IRenderable

/*!	A common interface that will make draw call.
	To simply renderer implementation.
 */
class MCD_ABSTRACT_CLASS IDrawCall
{
public:
	virtual ~IDrawCall() {}

	virtual void draw(sal_in void* context) = 0;
};	// IDrawCall

/*!	The component family which is something renderable.
 */
class MCD_ABSTRACT_CLASS MCD_RENDER_API RenderableComponent2 : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(RenderableComponent2);
	}

	//! The derived components should override this function for rendering.
	virtual void render() = 0;
	//!	Invoked by RendererComponent
	virtual void render2(sal_in void* context) = 0;

	//! Invoke the RenderableComponent::render() in every Entity under the entityNode sub-tree.
	static void traverseEntities(sal_maybenull Entity* entityNode);
};	// RenderableComponent2

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERABLE__
