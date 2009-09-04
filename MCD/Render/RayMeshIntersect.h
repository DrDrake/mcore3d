#ifndef __MCD_RENDER_RAYMESHINTERSECT__
#define __MCD_RENDER_RAYMESHINTERSECT__

#include "ShareLib.h"

#include "../Core/System/LinkList.h"
#include "../Core/Math/Vec3.h"

namespace MCD
{
class EditableMesh;

class MCD_ABSTRACT_CLASS MCD_RENDER_API RayMeshIntersect
{
public:
	struct Hit : public LinkListBase::Node<Hit>
	{
		float t;
		float u, v, w;
		int faceIdx;
		EditableMesh* mesh;
	};

	struct HitResult : public LinkListBase::Node<HitResult>
	{
		LinkList<Hit> hits;
		Hit* closest;
		Vec3f rayOrig;
		Vec3f rayDir;
	};

	virtual ~RayMeshIntersect() {}

	virtual void reset() = 0;

	virtual void addMesh(EditableMesh& mesh) = 0;

	virtual void begin() = 0;

	virtual void test(const Vec3f& rayOrig, const Vec3f& rayDir, bool twoSided) = 0;

	virtual LinkList<HitResult>& end() = 0;

};	// RayMeshIntersect

class MCD_RENDER_API SimpleRayMeshIntersect : public RayMeshIntersect
{
public:
	SimpleRayMeshIntersect();

	virtual ~SimpleRayMeshIntersect();

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