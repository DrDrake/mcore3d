#include "Pch.h"
#include "AnimationTrackWriter.h"
#include "AnimationTrack.h"
#include "../System/Stream.h"
#include <iostream>

namespace MCD {

// TODO: Handle endian problem
bool AnimationClipWriter::write(std::ostream& os, const AnimationClip& track)
{
	if(!os)
		return false;

	AnimationClip::ScopedReadLock lock(track);

	// Write the counters first
	uint16_t subtrackCount = static_cast<uint16_t>(track.subtrackCount());
	MCD::write(os, subtrackCount);

	for(size_t i=0; i<subtrackCount; ++i)
		MCD::write(os, uint32_t(track.keyframeCount(i)));

	MCD::write(os, track.loop);
	MCD::write(os, track.naturalFramerate);

	// Write the subtrack info
	// NOTE: For simplicity, the Subtrack::index is saved while it is not necessary.
	const AnimationClip::Subtracks& st = track.subtracks;
	MCD::write(os, st.getPtr(), st.sizeInByte());

	// Write the key frames
	const AnimationClip::KeyFrames& kf = track.keyframes;
	MCD::write(os, kf.getPtr(), kf.sizeInByte());

	return true;
}

}	// namespace MCD
