#ifndef __MCD_LOADER_FNTLOADER__
#define __MCD_LOADER_FNTLOADER__

#include "ShareLib.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

class ResourceManager;
typedef IntrusivePtr<class BmpFont> BmpFontPtr;

/*!	Bitmap font loader for *.fnt files generated using AngelCode's bitmap font generator
	\ref http://www.angelcode.com/products/bmfont/
 */
class MCD_LOADER_API FntLoader : public IResourceLoader, private Noncopyable
{
public:
	FntLoader();

	/*!	Load data from stream.
		Block until the whole font file is loaded.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type BmpFont.
	 */
	sal_override void commit(Resource& resource);

protected:
	BmpFontPtr mTmp;
	sal_maybenull ResourceManager* mResourceManager;
};	// FntLoader

class Entity;

class MCD_LOADER_API FntLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoaderPtr createLoader();
};	// FntLoaderFactory

}	// namespace MCD

#endif	// __MCD_LOADER_FNTLOADER__
