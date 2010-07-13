#ifndef __MCD_LOADER_FNTLOADER__
#define __MCD_LOADER_FNTLOADER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/ResourceLoader.h"

namespace MCD {

class IResourceManager;
typedef IntrusivePtr<class BmpFont> BmpFontPtr;

/*!	Bitmap font loader for *.fnt files generated using AngelCode's bitmap font generator
	\ref http://www.angelcode.com/products/bmfont/
 */
class MCD_LOADER_API FntLoader : public IResourceLoader, private Noncopyable
{
public:
	explicit FntLoader(sal_maybenull IResourceManager* resourceManager = nullptr);

	/*!	Load data from stream.
		Block until the whole font file is loaded.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type BmpFont.
	 */
	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	LoadingState mLoadingState;
	BmpFontPtr mTmp;
	sal_maybenull IResourceManager* mResourceManager;
};	// FntLoader

}	// namespace MCD

#endif	// __MCD_LOADER_FNTLOADER__
