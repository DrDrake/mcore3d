#ifndef __MCD_RENDER_RAYMESHINTERSECT__
#define __MCD_RENDER_RAYMESHINTERSECT__

#include "ShareLib.h"

#include "../Core/Math/Vec3.h"
#include "../Core/Math/Mat44.h"
#include "../Core/System/LinkList.h"
#include "../Core/System/SharedPtr.h"

namespace MCD {

class EditableMesh;

/*!	An interface for performing ray to mesh intersection test.
	Different implementation can be made eg. SimpleRayMeshIntersect,
	MultiThreadRayMeshIntersect and may be CudaRayMeshIntersect.
 */
class MCD_ABSTRACT_CLASS MCD_RENDER_API IRayMeshIntersect
{
public:
	struct MeshRecord : LinkListBase::Node<MeshRecord>
	{
		MeshRecord(EditableMesh& _mesh) : mesh(_mesh) {}

		EditableMesh& mesh;
		bool hasTransform;
		Mat44f transform;
	};	// MeshRecord

	struct Hit : public LinkListBase::Node<Hit>
	{
		Hit(MeshRecord& _meshRec) : meshRec(_meshRec) {}

		MeshRecord& meshRec;
		float t;
		float u, v, w;
		int faceIdx;
	};	// Hit

	struct HitResult : public LinkListBase::Node<HitResult>
	{
		LinkList<Hit> hits;
		Hit* closest;
		Vec3f rayOrig;
		Vec3f rayDir;
	};	// HitResult

	virtual ~IRayMeshIntersect() {}

	//!	Clear all added meshes and previous hit-results.
	virtual void reset() = 0;

	/*!	Add a mesh to this intersect object, duplicated mesh will not be checked.
		The ownership of the mesh will NOT be alerted.
		The user must call build after all meshes had been added.
	*/
	virtual void addMesh(EditableMesh& mesh) = 0;

	/*!	Add a mesh (with transformation) to this intersect object, duplicated mesh will not be checked.
		The ownership of the mesh will NOT be alerted.
		The user must call build after all meshes had been added.
	*/
	virtual void addMesh(EditableMesh& mesh, const Mat44f& transform) = 0;

	/*!	Call this method the build any internal data-structure after all meshes has been added.
		\sa addMesh()
	*/
	virtual void build() = 0;

	/*!	Begin intersection testes, any pervious results will be cleared.
		The user should add the meshes before calling this method by using
		addMesh() and build().
	*/
	virtual void begin() = 0;

	//!	Issue an intersection test, the result will be available when end() is called.
	virtual void test(const Vec3f& rayOrig, const Vec3f& rayDir, bool twoSided) = 0;

	/*!	End intersection testes, the results of issued test will be returned.
		The life-time of the result will keep along witht the concret derived class,
		and it's content will be cleared when begin() is invoked again.
	 */
	virtual LinkList<HitResult>& end() = 0;
};	// IRayMeshIntersect

class MCD_RENDER_API SimpleRayMeshIntersect : public IRayMeshIntersect, Noncopyable
{
public:
	SimpleRayMeshIntersect();

	sal_override ~SimpleRayMeshIntersect();

	sal_override void reset();

	sal_override void addMesh(EditableMesh& mesh);

	sal_override void addMesh(EditableMesh& mesh, const Mat44f& transform);

	sal_override void build();

	sal_override void begin();

	sal_override void test(const Vec3f& rayOrig, const Vec3f& rayDir, bool twoSided);

	sal_override LinkList<HitResult>& end();

private:
	class Impl;
	Impl* mImpl;
};	// SimpleRayMeshIntersect

}	// namespace MCD

#endif	// __MCD_RENDER_RAYMESHINTERSECT__