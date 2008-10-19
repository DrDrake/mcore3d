#ifndef __MCD_RENDERTEST_COMMON__
#define __MCD_RENDERTEST_COMMON__

namespace MCD {

/*!	Draw a screen aligned quad.
	\param textureType Type of the texture, can be GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB etc
 */
void drawViewportQuad(size_t x, size_t y, size_t width, size_t height, int textureType);

/*!	Draw an unit cube with the 6 faces appling the same texture.
 */
void drawUnitCube(float textureCoord1, float textureCoord2);

class Frustum;

//!	Debug draw of the frustum.
void drawFrustum(const Frustum& frustum);

}	// namespace MCD

#endif	// __MCD_RENDERTEST_COMMON__
