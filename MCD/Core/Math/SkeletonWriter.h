#ifndef __MCD_CORE_MATH_SKELETONWRITER__
#define __MCD_CORE_MATH_SKELETONWRITER__

#include "../ShareLib.h"
#include "../System/Platform.h"
#include <iosfwd>

namespace MCD {

class Skeleton;

/*!	A very simple writer that dump the Skeleton into the output stream.
 */
class MCD_CORE_API SkeletonWriter
{
public:
	static sal_checkreturn bool write(std::ostream& os, const Skeleton& skeleton);
};	// SkeletonWriter

}	// namespace MCD

#endif	// __MCD_CORE_MATH_SKELETONWRITER__
