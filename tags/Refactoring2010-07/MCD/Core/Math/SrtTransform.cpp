#include "Pch.h"
#include "SrtTransform.h"

namespace MCD {

SrtTransform::SrtTransform(const Mat44f& m)
	:
	translation(m.translation())
{
	scale.x = Vec3f(m.m00, m.m10, m.m20).length();
	scale.y = Vec3f(m.m01, m.m11, m.m21).length();
	scale.z = Vec3f(m.m02, m.m12, m.m22).length();

	// Undo the effect of scaling on the 3x3 matrix
//	rotation = m.mat33();
//	for(size_t i=0; i<3; ++i) for(size_t j=0; j<3; ++j)
//		rotation[i][j] /= scale[j];
}

}	// namespace MCD
