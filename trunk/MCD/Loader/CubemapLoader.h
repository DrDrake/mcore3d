#ifndef __MCD_LOADER_CUBEMAPLOADER__
#define __MCD_LOADER_CUBEMAPLOADER__

#include "TextureLoaderBase.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

/*!	Load Loading a cubemap texture.
	The 6 faces of the cubemap data is assumed arranged vertically
	inside the incomming image data.
 */
class MCD_LOADER_API CubemapLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	CubemapLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

protected:
    sal_override void uploadData(Texture& texture);
};	// CubemapLoader

class MCD_LOADER_API CubemapLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoaderPtr createLoader();
};	// CubemapLoaderFactory

}	// namespace MCD

#endif	// __MCD_LOADER_CUBEMAPLOADER__
