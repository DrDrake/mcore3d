#include "Pch.h"
#include "Frustum.h"
#include "../Core/Math/BasicFunction.h"
#include "../Core/Math/Mat44.h"
#include "../../3Party/glew/glew.h"
#include <memory.h> // For memset

namespace MCD {

Frustum::Frustum()
{
	projectionType = Perspective;
	create(45, 4.0f/3, 0.1f, 500);
}

void Frustum::create(float fovy, float aspect, float n, float f)
{
	if(fovy <= 0 || aspect <= 0)
		return;

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

void Frustum::computeProjection(float* matrix) const
{
	if(projectionType == Perspective)
		computePerspective(matrix);
	else if(projectionType == Ortho)
		computeOrtho(matrix);
	else {
		MCD_ASSERT(false);
	}
}

void Frustum::applyProjection() const
{
	float mat[16];
	computeProjection(mat);

	// The same can be acheived using gluPerspective()
//	gluPerspective(fov(), aspectRatio(), near, far);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(mat);
	glMatrixMode(GL_MODELVIEW);
}

// Reference:
// http://www.codeguru.com/cpp/misc/misc/math/article.php/c10123__3/Deriving-Projection-Matrices.htm
void Frustum::computePerspective(float* matrix) const
{
	assertValid();

	memset(matrix, 0, sizeof(float) * 16);
	Mat44f& m = *reinterpret_cast<Mat44f*>(matrix);

	m.m00 = 2.0f * near / (right - left);
	m.m02 = (right + left) / (right - left);
	m.m11 = 2.0f * near / (top - bottom);
	m.m12 = (top + bottom) / (top - bottom);
	m.m22 = -(far + near) / (far - near);
	m.m23 = -2.0f * (far * near) / (far - near);
	m.m32 = -1;

	// TODO: To verify which one is correct
/*	m.m00 = 2.0f * near / (right - left);
	m.m02 = -(right + left) / (right - left);
	m.m11 = 2.0f * near / (top - bottom);
	m.m12 = -(top + bottom) / (top - bottom);
	m.m22 = (near) / (far - near);
	m.m23 = -(far * near) / (far - near);
	m.m32 = -1;*/
}

// Correct formula:
// http://www.codeguru.com/cpp/misc/misc/math/article.php/c10123__2/
// Seems incorrect formulas:
// http://www.songho.ca/opengl/gl_projectionmatrix.html
// http://en.wikipedia.org/wiki/Orthographic_projection_%28geometry%29 (even wikipedia is incorrect?)
void Frustum::computeOrtho(float* matrix) const
{
	assertValid();

	memset(matrix, 0, sizeof(float) * 16);
	Mat44f& m = *reinterpret_cast<Mat44f*>(matrix);

	m.m00 = 2.0f / (right - left);
	m.m03 = -(right + left) / (right - left);
	m.m11 = 2.0f / (top - bottom);
	m.m13 = -(top + bottom) / (top - bottom);
	m.m22 = -1.0f / (far - near);
	m.m23 = -near / (far - near);
	m.m33 = 1;
}

void Frustum::computeVertex(Vec3f* vertex) const
{
	float halfHeight = (top - bottom) / 2;
	float halfWidth = (right - left) / 2;

	vertex[0] = Vec3f(-halfWidth, -halfHeight, -near);	// Near left-bottom
	vertex[1] = Vec3f( halfWidth, -halfHeight, -near);	// Near right-bottom
	vertex[2] = Vec3f( halfWidth,  halfHeight, -near);	// Near right-top
	vertex[3] = Vec3f(-halfWidth,  halfHeight, -near);	// Near left-top

	if(projectionType == Perspective) {
		halfHeight *= far / near;
		halfWidth *= far / near;
	}

	vertex[4] = Vec3f(-halfWidth, -halfHeight, -far);	// Far left-bottom
	vertex[5] = Vec3f( halfWidth, -halfHeight, -far);	// Far right-bottom
	vertex[6] = Vec3f( halfWidth,  halfHeight, -far);	// Far right-top
	vertex[7] = Vec3f(-halfWidth,  halfHeight, -far);	// Far left-top
}

float Frustum::fov() const
{
	return atanf((top - bottom) / 2 / near) * (360.0f / Mathf::cPi());
}

void Frustum::setFov(float degree)
{
	create(degree, aspectRatio(), near, far);
}

float Frustum::aspectRatio() const
{
	return (right - left) / (top - bottom);
}

void Frustum::setAcpectRatio(float ratio)
{
	MCD_ASSERT(projectionType == Perspective);
	create(fov(), ratio, near, far);
}

void Frustum::assertValid() const
{
	if(projectionType == Perspective)
		MCD_ASSERT(near > 0);

	MCD_ASSERT(far > near);
	MCD_ASSERT(right > left);
	MCD_ASSERT(top > bottom);
}

}	// namespace MCD
