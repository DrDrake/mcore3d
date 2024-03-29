#ifndef __MCD_RENDER_CAMERA__
#define __MCD_RENDER_CAMERA__

#include "Frustum.h"
#include "../Core/Math/Vec3.h"

namespace MCD {

/*!	Using position, lookAt and upVector to define a camera.
	The lookAt and upVector are NOT assumed to be unit vector.
 */
class MCD_RENDER_API Camera
{
public:
	/*!	Default constructor
		With default parameters:
			position = {0, 0, 0}
			lookAt   = {0, 0, -1}
			upVector = {0, 1, 0}
	 */
	Camera();

	Camera(const Vec3f& position, const Vec3f& lookAt, const Vec3f& upVector);

// Operations
	//!	Compute the view transform.
	void computeView(sal_out_ecount(16) float* matrix) const;

// Attributes
	//! Unit direction vector which the camera is directing.
	Vec3f lookAtDir() const;

	//! The direction vector pointing to the right of the camera.
	Vec3f rightVector() const;

	Vec3f position;		//!< Position of the camera.
	Vec3f lookAt;		//!< The direction which the camera is looking at.
	Vec3f upVector;		//!< Upward direction of the camera.

	Frustum frustum;	//!< The view frustum.
};	// Camera

}	// namespace MCD

#include "Renderable.h"

namespace MCD {

typedef IntrusiveWeakPtr<class RendererComponent> RendererComponentPtr;

class MCD_RENDER_API CameraComponent : public RenderableComponent
{
public:
	explicit CameraComponent(const RendererComponentPtr& renderer);

// Cloning
	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render(sal_in void* context) {}

// Attrubutes
	Frustum frustum;
	RendererComponentPtr renderer;
};	// CameraComponent

typedef IntrusiveWeakPtr<CameraComponent> CameraComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_CAMERA__
