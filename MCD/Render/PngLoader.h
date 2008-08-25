#ifndef __MCD_RENDER_PNGLOADER__
#define __MCD_RENDER_PNGLOADER__

#include "TextureLoaderBase.h"

namespace MCD {

/*!	Load png file.
	Progressive png is supported.
	\note Png that use color palette is not supported.
 */
class MCD_RENDER_API PngLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	PngLoader();

	/*!	Load data from stream.
		Returns when a single row of data is read (for interlaced png).
		Change state to PartialLoaded when a pass is loaded.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr);

protected:
	sal_override void uploadData();
};	// PngLoader

}	// namespace MCD

#endif	// __MCD_RENDER_PNGLOADER__
