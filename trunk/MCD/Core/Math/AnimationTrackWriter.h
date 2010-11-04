#ifndef __MCD_CORE_MATH_ANIMATIONTRACKWRITER__
#define __MCD_CORE_MATH_ANIMATIONTRACKWRITER__

#include "../ShareLib.h"
#include "../System/Platform.h"
#include <iosfwd>

namespace MCD {

class AnimationClip;

/*!	A very simple writer that dump the AnimationClip into the output stream.
 */
class MCD_CORE_API AnimationClipWriter
{
public:
	static sal_checkreturn bool write(std::ostream& os, const AnimationClip& track);
};	// AnimationClipWriter

}	// namespace MCD

#endif	// __MCD_CORE_MATH_ANIMATIONTRACKWRITER__
