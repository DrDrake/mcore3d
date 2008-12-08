#include "Pch.h"
#include "Material.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

Material::Material()
	: ambient(0.5), diffuse(1), specular(0), shininess(0)
{
}

// The destructor is implemented in cpp file so that Material.h need not to 
// include Texture.h because we have a TexturePtr as member variable
Material::~Material()
{
}

void Material::bind() const
{
	{	GLfloat a[] = { ambient.r, ambient.g, ambient.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, a);
	}

	{	GLfloat d[] = { diffuse.r, diffuse.g, diffuse.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, d);
	}

	{	GLfloat s[] = { specular.r, specular.g, specular.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	}

	if(texture) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture->bind();
	}
	else
		glBindTexture(GL_TEXTURE_2D, 0);
}

}	// namespace MCD
