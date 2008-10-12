#include "Pch.h"
#include "Frustum.h"
#include "../Core/Math/BasicFunction.h"
#include "../Core/Math/Vec3.h"

namespace MCD {

void Frustum::create(float fovy, float aspect, float n, float f)
{
	float halfHeight = n * tanf(fovy * (Mathf::cPi() / 360));
	float halfWidth = halfHeight * aspect;

	create(-halfWidth, halfWidth, -halfHeight, halfHeight, n, f);
}

void Frustum::create(float l, float r, float b, float t, float n, float f)
{
	left = l;	right = r;
	bottom = b;	top = t;
	near = n;	far = f;

	assertValid();
}

void Frustum::computePerspective(float* matrix) const
{
	assertValid();

	memset(matrix, 0, sizeof(float) * 16);

	matrix[0] = 2.0f * near / (right - left);
	matrix[2] = (right + left) / (right - left);
	matrix[5] = 2.0f * near / (top - bottom);
	matrix[6] = (top + bottom) / (top - bottom);
	matrix[10] = -(far + near) / (far - near);
	matrix[11] = -2.0f * (far * near) / (far - near);
	matrix[14] = -1;
}

void Frustum::computeOrtho(float* matrix) const
{
	assertValid();

	memset(matrix, 0, sizeof(float) * 16);

	matrix[0] = 2.0f / (right - left);
	matrix[3] = -(right + left) / (right - left);
	matrix[5] = 2.0f / (top - bottom);
	matrix[7] = -(top + bottom) / (top - bottom);
	matrix[10] = 2.0f / (far - near);
	matrix[11] = -(far + near) / (far - near);
	matrix[15] = -1;
}

void Frustum::computeVertex(Vec3f* vertex) const
{
	float halfHeight = near * tanf(fov() * (Mathf::cPi() / 360));
	float halfWidth = halfHeight * aspectRatio();

	vertex[0] = Vec3f(-halfWidth, -halfHeight, -near);	// Near left-bottom
	vertex[1] = Vec3f( halfWidth, -halfHeight, -near);	// Near right-bottom
	vertex[2] = Vec3f( halfWidth,  halfHeight, -near);	// Near right-top
	vertex[3] = Vec3f(-halfWidth,  halfHeight, -near);	// Near left-top

	halfHeight *= far / near;
	halfWidth *= far / near;

	vertex[4] = Vec3f(-halfWidth, -halfHeight, -far);	// Far left-bottom
	vertex[5] = Vec3f( halfWidth, -halfHeight, -far);	// Far right-bottom
	vertex[6] = Vec3f( halfWidth,  halfHeight, -far);	// Far right-top
	vertex[7] = Vec3f(-halfWidth,  halfHeight, -far);	// Far left-top
}

float Frustum::fov() const
{
	return atanf((top - bottom) / 2 / near) * (360.0f / Mathf::cPi());
}

float Frustum::aspectRatio() const
{
	return (right - left) / (top - bottom);
}

void Frustum::assertValid() const
{
	MCD_ASSERT(near > 0);
	MCD_ASSERT(far > near);
	MCD_ASSERT(right > left);
	MCD_ASSERT(top > bottom);
}

}	// namespace MCD
