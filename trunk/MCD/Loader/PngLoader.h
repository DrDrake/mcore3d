#ifndef __MCD_LOADER_PNGLOADER__
#define __MCD_LOADER_PNGLOADER__

#include "TextureLoaderBase.h"
#include "../Core/System/ResourceManager.h"

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

class MCD_LOADER_API PngLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoaderPtr createLoader();
};	// PngLoaderFactory

}	// namespace MCD

#endif	// __MCD_LOADER_PNGLOADER__
