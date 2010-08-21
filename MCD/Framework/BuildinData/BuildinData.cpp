#include "Pch.h"
#include "BuildinData.h"
#include "../../Core/System/MemoryFileSystem.h"

#include "Arial-20.fnt.h"
#include "Arial-20_0.png.h"

namespace MCD {

void addDataToMemoryFileSystem(MemoryFileSystem& fs)
{
	MCD_VERIFY(fs.add("buildin/Arial-20.fnt", _binary_Arial_20_fnt_start, sizeof(_binary_Arial_20_fnt_start)));
	MCD_VERIFY(fs.add("buildin/Arial-20_0.png", _binary_Arial_20_0_png_start, sizeof(_binary_Arial_20_0_png_start)));
}

}	// namespace MCD
