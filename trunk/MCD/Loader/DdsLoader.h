#ifndef __MCD_RENDER_DDSLOADER__
#define __MCD_RENDER_DDSLOADER__

#include "TextureLoaderBase.h"

namespace MCD {

/*!	Load dds file.
	This loader support background loading but not progressive loading.
 */
class MCD_LOADER_API DdsLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	DdsLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

protected:
	sal_override void uploadData(Texture& texture);
};	// DdsLoader

}	// namespace MCD

#endif	// __MCD_RENDER_DDSLOADER__