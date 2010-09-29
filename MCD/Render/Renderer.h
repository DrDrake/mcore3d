#ifndef __MCD_RENDER_RENDERER__
#define __MCD_RENDER_RENDERER__

#include "Renderable.h"

namespace MCD {

class RenderTargetComponent;

class MCD_RENDER_API RendererComponent : public ComponentUpdater
{
public:
	RendererComponent();

// Attributes
//	void setDefaultCamera(CameraComponent& camera);
//	sal_maybenull CameraComponent* defaultCamera() const;

// Operations
	//!	Override the default camera
	void render(Entity& entityTree);
	void render(Entity& entityTree, RenderTargetComponent& renderTarget);

	class Impl;
	Impl& mImpl;

	static RendererComponent& current();

	//! Invoked by RenderableComponent::gather()
	static void addRenderableToCurrent(RenderableComponent& renderable);

protected:
	sal_override virtual ~RendererComponent();
	sal_override void begin();
	sal_override void end(float dt);
};	// RendererComponent

typedef IntrusiveWeakPtr<RendererComponent> RendererComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERER__
