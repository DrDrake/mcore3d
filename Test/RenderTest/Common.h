#ifndef __MCD_RENDERTEST_COMMON__
#define __MCD_RENDERTEST_COMMON__

// Forward declarations of some class in MCD
namespace MCD {

class Frustum;
class ShaderProgram;

}	// namespace MCD

/*!	Draw a screen aligned quad.
	\param textureType Type of the texture, can be GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB etc
 */
void drawViewportQuad(size_t x, size_t y, size_t width, size_t height, int textureType);

/*!	Draw an unit cube with the 6 faces appling the same texture.
 */
void drawUnitCube(float textureCoord1, float textureCoord2);

//!	Debug draw of the frustum.
void drawFrustum(const MCD::Frustum& frustum);

/*!	Make the random unit sphere random vector more uniform by minimize the energy between 
	those vectors.
	\ref http://www.malmer.nu/index.php/tag/ssao/
 */
void minizeEnergy(float* data, size_t sampleCount, size_t stride=sizeof(float)*3);

class DefaultResourceManager;

//! Load the shaders synchronously
bool loadShaderProgram(
	const wchar_t* vsSource, const wchar_t* psSource,
	MCD::ShaderProgram& shaderProgram,
	DefaultResourceManager& resourceManager);

#endif	// __MCD_RENDERTEST_COMMON__
