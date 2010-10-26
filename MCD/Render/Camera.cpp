#include "Pch.h"
#include "Camera.h"
#include "../Core/Math/Mat44.h"
#include "../Core/Math/Quaternion.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

Camera::Camera()
	: position(0), lookAt(0, 0, -1), upVector(0, 1, 0)
{
}

Camera::Camera(const Vec3f& pos, const Vec3f& look, const Vec3f& up)
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
	Vec3f view = lookAtDir();
	rotation.transform(view);
	lookAt = position + view;

	rotation.transform(upVector);
}

void Camera::computeView(float* matrix) const
{
	// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=479402
	const Vec3f f = lookAtDir();
	const Vec3f s = f.cross(upVector.normalizedCopy());
	const Vec3f u = s.cross(f);

	Mat44f& m = *reinterpret_cast<Mat44f*>(matrix);

	m.m00 = s.x;	m.m01 = s.y;	m.m02 = s.z;	m.m03 = 0;
	m.m10 = u.x;	m.m11 = u.y;	m.m12 = u.z;	m.m13 = 0;
	m.m20 = -f.x;	m.m21 = -f.y;	m.m22 = -f.z;	m.m23 = 0;
	m.m30 = 0;		m.m31 = 0;		m.m32 = 0;		m.m33 = 1.0f;

	Mat44f translate = Mat44f::cIdentity;
	translate.setTranslation(-position);

	// Translate then rotate
	(m * translate).copyTo(matrix);
}

void Camera::applyViewTransform()
{
	glMatrixMode(GL_MODELVIEW);

	// The same can be acheived using gluLookAt()
//	gluLookAt(
//		position.x, position.y, position.z,
//		lookAt.x, lookAt.y, lookAt.z,
//		upVector.x, upVector.y, upVector.z);

	Mat44f mat;
	computeView(mat.getPtr());
	glLoadMatrixf(mat.getPtr());
}

void Camera::applyTransform()
{
	frustum.applyProjection();
	applyViewTransform();
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

#include "Renderer.h"

namespace MCD {

CameraComponent::CameraComponent(const RendererComponentPtr& r)
	: renderer(r)
{
}

Component* CameraComponent::clone() const
{
	CameraComponent* cloned = new CameraComponent(renderer);
	cloned->frustum = this->frustum;
	return cloned;
}

}	// namespace MCD
