#include "Pch.h"
#include "ProjectiveTexture.h"
#include "Texture.h"
#include "../../MCD/Core/Math/Mat44.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

// Reference: http://dalab.se.sjtu.edu.cn/~jietan/shadowMappingTutorial.html
void ProjectiveTexture::bind(int textureUnit) const
{
	if(!texture)// || !texture->isValid())
		return;

	glPushAttrib(GL_ALL_ATTRIB_BITS | GL_ENABLE_BIT | GL_TEXTURE_BIT);

	Mat44f tmp = Mat44f::cIdentity;

	// Scale and translate by one-half to bring the coordinates from [-1, 1]
	// to the texture coordinate [0, 1]
	// Note that z component also need to be 0.5, otherwise shadow map will not work correctly
	tmp.setTranslation(Vec3f(0.5f, 0.5f, 0.5f));
	tmp.setScale(Vec3f(0.5f, 0.5f, 0.5f));

	Mat44f projection;
	frustum.computeProjection(projection.getPtr());

	Mat44f modelView;
	camera.computeTransform(modelView.getPtr());
	tmp = tmp * projection * modelView;

	// Use another texture unit to avoid conflit with the diffuse texture of the model
	glActiveTexture(textureUnit);
	glEnable(texture->type());

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);

	// A post-multiply by the inverse of the current modelview matrix is
	// applied automatically to the eye plane equations we provided.
	glTexGenfv(GL_S, GL_EYE_PLANE, &tmp[0][0]);
	glTexGenfv(GL_T, GL_EYE_PLANE, &tmp[1][0]);
	glTexGenfv(GL_R, GL_EYE_PLANE, &tmp[2][0]);
	glTexGenfv(GL_Q, GL_EYE_PLANE, &tmp[3][0]);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	texture->bind();
}

void ProjectiveTexture::unbind() const
{
	if(!texture)// || !texture->isValid())
		return;

	texture->unbind();
	glPopAttrib();
}

}	// namespace MCD
