#ifndef __MCD_RENDER_RENDERER__
#define __MCD_RENDER_RENDERER__

#include "Renderable.h"

namespace MCD {

class RenderTargetComponent;

class MCD_RENDER_API RendererComponent : public Component
{
public:
	RendererComponent();

	sal_override const std::type_info& familyType() const {
		return typeid(RendererComponent);
	}

// Attributes
//	void setDefaultCamera(CameraComponent& camera);
//	sal_maybenull CameraComponent* defaultCamera() const;

// Operations
	//!	Override the default camera
	void render(Entity& entityTree);
	void render(Entity& entityTree, RenderTargetComponent& renderTarget);

	class Impl;
	Impl& mImpl;

protected:
	sal_override virtual ~RendererComponent();
};	// RendererComponent

typedef IntrusiveWeakPtr<RendererComponent> RendererComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERER__
