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
    
    void beginEditing();

    /*
	size_t getTriangleCount() const;

	uint16_t* getTriangleIndexAt(size_t face);

	Vec3f& getPositionAt(uint16_t vertexIndex);

	Vec3f& getNormalAt(uint16_t vertexIndex);
	
	Vec3f& getTangentAt(uint16_t vertexIndex);

	Vec2f& getUV0At(uint16_t vertexIndex);

	Vec2f& getUV1At(uint16_t vertexIndex);

	Vec4f& getColorAt(size_t vertexIndex);

	void endEditing(bool commit);
    */

};	// EditableMesh

typedef IntrusivePtr<EditableMesh> EditableMeshPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_EDITABLEMESH__