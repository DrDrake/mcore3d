#ifndef __MCD_RENDER_PROJECTIVETEXTURE__
#define __MCD_RENDER_PROJECTIVETEXTURE__

#include "Camera.h"
#include "../Core/System/IntrusivePtr.h"

namespace MCD {

typedef IntrusivePtr<class Texture> TexturePtr;

class MCD_RENDER_API ProjectiveTexture
{
public:
	/*!	Generate texture matrix for the given texture unit.
		\param textureUnit GL_TEXTURE0 - GL_TEXTUREn

		\param postMultipleMatrix
			Post multiple the texture matrix with this one (in row major), useful in
			conjunction with glsl; where in the vertex shader you generate the coordinate
			using "coord = gl_TextureMatrix[textureUnit] * gl_ModelViewMatrix * gl_Vertex;".
			To cancel out gl_ModelViewMatrix into the model matrix only, you would set
			postMultipleMatrix as the inverse of the camera view matrix.

		\param textureMatrix Get the texture matrix computed (in row major).

		\sa http://www.nvidia.com/object/Projective_Texture_Mapping.html
		\sa http://dalab.se.sjtu.edu.cn/~jietan/shadowMappingTutorial.html

		\note
			NO wrapping parameter GL_TEXTURE_WRAP_S/T/R will be applied. The wrapping
			parameter is better to set once for the texture rather than every bind().
	 */
	void bind(int textureUnit,
		sal_in_ecount_opt(16) const float* postMultipleMatrix = nullptr,
		sal_out_ecount_opt(16) float* textureMatrix = nullptr) const;

	void unbind() const;

	TexturePtr texture;
	Camera camera;
};	// ProjetiveTexture

}	// namespace MCD

#endif	// __MCD_RENDER_PROJECTIVETEXTURE__
