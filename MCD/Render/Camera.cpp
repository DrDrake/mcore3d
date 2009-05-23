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
	Vec3f f = lookAtDir();
	Vec3f s = f.cross(upVector.normalizedCopy());
	Vec3f u = s.cross(f);

	matrix[0] = s.x;	matrix[1] = s.y;	matrix[2] = s.z;	matrix[3] = 0;
	matrix[4] = u.x;	matrix[5] = u.y;	matrix[6] = u.z;	matrix[7] = 0;
	matrix[8] = -f.x;	matrix[9] = -f.y;	matrix[10] = -f.z;	matrix[11] = 0;
	matrix[12] = 0;		matrix[13] = 0;		matrix[14] = 0;		matrix[15] = 1.0f;

	Mat44f tmp;
	tmp.copyFrom(matrix);

	Mat44f translate = Mat44f::cIdentity;
	translate.setTranslation(-position);

	// Translate then rotate
	(tmp * translate).copyTo(matrix);
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
	glLoadTransposeMatrixf(mat.getPtr());
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
