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

	size_t width() const {
		return mWidth;
	}

	size_t height() const {
		return mHeight;
	}

	uint handle() const {
		return mHandle;
	}

	//! Bind this texture to the current texture unit.
	void bind() const;

	//! Unbind ANY texture of the current texture unit.
	void unbind() const;

	/*!	Type of the texture.
		Can be GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB
	 */
	int type() const {
		return mType;
	}

	//! The texture is invalid before the async resource is loaded.
	bool isValid() const;

	/*!	Format of the texture.
		Use those appeared in OpenGl.
		Can be GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE and GL_LUMINANCE_ALPHA
	 */
	int format() const {
		return mFormat;
	}

	//! Check whether the format has an alpha channel or not.
	static bool hasAlpha(int format);

	/*!	User can define this class in their scope in order to modify the data member in Texture.
		Various texture loaders use this technique to modify the texture.
	 */
	template<class T> class PrivateAccessor;

protected:
	sal_override ~Texture();

protected:
	uint mHandle;
	size_t mWidth;
	size_t mHeight;
	int mType;
	int mFormat;
};	// Texture

typedef IntrusivePtr<Texture> TexturePtr;

}	// namespace MCD

#endif	// __MCD_RENDER_TEXTURE__
