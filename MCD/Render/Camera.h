#ifndef __MCD_RENDER_CAMERA__
#define __MCD_RENDER_CAMERA__

#include "ShareLib.h"
#include "../Core/Math/Vec3.h"

namespace MCD {

/*!	Using position, lookAt and upVector to define a camera.
	The lookAt and upVector are NOT assumed to be unit vector.
 */
class MCD_RENDER_API Camera
{
public:
	Camera() {}

	Camera(const Vec3f& position, const Vec3f lookAt, const Vec3f& upVector);

	void move(const Vec3f& direction, float amount);

	void moveForward(float amount);

	void moveRight(float amount);

	void moveUp(float amount);

	void rotate(const Vec3f& axis, float angle);

	void applyTransform();

	//! Unit direction vector which the camera is directing.
	Vec3f lookAtDir() const;

	//! The direction vector pointing to the right of the camera.
	Vec3f rightVector() const;

	Vec3f position;	//!< Position of the camera.
	Vec3f lookAt;	//!< The direction which the camera is look at.
	Vec3f upVector;	//!< Upward direction of the camera.
};	// Camera

}	// namespace MCD

#endif	// __MCD_RENDER_CAMERA__
