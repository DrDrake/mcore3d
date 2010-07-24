#include "Pch.h"
#include "SkeletonWriter.h"
#include "Skeleton.h"
#include "../System/Stream.h"
#include <iostream>

namespace MCD {

// TODO: Handle endian problem
bool SkeletonWriter::write(std::ostream& os, const Skeleton& skeleton)
{
	if(!os)
		return false;

	// Write the counters first
	uint16_t jointCount = static_cast<uint16_t>(skeleton.parents.size());
	MCD::write(os, jointCount);

	// Write the joint data
	for(size_t i=0; i<jointCount; ++i) {
		MCD::write(os, uint16_t(skeleton.parents[i]));
		MCD::writeString(os, skeleton.names[i]);
	}

	return true;
}

}	// namespace MCD
