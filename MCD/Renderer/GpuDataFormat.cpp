#include "Pch.h"
#include "GpuDataFormat.h"

namespace MCD {

GpuDataFormat::GpuDataFormat(const StringHash& stringHash) 
	: name(stringHash.hash), format(-1), dataType(-1), components(-1), componentSize(0), componentCount(0)
{
}

}	// namespace MCD
