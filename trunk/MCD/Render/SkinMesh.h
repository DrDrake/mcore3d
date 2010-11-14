#ifndef __MCD_RENDER_SKINMESH__
#define __MCD_RENDER_SKINMESH__

#include "Mesh.h"

namespace MCD {

typedef IntrusiveWeakPtr<class SkeletonPose> SkeletonPosePtr;

class MCD_RENDER_API SkinMesh : public MeshComponent
{
public:
	SkinMesh();

	sal_override ~SkinMesh();

// Cloning
	sal_override sal_notnull Component* clone() const;
	sal_override sal_checkreturn bool postClone(const Entity& src, Entity& dest);

// Attruibutes
	/// This mesh should correspond to the base skeleton pose.
	MeshPtr basePoseMesh;

	/// The skeleton pose to apply to the skin mesh.
	SkeletonPosePtr pose;

protected:
	sal_override void draw(void* context, Statistic& statistic);
};	// SkinMesh

typedef IntrusiveWeakPtr<SkinMesh> SkinMeshPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_SKINMESH__
