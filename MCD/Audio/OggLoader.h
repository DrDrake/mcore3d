#ifndef __MCD_AUDIO_OGGLOADER__
#define __MCD_AUDIO_OGGLOADER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/ResourceLoader.h"

namespace MCD {

class MCD_AUDIO_API OggLoader : public IResourceLoader, private Noncopyable
{
public:
	OggLoader();

	sal_override ~OggLoader();

// Operations
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const wchar_t* args=nullptr);

	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

// Attributes
	volatile LoadingState loadingState;

protected:
	class Impl;
	Impl* mImpl;
};	// OggLoader

}	// namespace MCD

#endif	// __MCD_AUDIO_OGGLOADER__
