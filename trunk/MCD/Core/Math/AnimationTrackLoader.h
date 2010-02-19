#ifndef __MCD_CORE_ANIMATIONTRACKLOADER__
#define __MCD_CORE_ANIMATIONTRACKLOADER__

#include "../ShareLib.h"
#include "../System/NonCopyable.h"
#include "../System/ResourceLoader.h"

namespace MCD {

class IResourceManager;

/*!	Loader for the Ogre 's .mesh file format.
 */
class MCD_CORE_API AnimationTrackLoader : public IResourceLoader, private Noncopyable
{
public:
	AnimationTrackLoader();

	sal_override ~AnimationTrackLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const wchar_t* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type Model.
	 */
	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	class Impl;
	Impl& mImpl;
};	// AnimationTrackLoader

}	// namespace MCD

#endif	// __MCD_CORE_ANIMATIONTRACKLOADER__
