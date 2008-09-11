#ifndef __MCD_RENDERTEST_COMMON__
#define __MCD_RENDERTEST_COMMON__

/*!	Draw a screen aligned quad.
	\param textureType Type of the texture, can be GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB etc
 */
void drawViewportQuad(size_t x, size_t y, size_t width, size_t height, int textureType);

/*!	Draw an unit cube with the 6 faces appling the same texture.
 */
void drawUnitCube(float textureCoord1, float textureCoord2);

#endif	// __MCD_RENDERTEST_COMMON__
