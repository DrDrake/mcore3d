#ifndef __MCD_RENDER_EDITABLEMESH__
#define __MCD_RENDER_EDITABLEMESH__

#include "Mesh.h"

namespace MCD {

typedef IntrusivePtr<class MeshBuilder> MeshBuilderPtr;

/*! A Mesh which provide data read-back and editing functions.
*/
class MCD_RENDER_API EditableMesh : public Mesh
{
public:
	explicit EditableMesh(const Path& fileId = L"");

	//! Construct that take another mesh's buffers to share with.
	EditableMesh(const Path& fileId, const EditableMesh& shareBuffer);

	/*! Pointer to the MeshBuilder of this mesh, it is primaryly used for reading the mesh data
		without downloading them from the GPU.
		Please notice that this pointer may be nullptr.
	*/
	MeshBuilderPtr builder;

};	// EditableMesh

typedef IntrusivePtr<EditableMesh> EditableMeshPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_EDITABLEMESH__