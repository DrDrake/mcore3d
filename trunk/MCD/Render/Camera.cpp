#include "Pch.h"
#include "Camera.h"
#include "../Core/Math/Quaternion.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

Camera::Camera(const Vec3f& pos, const Vec3f look, const Vec3f& up)
	: position(pos), lookAt(look), upVector(up)
{
}

void Camera::move(const Vec3f& direction, float amount)
{
	Vec3f v = direction.normalizedCopy() * amount;
	position += v;
	lookAt += v;
}

void Camera::moveForward(float amount)
{
	move(lookAtDir(), amount);
}

void Camera::moveRight(float amount)
{
	Vec3f v = rightVector() * amount;
	position += v;
	lookAt += v;
}

void Camera::moveUp(float amount)
{
	Vec3f v = upVector.normalizedCopy() * amount;
	position += v;
	lookAt += v;
}

void Camera::rotate(const Vec3f& axis, float angle)
{
	Quaternionf rotation;
	rotation.fromAxisAngle(axis, angle);

	// We are rotating lookAt - position
	Quaternionf view(lookAtDir(), 1);
	Quaternionf newView(rotation * view * rotation.conjugate());
	lookAt = position + newView.getVec3();

	// Reference: http://www.gamedev.net/reference/articles/article1824.asp
	upVector = (rotation * Quaternionf(upVector, 1) * rotation.conjugate()).getVec3();
}

void Camera::applyTransform()
{
	gluLookAt(position.x, position.y, position.z,
		lookAt.x, lookAt.y, lookAt.z,
		upVector.x, upVector.y, upVector.z);
}

Vec3f Camera::lookAtDir() const
{
	return (lookAt - position).normalizedCopy();
}

Vec3f Camera::rightVector() const
{
	return lookAtDir() ^ upVector;
}

}	// namespace MCD
