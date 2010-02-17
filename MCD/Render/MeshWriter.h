#ifndef __MCD_RENDER_MESHWRITER__
#define __MCD_RENDER_MESHWRITER__

#include "ShareLib.h"
#include <iosfwd>

namespace MCD {

class Mesh;

/*!	A very simple writer that dump the Mesh buffers into the output stream.
 */
class MCD_RENDER_API MeshWriter
{
public:
	static sal_checkreturn bool write(std::ostream& os, const Mesh& mesh);
};	// MeshWriter

}	// namespace MCD

#endif	// __MCD_RENDER_MESHWRITER__
