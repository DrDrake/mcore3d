#ifndef __MCD_LOADER_BITMAPLOADER__
#define __MCD_LOADER_BITMAPLOADER__

#include "TextureLoaderBase.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

/*!	Load bitmap file.
 */
class MCD_LOADER_API BitmapLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	BitmapLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

protected:
	sal_override void uploadData(Texture& texture);
};	// BitmapLoader

class MCD_LOADER_API BitmapLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoaderPtr createLoader();
};	// BitmapLoaderFactory

}	// namespace MCD

#endif	// __MCD_LOADER_BITMAPLOADER__
