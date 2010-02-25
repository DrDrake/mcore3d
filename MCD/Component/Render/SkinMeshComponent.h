#ifndef __MCD_COMPONENTS_RENDER_SKINMESHCOMPONENT__
#define __MCD_COMPONENTS_RENDER_SKINMESHCOMPONENT__

#include "RenderableComponent.h"
#include "../../Core/Math/Skeleton.h"

namespace MCD {

class IResourceManager;
typedef IntrusivePtr<class Model> ModelPtr;
typedef IntrusivePtr<class Skeleton> SkeletonPtr;

class MCD_COMPONENT_API SkinMeshComponent : public RenderableComponent
{
public:
	explicit SkinMeshComponent();

	sal_override ~SkinMeshComponent();

// Cloning
	sal_override sal_checkreturn bool cloneable() const { return true; }

	sal_override sal_notnull Component* clone() const;

// Operations
	/*!	Create the necessary \em meshes for skinning.
		This function will insert the \em namePrefix to the resource name of \em basePose, and use
		the new resource name as a key to find the mesh model from \em resourceManager.
		If the new mesh model cannot be found, one will clone from \em basePose and cache it
		in \em resourceManager.

		Fail if \em basePose didn't has a resource name, or the clone operation fail.
	 */
	sal_checkreturn bool init(IResourceManager& resourceManager, const Model& basePose, sal_in_z const wchar_t* namePrefix=L"skinning");

	sal_override void render();

// Attrubutes
	/*!	This Model MAY be shared by multiple SkinMeshComponent, and is being
		modified during render();
	 */
	const ModelPtr meshes;
	const ModelPtr basePoseMeshes;
	SkeletonPose pose;			//!< The current pose
	SkeletonPtr skeleton;

protected:
	SkeletonPose mTmpPose;		//!< Member variable to reduce memory allocation during render().
};	// SkinMeshComponent

typedef WeakPtr<SkinMeshComponent> SkinMeshComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENTS_RENDER_SKINMESHCOMPONENT__
