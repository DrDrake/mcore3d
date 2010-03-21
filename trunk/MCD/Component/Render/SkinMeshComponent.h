#ifndef __MCD_COMPONENTS_RENDER_SKINMESHCOMPONENT__
#define __MCD_COMPONENTS_RENDER_SKINMESHCOMPONENT__

#include "RenderableComponent.h"
#include "../../Core/Math/Skeleton.h"

namespace MCD {

class IResourceManager;
typedef IntrusivePtr<class Model> ModelPtr;
typedef IntrusivePtr<class Skeleton> SkeletonPtr;
typedef IntrusiveWeakPtr<class SkeletonAnimationComponent> SkeletonAnimationComponentPtr;

class MCD_COMPONENT_API SkinMeshComponent : public RenderableComponent
{
public:
	explicit SkinMeshComponent();

	sal_override ~SkinMeshComponent();

// Cloning
	sal_override sal_checkreturn bool cloneable() const { return true; }

	sal_override sal_notnull Component* clone() const;

	sal_override sal_checkreturn bool postClone(const Entity& src, Entity& dest);

// Operations
	/*!	Create the necessary \em meshes for skinning.
		This function will insert the \em namePrefix to the resource name of \em basePose, and use
		the new resource name as a key to find the mesh model from \em resourceManager.
		If the new mesh model cannot be found, one will clone from \em basePose and cache it
		in \em resourceManager.

		Fail if \em basePose didn't has a resource name, or the clone operation fail.
	 */
	sal_checkreturn bool init(IResourceManager& resourceManager, const Model& basePose, sal_in_z const char* namePrefix="skinning");

	sal_override void render();

// Attrubutes
	/*!	This Model MAY be shared by multiple SkinMeshComponent, and is being
		modified during render();
	 */
	const ModelPtr meshes;
	const ModelPtr basePoseMeshes;
	SkeletonPtr skeleton;

	/*!	The SkeletonAnimationComponent::pose will apply to this skin mesh.
		Storing pointer pointer from SkinMeshComponent to SkeletonAnimationComponent
		allows using one animation pose to skin multiple mesh (eg. cloth on human body).
	 */
	SkeletonAnimationComponentPtr skeletonAnimation;

protected:
	SkeletonPose mTmpPose;		//!< Member variable to reduce memory allocation during render().
};	// SkinMeshComponent

typedef IntrusiveWeakPtr<SkinMeshComponent> SkinMeshComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENTS_RENDER_SKINMESHCOMPONENT__
