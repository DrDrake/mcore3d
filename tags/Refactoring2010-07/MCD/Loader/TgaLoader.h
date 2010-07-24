#ifndef __MCD_LOADER_TGALOADER__
#define __MCD_LOADER_TGALOADER__

#include "TextureLoaderBase.h"

namespace MCD {

/*!	Load tga file.
 */
class MCD_LOADER_API TgaLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	TgaLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

protected:
	sal_override void uploadData(Texture& texture);
};	// TgaLoader

}	// namespace MCD

#endif	// __MCD_LOADER_TGALOADER__
