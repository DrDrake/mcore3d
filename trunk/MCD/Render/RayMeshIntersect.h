#ifndef __MCD_RENDER_RAYMESHINTERSECT__
#define __MCD_RENDER_RAYMESHINTERSECT__

#include "Mesh.h"
#include "../Core/Math/Mat44.h"
#include "../Core/System/LinkList.h"

namespace MCD {

/*!	An interface for performing ray to mesh intersection test.
	Different implementation can be made eg. SimpleRayMeshIntersect,
	MultiThreadRayMeshIntersect and may be CudaRayMeshIntersect.

	Example usage:
	\code
	SimpleRayMeshIntersect i;
	i.addMesh(mesh);				// mesh:Mesh
	i.addMesh(mesh, transform);		// transform:Mat44f
	i.build();

	i.begin();
	i.test(rayOrig1, rayDir1, false);	// Issue an intersection test with ray 1; disabling 2-sided test
	i.test(rayOrig2, rayDir2, true);	// Issue an intersection test with ray 2; enabling 2-sided test
	i.end();

	// There should be two results in the list, if both rays does intersect the mesh.
	LinkList<IRayMeshIntersect::HitResult>& result = i.result();
	\endcode
 */
class MCD_ABSTRACT_CLASS MCD_RENDER_API IRayMeshIntersect
{
public:
	struct MeshRecord;

	/*!	Example code on how to query informations from the hit result.
		\code
		// Query the hit position:
		Vec3f hitPos = rayOrig + hit.>t * (rayTarget - rayOrig).normalizedCopy();

		// Query the hitting point's normal:
		Mesh& mesh = hit.mesh();
		StrideArray<Vec3f> normal = mesh.mapAttribute<Vec3f>(mesh.normalAttrIdx, hit.mappedBuffers(), Mesh::Read);

		Vec3f v0 = normal[hit.faceIdx*3 + 0];
		Vec3f v1 = normal[hit.faceIdx*3 + 1];
		Vec3f v2 = normal[hit.faceIdx*3 + 2];

		if(const Mat44f* m = hit.transform()) {
			m->transformNormal(v0);
			m->transformNormal(v1);
			m->transformNormal(v2);
		}

		Vec3f hitNormal = hit.w * v0 + hit.u * v1 + hit.v * v2;
	 */
	struct MCD_RENDER_API Hit : public LinkListBase::Node<Hit>
	{
		explicit Hit(MeshRecord& rec) : meshRec(rec) {}

		Mesh& mesh();

		//!	Get back the transform that was passed to addMesh(), if any.
		sal_maybenull const Mat44f* transform();

		Mesh::MappedBuffers& mappedBuffers();

		float t;		//!< Parameter along the ray (will NOT be negative)
		float u, v, w;	//!< Barycentric coordinates
		int faceIdx;	//!< Which is vertex index / 3
		MeshRecord& meshRec;
	};	// Hit

	struct HitResult : public LinkListBase::Node<HitResult>
	{
		LinkList<Hit> hits;
		//! Always points to one of the element in \em hits, which is closet to \em rayOrig.
		sal_notnull Hit* closest;
		Vec3f rayOrig;
		Vec3f rayDir;
	};	// HitResult

	virtual ~IRayMeshIntersect() {}

	//!	Clear all added meshes and previous hit-results.
	virtual void reset() = 0;

	/*!	Add a mesh to this intersect object, and this will increase the reference count of the mesh.
		Duplicated mesh will not be checked.
		The user must call build after all meshes had been added.
		\note
			Once the mesh is added, ALL it's buffer will be mapped (locked) and cannot perform rendering.
			The mesh is un-mapped till reset() is called.
	 */
	virtual void addMesh(Mesh& mesh) = 0;

	/*!	Add a mesh (with transformation) to this intersect object.
		\sa addMesh()
	 */
	virtual void addMesh(Mesh& mesh, const Mat44f& transform) = 0;

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
		The life-time of the result will keep along witht the concrete derived class,
		and it's content will be cleared when begin() is invoked again.
	 */
	virtual void end() = 0;

	/*!	Returns the results of issued test between the last begin() & end().
		The life-time of the results will keep along with the concert derived class,
		and it's content will be cleared when begin() is invoked again.
	 */
	virtual LinkList<IRayMeshIntersect::HitResult>& results() = 0;
};	// IRayMeshIntersect

class MCD_RENDER_API SimpleRayMeshIntersect : public IRayMeshIntersect, Noncopyable
{
public:
	SimpleRayMeshIntersect();

	sal_override ~SimpleRayMeshIntersect();

	sal_override void reset();

	sal_override void addMesh(Mesh& mesh);

	sal_override void addMesh(Mesh& mesh, const Mat44f& transform);

	sal_override void build();

	sal_override void begin();

	sal_override void test(const Vec3f& rayOrig, const Vec3f& rayDir, bool twoSided);

	sal_override void end();

	sal_override LinkList<IRayMeshIntersect::HitResult>& results();

private:
	class Impl;
	Impl& mImpl;
};	// SimpleRayMeshIntersect

}	// namespace MCD

#endif	// __MCD_RENDER_RAYMESHINTERSECT__