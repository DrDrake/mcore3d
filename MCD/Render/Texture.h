#ifndef __MCD_RENDER_TEXTURE__
#define __MCD_RENDER_TEXTURE__

#include "GpuDataFormat.h"
#include "../Core/System/Resource.h"

namespace MCD {

struct GpuDataFormat;

class MCD_RENDER_API Texture : public Resource
{
public:
	explicit Texture(const Path& fileId);

// Operations
	//! Bind this texture to the specified texture unit.
	void bind(size_t textureUnit=0) const;

	//! Unbind texture, the textureUnit should match the one passed in bind().
	void unbind(size_t textureUnit=0) const;

// Attributes
	uint handle;

	size_t width;

	size_t height;

	/*!	Type of the texture.
		Can be GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB
	 */
	int type;

	/*!	Format of the texture.
		Use those appeared in OpenGl.
		Can be GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE and GL_LUMINANCE_ALPHA
	 */
//	int format;

	GpuDataFormat format;

	//! The texture is invalid before the async resource is loaded.
	bool isValid() const;

	//! Check whether the format has an alpha channel or not.
	static bool hasAlpha(int format);

// Operations
	void clear();

	/*!	The loop order of the image data is assumed to be:
		for each surface: for each mip-map level
	 */
	// TODO: Support texture array, cube map etc.
	sal_checkreturn bool create(
		const GpuDataFormat& gpuFormat,
		const GpuDataFormat& srcFormat,
		size_t width, size_t height,
		size_t surfaceCount, size_t mipLevelCount,
		sal_maybenull sal_in_ecount(dataSize) const char* data, size_t dataSize,
		int apiSpecificflags = 0
	);

protected:
	sal_override ~Texture();
};	// Texture

typedef IntrusivePtr<Texture> TexturePtr;
typedef IntrusiveWeakPtr<Texture> TextureWeakPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_TEXTURE__
