#ifndef __MCD_RENDER_FRUSTUM__
#define __MCD_RENDER_FRUSTUM__

#include "ShareLib.h"
#include "../Core/System/Platform.h"

namespace MCD {

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

/*!	Defines a view frustum.
	The view frustum can be defined by six plane which where:
	left, right, bottom, top, near and far. You can interpret those
	six plans as perspective / orthogonal projection at the point
	you want to calculate the projection matrix using computePerspective()
	or computeOrtho() function.
 */
class MCD_RENDER_API Frustum
{
public:
	enum ProjectionType
	{
		Perspective,
		Ortho
	};

	/*!	By default the projection type is perspective.
		With parameters:
			fovy = 60
			aspect = 4/3
			near = 1
			far = 500
	 */
	Frustum();

// Operations
	/*!	Define the frustum matrix from parameters: field of view, aspect ratio
		and the near/far clip plane.
		\param fovy Field of view in y-direction (in degree).
		\param aspect Aspect ratio (width over height).
		\param near The near clip plane.
		\param near The far clip plane.
		\note The same as the gluPerspective() function.
	 */
	void create(float fovy, float aspect, float near, float far);

	/*!	
		\note The same as the glFrustum() function.
	 */
	void create(float left, float right, float bottom, float top, float near, float far);

	/*!	Compute the projection matrix.
		The matrix may be perspective or ortho, depending on the member variable \em projectionType.
		The matrix format should be the same as in opengl.
		\sa computePerspective
		\sa computeOrtho
	 */
	void computeProjection(sal_out_ecount(16) float* matrix) const;

	/*!	Apply the projection transform.
		\note Make sure the current matrix mode is GL_PROJECTION.
	 */
	void applyProjection() const;

	/*!	Compute the perspective projection matrix.
		The matrix format should be the same as in opengl.
		\sa http://www.opengl.org/sdk/docs/man/xhtml/glFrustum.xml
		\sa http://www.webkinesia.com/online/graphics/notes/viewing2.php
	 */
	void computePerspective(sal_out_ecount(16) float* matrix) const;

	/*!	Compute the orthographic parallel projection matrix.
		The matrix format should be the same as in opengl.
		\sa http://www.opengl.org/sdk/docs/man/xhtml/glOrtho.xml
		\sa http://www.webkinesia.com/online/graphics/notes/viewing2.php
	 */
	void computeOrtho(sal_out_ecount(16) float* matrix) const;

	/*!	Compute the 8 vertex of the frustum.
		The order of the vertex are:
		Near left-bottom, right-bottom, right-top, left-top;
		Far left-bottom, right-bottom, right-top, left-top;
	 */
	void computeVertex(sal_out_ecount(8) Vec3f* vertex) const;

// Attributes
	//! The field of view in degree.
	float fov() const;

	void setFov(float degree);

	//! Aspect ratio (width / height).
	float aspectRatio() const;

	void setAcpectRatio(float ratio);

	ProjectionType projectionType;
	float left, right, bottom, top, near, far;

protected:
	void assertValid() const;
};	// Frustum

}	// namespace MCD

#endif	// __MCD_RENDER_FRUSTUM__
