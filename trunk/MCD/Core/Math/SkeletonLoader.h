#ifndef __MCD_CORE_MATH_SKELETONLOADER__
#define __MCD_CORE_MATH_SKELETONLOADER__

#include "../ShareLib.h"
#include "../System/NonCopyable.h"
#include "../System/ResourceLoader.h"

namespace MCD {

class IResourceManager;

/*!	Loader for the MCore's .skt file format.
 */
class MCD_CORE_API SkeletonLoader : public IResourceLoader, private Noncopyable
{
public:
	SkeletonLoader();

	sal_override ~SkeletonLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const wchar_t* args=nullptr);

	/*!	Commit the data form it's internal buffer to the resource.
		The resource must be of type Skeleton.
	 */
	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	class Impl;
	Impl& mImpl;
};	// SkeletonLoader

}	// namespace MCD

#endif	// __MCD_CORE_MATH_SKELETONLOADER__
