#ifndef __MCD_RENDER_RAYMESHINTERSECT__
#define __MCD_RENDER_RAYMESHINTERSECT__

#include "ShareLib.h"

#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"
#include "../Core/Math/Vec4.h"
#include "../Core/Math/Mat44.h"
#include "../Core/System/LinkList.h"
#include "../Core/System/SharedPtr.h"

namespace MCD {

class Mesh;

/*!	An interface for performing ray to mesh intersection test.
	Different implementation can be made eg. SimpleRayMeshIntersect,
	MultiThreadRayMeshIntersect and may be CudaRayMeshIntersect.

	Example usage:
@code
	SimpleRayMeshIntersect i;
	i.addMesh(mesh);				// mesh:Mesh
	i.addMesh(mesh, transform);		// transform:Mat44f
	i.build();

	i.begin();
	i.test(rayOrig1, rayDir1, false);	// issue an intersection test with ray 1; disabling 2-sided test
	i.test(rayOrig2, rayDir2, true);	// issue an intersection test with ray 2; enabling 2-sided test
	i.end();

	LinkList<IRayMeshIntersect::HitResult>& result = i.result(); // there should be two results in the list

@endcode
 */
class MCD_ABSTRACT_CLASS MCD_RENDER_API IRayMeshIntersect
{
public:
	struct MeshRecord : LinkListBase::Node<MeshRecord>
	{
		MeshRecord(Mesh& _mesh) : mesh(_mesh) {}

		Mesh& mesh;
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

	struct MCD_RENDER_API Helper
	{
		static Vec3f getHitPosition(IRayMeshIntersect::Hit* hit);
		static Vec3f getHitNormal(IRayMeshIntersect::Hit* hit);
		static Vec2f getHitUV2d(IRayMeshIntersect::Hit* hit, size_t unit);
		static Vec3f getHitUV3d(IRayMeshIntersect::Hit* hit, size_t unit);
		static Vec4f getHitUV4d(IRayMeshIntersect::Hit* hit, size_t unit);
	};	// Helper

	virtual ~IRayMeshIntersect() {}

	//!	Clear all added meshes and previous hit-results.
	virtual void reset() = 0;

	/*!	Add a mesh to this intersect object, and this will increase the reference count of the mesh.
		Duplicated mesh will not be checked.
		If mesh is nullptr, it will be ignored.
		The user must call build after all meshes had been added.
	*/
	virtual void addMesh(sal_maybenull Mesh* mesh) = 0;

	/*!	Add a mesh (with transformation) to this intersect object.
		\sa addMesh()
	*/
	virtual void addMesh(sal_maybenull Mesh* mesh, const Mat44f& transform) = 0;

	/*!	Call this method the build any internal data-structure after all meshes has been added.
		\sa addMesh()
	*/
	virtual void build() = 0;

	/*!	Begin intersection testes, any pervious results will be cleared.
		The user should add the meshes before calling this method by using
		addMesh() and build().

		Call the added mesh's beginEditing() before this method is invoked!
	*/
	virtual void begin() = 0;

	/*!	Issue an intersection test, the result will be available when end() is called.

		Call the added mesh's beginEditing() before this method is invoked!
	*/
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

	sal_override void addMesh(sal_maybenull Mesh* mesh);

	sal_override void addMesh(sal_maybenull Mesh* mesh, const Mat44f& transform);

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