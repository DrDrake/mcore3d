#ifndef __MCD_LOADER_PVRLOADER__
#define __MCD_LOADER_PVRLOADER__

#include "TextureLoaderBase.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

class IResourceManager;

/*!	Loader for PowerVR's .pvr texture format.
	Some codes are borrowed from PVRTTextureAPI.cpp in the PowerVR SDK
 */
class MCD_LOADER_API PvrLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	PvrLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

protected:
	sal_override void uploadData(Texture& texture);
};	// PvrLoader

class MCD_LOADER_API PvrLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoaderPtr createLoader();
};	// PvrLoaderFactory

}	// namespace MCD

#endif	// __MCD_LOADER_PVRLOADER__
