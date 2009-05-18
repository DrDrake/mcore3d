#ifndef __MCD_RENDER_TEXTURE__
#define __MCD_RENDER_TEXTURE__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/Resource.h"

namespace MCD {

class MCD_RENDER_API Texture : public Resource, Noncopyable
{
public:
	explicit Texture(const Path& fileId);

// Operations
	//! Bind this texture to the current texture unit.
	void bind() const;

	//! Unbind ANY texture of the current texture unit.
	void unbind() const;

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
	int format;

	//! The texture is invalid before the async resource is loaded.
	bool isValid() const;

	//! Check whether the format has an alpha channel or not.
	static bool hasAlpha(int format);

	/*!	Tells the byte-per-pixels of the input OpenGL format.
		Returns 0 if format is not reconized.
	 */
	static int bytePerPixel(int format);

protected:
	sal_override ~Texture();
};	// Texture

typedef IntrusivePtr<Texture> TexturePtr;

}	// namespace MCD

#endif	// __MCD_RENDER_TEXTURE__
