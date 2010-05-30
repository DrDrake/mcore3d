// To be included in cpp only.
// See also TextureLoaderBase.h

#include "TextureLoaderBase.h"
#include "GpuDataFormat.h"
#include "../Core/System/Mutex.h"

namespace MCD {

class MCD_RENDER_API TextureLoaderBase::LoaderBaseImpl
{
public:
	LoaderBaseImpl(TextureLoaderBase& loader);
	virtual ~LoaderBaseImpl();

	TextureLoaderBase& mLoader;
	byte_t* mImageData; /*!< byte array which holds the pixels */

	size_t mWidth;		/*!< width of the image */
	size_t mHeight;		/*!< height of the image */
	int mFormat;		/*!< describes which color-component exists in the mImageData */
	GpuDataFormat mGpuFormat;

	Mutex mMutex;
};	// LoaderBaseImpl

}	// namespace MCD
