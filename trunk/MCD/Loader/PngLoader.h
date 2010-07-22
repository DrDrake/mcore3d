#ifndef __MCD_LOADER_PNGLOADER__
#define __MCD_LOADER_PNGLOADER__

#include "TextureLoaderBase.h"

namespace MCD {

/*!	Load png file.
	Progressive png is supported.
	\note Png that use color palette is not supported.
 */
class MCD_LOADER_API PngLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	PngLoader();

	/*!	Load data from stream.
		Returns when a single row of data is read (for interlaced png).
		Change state to PartialLoaded when a pass is loaded.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

protected:
	sal_override void uploadData(Texture& texture);
};	// PngLoader

}	// namespace MCD

#endif	// __MCD_LOADER_PNGLOADER__