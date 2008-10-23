#include "Pch.h"
#include "ProjectiveTexture.h"
#include "Texture.h"
#include "../../MCD/Core/Math/Mat44.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

// Reference: http://dalab.se.sjtu.edu.cn/~jietan/shadowMappingTutorial.html
// Reference: http://www.nvidia.com/object/Projective_Texture_Mapping.html

void ProjectiveTexture::bind(int textureUnit, float* outTextureMatrix) const
{
	if(!texture)// || !texture->isValid())
		return;

	glPushAttrib(GL_ALL_ATTRIB_BITS | GL_ENABLE_BIT | GL_TEXTURE_BIT);

	// Scale and translate by one-half to bring the coordinates from [-1, 1]
	// to the texture coordinate [0, 1]
	// Note that z component also need to be 0.5, otherwise shadow map will not work correctly
	Mat44f bias = Mat44f::cIdentity;
	bias.setTranslation(Vec3f(0.5f, 0.5f, 0.5f));
	bias.setScale(Vec3f(0.5f, 0.5f, 0.5f));

	Mat44f projection;
	frustum.computeProjection(projection.getPtr());

	Mat44f modelView;
	camera.computeTransform(modelView.getPtr());

	Mat44f textureMatrix = bias * projection * modelView;

	// Use another texture unit to avoid conflit with the diffuse texture of the model
	glActiveTexture(textureUnit);

	// The following code block is only relavent for fixed pipeline
	{
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);

		static const GLfloat sPlane[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
		static const GLfloat tPlane[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
		static const GLfloat rPlane[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
		static const GLfloat qPlane[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		// A post-multiply by the inverse of the current modelview matrix is
		// applied automatically to the eye plane equations we provided.
		glTexGenfv(GL_S, GL_EYE_PLANE, sPlane);
		glTexGenfv(GL_T, GL_EYE_PLANE, tPlane);
		glTexGenfv(GL_R, GL_EYE_PLANE, rPlane);
		glTexGenfv(GL_Q, GL_EYE_PLANE, qPlane);

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	}

	glMatrixMode(GL_TEXTURE);
	glLoadTransposeMatrixf(textureMatrix.getPtr());

	if(outTextureMatrix)
		textureMatrix.copyTo(outTextureMatrix);

	glMatrixMode(GL_MODELVIEW);
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
