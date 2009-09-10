#include "Pch.h"
#include "RayMeshIntersect.h"
#include "EditableMesh.h"

#include <list>

namespace MCD {

class SimpleRayMeshIntersect::Impl
{
public:
	LinkList<IRayMeshIntersect::HitResult> mLastResults;
	LinkList<MeshRecord> mMeshes;

// source code copied from:
// Fast, Minimum Storage Ray/Triangle Intersection
// http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
//
#define _EPSILON 0.000001

#define _CROSS(dest,v1,v2) \
	dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define _DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define _SUB(dest,v1,v2) \
	dest[0]=v1[0]-v2[0]; \
	dest[1]=v1[1]-v2[1]; \
	dest[2]=v1[2]-v2[2];

	template<typename REAL>
	int intersect_triangle(
		const REAL orig[3], const REAL dir[3],
		const REAL vert0[3], const REAL vert1[3], const REAL vert2[3],
		REAL *t, REAL *u, REAL *v,
		bool twosided)
	{
		REAL edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
		REAL det,inv_det;
		// find vectors for two edges sharing vert0
		_SUB(edge1, vert1, vert0);
		_SUB(edge2, vert2, vert0);
		// begin calculating determinant - also used to calculate U parameter
		_CROSS(pvec, dir, edge2);
		// if determinant is near zero, ray lies in plane of triangle
		det = _DOT(edge1, pvec);

		if(!twosided) 
		{	// define TEST_CULL if culling is desired
			if (det < _EPSILON)
				return 0;
			// calculate distance from vert0 to ray origin
			_SUB(tvec, orig, vert0);
			// calculate U parameter and test bounds
			*u = _DOT(tvec, pvec);
			if (*u < 0.0 || *u > det)
				return 0;
			// prepare to test V parameter
			_CROSS(qvec, tvec, edge1);
			// calculate V parameter and test bounds
			*v = _DOT(dir, qvec);
			if (*v < 0.0 || *u + *v > det)
				return 0;

			// calculate t, scale parameters, ray intersects triangle
			*t = _DOT(edge2, qvec);
			inv_det = (REAL)1.0 / det;
			*t *= inv_det;
			*u *= inv_det;
			*v *= inv_det;
		}
		else
		{	// the non-culling branch
			if (det > (REAL)-_EPSILON && det < (REAL)_EPSILON)
				return 0;
			inv_det = (REAL)1.0 / det;
			// calculate distance from vert0 to ray origin
			_SUB(tvec, orig, vert0);
			// calculate U parameter and test bounds
			*u = _DOT(tvec, pvec) * inv_det;
			if (*u < (REAL)0.0 || *u > (REAL)1.0)
				return 0;
			// prepare to test V parameter
			_CROSS(qvec, tvec, edge1);
			// calculate V parameter and test bounds
			*v = _DOT(dir, qvec) * inv_det;
			if (*v < (REAL)0.0 || *u + *v > (REAL)1.0)
				return 0;
			// calculate t, ray intersects triangle
			*t = _DOT(edge2, qvec) * inv_det;
		}

		return 1;
	}

#undef _EPSILON
#undef _CROSS
#undef _DOT
#undef _SUB

	template<typename REAL>
	int intersectTriangle(
		const Vec3<REAL>& orig, const Vec3<REAL>& dir,
		const Vec3<REAL>& vert0, const Vec3<REAL>& vert1, const Vec3<REAL>& vert2,
		REAL *t, REAL *u, REAL *v,
		bool twosided)
	{
		return intersect_triangle<REAL>(orig.data, dir.data, vert0.data, vert1.data, vert2.data, t, u, v, twosided);
	}

};	// Impl

SimpleRayMeshIntersect::SimpleRayMeshIntersect()
	: mImpl(new Impl)
{
}

SimpleRayMeshIntersect::~SimpleRayMeshIntersect()
{
	delete mImpl;
}

void SimpleRayMeshIntersect::reset()
{
	mImpl->mMeshes.destroyAll();
	mImpl->mLastResults.destroyAll();
}

void SimpleRayMeshIntersect::addMesh(EditableMesh& mesh)
{
	MeshRecord* rec = new MeshRecord(mesh);
	rec->hasTransform = false;
	mImpl->mMeshes.pushBack(*rec);
}

void SimpleRayMeshIntersect::addMesh(EditableMesh& mesh, const Mat44f& transform)
{
	MeshRecord* rec = new MeshRecord(mesh);
	rec->hasTransform = true;
	rec->transform = transform;
	mImpl->mMeshes.pushBack(*rec);
}

void SimpleRayMeshIntersect::build()
{
}

void SimpleRayMeshIntersect::begin()
{
	mImpl->mLastResults.destroyAll();

	for(MeshRecord* i = mImpl->mMeshes.begin()
		; i != mImpl->mMeshes.end()
		; i = i->next())
	{
		i->mesh.beginEditing();
	}
}

void SimpleRayMeshIntersect::test(const Vec3f& rayOrig, const Vec3f& rayDir, bool twoSided)
{
	HitResult* result = new HitResult;
	result->rayOrig = rayOrig;
	result->rayDir = rayDir;
	result->closest = nullptr;

	for(MeshRecord* i = mImpl->mMeshes.begin()
		; i != mImpl->mMeshes.end()
		; i = i->next())
	{
		EditableMesh& mesh = i->mesh;

		const size_t cTriCnt = mesh.getTriangleCount();

		for(size_t itri = 0; itri < cTriCnt; ++itri)
		{
			uint16_t* idx = mesh.getTriangleIndexAt(itri);

			// NOTE: Will it be more efficient to use the MeshBuilder's buffer and offset
			// interface, other than the immediate mode interface of EditableMesh?
			Vec3f v0 = mesh.getPositionAt(idx[0]);
			Vec3f v1 = mesh.getPositionAt(idx[1]);
			Vec3f v2 = mesh.getPositionAt(idx[2]);

			if(i->hasTransform)
			{
				i->transform.transformPoint(v0);
				i->transform.transformPoint(v1);
				i->transform.transformPoint(v2);
			}

			float t, u, v;

			if(1 == mImpl->intersectTriangle<float>(rayOrig, rayDir, v0, v1, v2, &t, &u, &v, twoSided))
			{
				if(t < 0) continue;
				
				Hit& hit = *(new Hit(*i));
				hit.faceIdx = itri;
				hit.t = t;
				hit.u = u;
				hit.v = v;
				hit.w = (1.0f - u - v);

				if(nullptr == result->closest)
					result->closest = &hit;
				else if(t < result->closest->t)
					result->closest = &hit;
				
				result->hits.pushBack(hit);
			}
		}
	}

	mImpl->mLastResults.pushBack(*result);
}

LinkList<IRayMeshIntersect::HitResult>& SimpleRayMeshIntersect::end()
{
	for(MeshRecord* i = mImpl->mMeshes.begin()
		; i != mImpl->mMeshes.end()
		; i = i->next())
	{
		i->mesh.endEditing(false);
	}

	return mImpl->mLastResults;
}

}	// namespace MCD