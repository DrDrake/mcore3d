#ifndef __MCD_CORE_ANIMATIONTRACKWRITER__
#define __MCD_CORE_ANIMATIONTRACKWRITER__

#include "../ShareLib.h"
#include "../System/Platform.h"
#include <iosfwd>

namespace MCD {

class AnimationTrack;

/*!	A very simple writer that dump the AnimationTrac into the output stream.
 */
class MCD_CORE_API AnimationTrackWriter
{
public:
	static sal_checkreturn bool write(std::ostream& os, const AnimationTrack& track);
};	// AnimationTrackWriter

}	// namespace MCD

#endif	// __MCD_CORE_ANIMATIONTRACKWRITER__
