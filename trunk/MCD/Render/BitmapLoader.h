#ifndef __MCD_RENDER_BITMAPLOADER__
#define __MCD_RENDER_BITMAPLOADER__

#include "TextureLoaderBase.h"

namespace MCD {

/*!	Load bitmap file.
 */
class MCD_RENDER_API BitmapLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	BitmapLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(sal_maybenull std::istream* is);

protected:
	sal_override void uploadData();
};	// BitmapLoader

}	// namespace MCD

#endif	// __MCD_RENDER_BITMAPLOADER__
