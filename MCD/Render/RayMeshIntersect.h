#ifndef __MCD_RENDER_RAYMESHINTERSECT__
#define __MCD_RENDER_RAYMESHINTERSECT__

#include "ShareLib.h"

#include "../Core/Math/Vec3.h"
#include "../Core/System/LinkList.h"
#include "../Core/System/SharedPtr.h"

namespace MCD {

class EditableMesh;
typedef IntrusivePtr<EditableMesh> EditableMeshPtr;

/*!	An interface for performing ray to mesh intersection test.
	Different implementation can be made eg. SimpleRayMeshIntersect,
	MultiThreadRayMeshIntersect and may be CudaRayMeshIntersect.
 */
class MCD_ABSTRACT_CLASS MCD_RENDER_API IRayMeshIntersect
{
public:
	struct Hit : public LinkListBase::Node<Hit>
	{
		float t;
		float u, v, w;
		int faceIdx;
		EditableMeshPtr mesh;	//!< Note that we take shared ownership of the mesh and always not null.
	};	// Hit

	struct HitResult : public LinkListBase::Node<HitResult>
	{
		LinkList<Hit> hits;
		Hit* closest;
		Vec3f rayOrig;
		Vec3f rayDir;
	};	// HitResult

	virtual ~IRayMeshIntersect() {}

	//!	Clear all added meshes and stored results.
	virtual void reset() = 0;

	//!	Add a mesh to this intersect object, duplicated mesh will not be checked.
	// TODO: Any restriction when calling this? Before begin() ?
	virtual void addMesh(EditableMesh& mesh) = 0;

	//!	Begin intersection testes, any pervious results will be cleared.
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

	sal_override void begin();

	sal_override void test(const Vec3f& rayOrig, const Vec3f& rayDir, bool twoSided);

	sal_override LinkList<HitResult>& end();

private:
	class Impl;
	Impl* mImpl;
};	// SimpleRayMeshIntersect

}	// namespace MCD

#endif	// __MCD_RENDER_RAYMESHINTERSECT__