// To be included in cpp only.
// See also TextureLoaderBase.h

#include "TextureLoaderBase.h"
#include "../Core/System/Mutex.h"

namespace MCD {

class MCD_RENDER_API TextureLoaderBase::LoaderBaseImpl
{
public:
	LoaderBaseImpl(TextureLoaderBase& loader);
	virtual ~LoaderBaseImpl();

	TextureLoaderBase& mLoader;
	byte_t* mImageData;

	size_t mWidth;
	size_t mHeight;
	int mFormat;

	Mutex mMutex;
};	// LoaderBaseImpl

}	// namespace MCD
