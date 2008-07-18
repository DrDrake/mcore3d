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

	//! Bind this texture to the current rendering system.
	void bind();

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
	int mFormat;
};	// Texture

}	// namespace MCD

#endif	// __MCD_RENDER_TEXTURE__
