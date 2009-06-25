#ifndef __MCD_RENDER_MODELPOD__
#define __MCD_RENDER_MODELPOD__

#include "ShareLib.h"
#include "Renderable.h"
#include "../Core/System/Resource.h"

namespace MCD {

class MCD_RENDER_API ModelPod : public Resource, public IRenderable, private Noncopyable
{
public:
	explicit ModelPod(const Path& fileId);

	sal_override ~ModelPod();

	sal_override void draw();

	class Impl;
	Impl* mImpl;
};	// ModelPod

typedef IntrusivePtr<ModelPod> ModelPodPtr;

}	// namespace MCD

#include "ShareLib.h"
#include "../Core/System/ResourceLoader.h"

namespace MCD {

class IResourceManager;

class MCD_RENDER_API PodLoader : public IResourceLoader, private Noncopyable
{
	class LoaderImpl;

public:
	explicit PodLoader(sal_maybenull IResourceManager* resourceManager = nullptr);

	sal_override ~PodLoader();

	//!	Load data from stream.
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr);

	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	class Impl;
	Impl* mImpl;
};	// PodLoader

}	// namespace MCD

#endif	// __MCD_RENDER_MODELPOD__
