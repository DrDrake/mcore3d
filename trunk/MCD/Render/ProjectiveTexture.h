#ifndef __MCD_RENDER_PROJECTIVETEXTURE__
#define __MCD_RENDER_PROJECTIVETEXTURE__

#include "Camera.h"
#include "Frustum.h"
#include "../Core/System/IntrusivePtr.h"

namespace MCD {

class Texture;
typedef IntrusivePtr<Texture> TexturePtr;

class MCD_RENDER_API ProjectiveTexture
{
public:
	/*!
		\param textureUnit GL_TEXTURE0 - GL_TEXTUREn
		\param textureMatrix Get the texture matrix computed (in row major).
		\note NO wrapping parameter GL_TEXTURE_WRAP_S/T/R will be applied.
	 */
	void bind(int textureUnit, sal_out_ecount_opt(16) float* textureMatrix = nullptr) const;
	void unbind() const;

	TexturePtr texture;
	Frustum frustum;
	Camera camera;
};	// ProjetiveTexture

}	// namespace MCD

#endif	// __MCD_RENDER_PROJECTIVETEXTURE__
