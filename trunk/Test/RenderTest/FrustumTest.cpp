#include "Pch.h"
#include "../../MCD/Core/Math/Mat44.h"
#include "../../MCD/Render/Frustum.h"

using namespace MCD;

TEST(FrustumTest)
{
	BasicGlWindow window(L"show=0");

	{	// Perspective transform
		Mat44f m1;
		Frustum f;
		f.projectionType = Frustum::Perspective;
		f.create(40, 4.0f/3, 1, 50);
		f.computePerspective(m1.getPtr());
		m1 = m1.transpose();

		Mat44f m2;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(40, 4.0f/3, 1, 50);
		glGetFloatv(GL_PROJECTION_MATRIX, m2.getPtr());

		CHECK(m1.isNearEqual(m2));
	}

	{	// Orthogonal transform
		Mat44f m1;
		Frustum f;
		f.projectionType = Frustum::Ortho;
		f.create(-10, 10, -10, 10, -10, 10);
		f.computeOrtho(m1.getPtr());
		m1 = m1.transpose();

		Mat44f m2;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-10, 10, -10, 10, -10, 10);
		glGetFloatv(GL_PROJECTION_MATRIX, m2.getPtr());

		CHECK(m1.isNearEqual(m2));
	}
}
