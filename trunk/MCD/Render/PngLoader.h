#ifndef __SGE_RENDER_PNGLOADER__
#define __SGE_RENDER_PNGLOADER__

#include "TextureLoaderBase.h"

namespace SGE {

/*!	Load png file.
	Progressive png is supported.
	\note Png that use color palette is not supported.
 */
class SGE_RENDER_API PngLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	PngLoader();

	/*!	Load data from stream.
		Returns when a single row of data is read (for interlaced png).
		Change state to PartialLoaded when a pass is loaded.
	 */
	sal_override LoadingState load(sal_maybenull std::istream* is);

protected:
	sal_override void uploadData();
};	// PngLoader

}	// namespace SGE

#endif	// __SGE_RENDER_PNGLOADER__
