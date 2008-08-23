#include "Pch.h"
#include "Material.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

Material::Material()
	: mAmbient(0.5), mDiffuse(1), mSpecular(0), mShininess(0)
{
}

void Material::bind() const
{
	{	GLfloat ambient[] = { mAmbient.r, mAmbient.g, mAmbient.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	}

	{	GLfloat diffuse[] = { mDiffuse.r, mDiffuse.g, mDiffuse.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	}

	{	GLfloat specular[] = { mSpecular.r, mSpecular.g, mSpecular.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, mShininess);
	}

	if(mTexture) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		mTexture->bind();
	}
	else
		glBindTexture(GL_TEXTURE_2D, 0);
}

}	// namespace MCD
