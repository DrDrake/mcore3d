#ifndef __MCD_RENDER_FRUSTUM__
#define __MCD_RENDER_FRUSTUM__

#include "ShareLib.h"

namespace MCD {

/*!	Defines a view frustum.
	The view frustum can be defined by six plane which where:
	left, right, bottom, top, near and far. You can interpret those
	six plans as perspective / orthogonal projection at the point
	you want to calculte the projection matrix using computePerspective()
	or computeOrtho() function.
 */
class MCD_RENDER_API Frustum
{
public:
	Frustum() {}

	/*!	Define the frustum matrix from parameters: field of view, aspect ratio
		and the near/far clip plane.
		\param fovy Field of view in y-direction (radian).
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

	//! The field of view in radian.
	float fov() const;

	//! Aspect ratio (width / height).
	float aspectRatio() const;

	float left, right, bottom, top, near, far;

protected:
	void assertValid() const;
};	// Frustum

}	// namespace MCD

#endif	// __MCD_RENDER_FRUSTUM__
