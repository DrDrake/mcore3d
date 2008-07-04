#ifndef __SGE_RENDER_TEXTURELOADERBASE__
#define __SGE_RENDER_TEXTURELOADERBASE__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/ResourceLoader.h"

namespace SGE {

/*!	Common base class for texture loaders.
	What this base class provided are:
		1. Generating OpenGl texture object id.
		2. Handles the flow inside commit() according to the current loading state.
		3. Proper lock/unlocking of mutex inside commit() and getLoadingState().
	See JpegLoader.cpp for a typical usage of TextureLoaderBase.
 */
class SGE_RENDER_API SGE_NOVTABLE TextureLoaderBase : public IResourceLoader, private Noncopyable
{
protected:
	class LoaderBaseImpl;

	TextureLoaderBase();
	sal_override ~TextureLoaderBase();

	void setImpl(LoaderBaseImpl* impl);

public:
	sal_override void commit(Resource& resource);
	sal_override LoadingState getLoadingState() const;

protected:
	/*!	Derived class should implements this function to upload the
		image data to the rendering system.
		\note mImpl->mMutex is locked before the invocation of this function.
	 */
	virtual void uploadData() = 0;

protected:
	template<class T> class PrivateAccessor;

	LoaderBaseImpl* mImpl;
	volatile LoadingState mLoadingState;
};	// TextureLoaderBase

}	// namespace SGE

#endif	// __SGE_RENDER_TEXTURELOADERBASE__
