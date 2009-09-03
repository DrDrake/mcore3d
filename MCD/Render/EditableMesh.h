#ifndef __MCD_RENDER_EDITABLEMESH__
#define __MCD_RENDER_EDITABLEMESH__

#include "Mesh.h"

namespace MCD {

typedef IntrusivePtr<class MeshBuilder> MeshBuilderPtr;
template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

template<typename T> class Vec2;
typedef Vec2<float> Vec2f;

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
	
	/*! Call the after beginEditing() */
	size_t getTriangleCount() const;
	
	/*! Call the after beginEditing() */
	uint16_t* getTriangleIndexAt(size_t face);
	
	/*! Call the after beginEditing() */
	Vec3f& getPositionAt(uint16_t vertexIndex);
	
	/*! Call the after beginEditing() */
	Vec3f& getNormalAt(uint16_t vertexIndex);
	
	/*! Call the after beginEditing() */
	Vec2f& getUV0At(uint16_t vertexIndex);
	
	/*! Call the after beginEditing() */
	Vec2f& getUV1At(uint16_t vertexIndex);

	void endEditing(bool commit);
	
private:
	class Impl;
	Impl* mImpl;

};	// EditableMesh

typedef IntrusivePtr<EditableMesh> EditableMeshPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_EDITABLEMESH__