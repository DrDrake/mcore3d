#ifndef __MCD_RENDER_TGALOADER__
#define __MCD_RENDER_TGALOADER__

#include "TextureLoaderBase.h"

namespace MCD {

/*!	Load tga file.
 */
class MCD_RENDER_API TgaLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	TgaLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr);

protected:
	sal_override void uploadData();
};	// TgaLoader

}	// namespace MCD

#endif	// __MCD_RENDER_TGALOADER__
