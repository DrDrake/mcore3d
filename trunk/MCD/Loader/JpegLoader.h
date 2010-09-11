#ifndef __MCD_LOADER_JPEGLOADER__
#define __MCD_LOADER_JPEGLOADER__

#include "TextureLoaderBase.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

/*!	Load jpeg file, read line by line.
	\note Progressive jpeg file can be loaded but cannot be displayed progressively.
 */
class MCD_LOADER_API JpegLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	JpegLoader();

	/*!	Load data from stream.
		Returns when a single row of data (scan line) is read.
		Change state to PartialLoaded when every 1/4 of the total scan lines are loaded.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

protected:
	sal_override void uploadData(Texture& texture);
};	// JpegLoader

class MCD_LOADER_API JpegLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoaderPtr createLoader();
};	// JpegLoaderFactory

}	// namespace MCD

#endif	// __MCD_LOADER_JPEGLOADER__
