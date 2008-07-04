#ifndef __MCD_RENDER_JPEGLOADER__
#define __MCD_RENDER_JPEGLOADER__

#include "TextureLoaderBase.h"

namespace MCD {

/*!	Load jpeg file, read line by line.
	\note Progressive jpeg file can be loaded but cannot be displayed progressively.
 */
class MCD_RENDER_API JpegLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	JpegLoader();

	/*!	Load data from stream.
		Returns when a single row of data (scan line) is read.
		Change state to PartialLoaded when a scan line is loaded.
	 */
	sal_override LoadingState load(sal_maybenull std::istream* is);

protected:
	sal_override void uploadData();
};	// JpegLoader

}	// namespace MCD

#endif	// __MCD_RENDER_JPEGLOADER__
