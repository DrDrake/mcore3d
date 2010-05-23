#ifndef __MCD_RENDER_RENDERER__
#define __MCD_RENDER_RENDERER__

#include "Renderable.h"

namespace MCD {

class CameraComponent2;

class MCD_RENDER_API RendererComponent : public Component
{
public:
	RendererComponent();
	sal_override virtual ~RendererComponent();

	sal_override const std::type_info& familyType() const {
		return typeid(RendererComponent);
	}

// Attributes
	void setDefaultCamera(CameraComponent2& camera);
	sal_maybenull CameraComponent2* defaultCamera() const;

// Operations
	//!	Override the default camera
	void render(Entity& entityTree, sal_maybenull CameraComponent2* camera=nullptr);

	class Impl;
	Impl& mImpl;
};	// RendererComponent

typedef IntrusiveWeakPtr<RendererComponent> RendererComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERER__
