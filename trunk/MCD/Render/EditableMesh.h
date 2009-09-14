#ifndef __MCD_RENDER_EDITABLEMESH__
#define __MCD_RENDER_EDITABLEMESH__

#include "Mesh.h"

namespace MCD {

typedef IntrusivePtr<class MeshBuilder> MeshBuilderPtr;

template<typename T> class Vec4;
typedef Vec4<float> Vec4f;

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

template<typename T> class Vec2;
typedef Vec2<float> Vec2f;

/*! A Mesh which provide data read-back and editing functions.
	Currently it just a very thin wrapper over MeshBuilder; transforming
	the buffer manipulation interface of MeshBuilder into a OpenGL
	intermediate mode interface.
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

	bool isEditing() const;

	void beginEditing();

	//!	Should call after beginEditing()
	size_t getTriangleCount() const;

	//!	Should call after beginEditing()
	uint16_t* getTriangleIndexAt(size_t face);

	//!	Should call after beginEditing()
	Vec3f& getPositionAt(uint16_t vertexIndex);

	//!	Should call after beginEditing()
	Vec3f& getNormalAt(uint16_t vertexIndex);

	//!	Should call after beginEditing()
	Vec2f& getUV2dAt(size_t unit, uint16_t vertexIndex);

	//!	Should call after beginEditing()
	Vec3f& getUV3dAt(size_t unit, uint16_t vertexIndex);

	//!	Should call after beginEditing()
	Vec4f& getUV4dAt(size_t unit, uint16_t vertexIndex);

	//!	\param commit Currently this parameter is not used.
	void endEditing(bool commit);

protected:
	sal_override ~EditableMesh();

private:
	class Impl;
	Impl* mImpl;
};	// EditableMesh

typedef IntrusivePtr<EditableMesh> EditableMeshPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_EDITABLEMESH__
