#include "Pch.h"
#include "AnimationTrackWriter.h"
#include "AnimationTrack.h"
#include "../System/Stream.h"
#include <iostream>

namespace MCD {

// TODO: Handle endian problem
bool AnimationTrackWriter::write(std::ostream& os, const AnimationTrack& track)
{
	if(!os)
		return false;

	AnimationTrack::ScopedReadLock lock(track);

	// Write the counters first
	uint16_t subtrackCount = static_cast<uint16_t>(track.subtrackCount());
	MCD::write(os, subtrackCount);

	for(size_t i=0; i<subtrackCount; ++i)
		MCD::write(os, uint32_t(track.keyframeCount(i)));

	MCD::write(os, track.loop);
	MCD::write(os, track.naturalFramerate);

	// Write the subtrack info
	// NOTE: For simplicity, the Subtrack::index is saved while it is not necessary.
	const AnimationTrack::Subtracks& st = track.subtracks;
	MCD::write(os, st.getPtr(), st.sizeInByte());

	// Write the key frames
	const AnimationTrack::KeyFrames& kf = track.keyframes;
	MCD::write(os, kf.getPtr(), kf.sizeInByte());

	return true;
}

}	// namespace MCD
