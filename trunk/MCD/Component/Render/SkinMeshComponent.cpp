#include "Pch.h"
#include "SkinMeshComponent.h"
#include "../../Render/Mesh.h"
#include "../../Render/Model.h"
#include "../../Render/Skinning.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/Math/Skeleton.h"
#include "../../Core/System/ResourceManager.h"

namespace MCD {

SkinMeshComponent::SkinMeshComponent()
{
}

SkinMeshComponent::~SkinMeshComponent()
{
}

Component* SkinMeshComponent::clone() const
{
	SkinMeshComponent* cloned = new SkinMeshComponent;
	const_cast<ModelPtr&>(cloned->basePoseMeshes) = this->basePoseMeshes;
	const_cast<ModelPtr&>(cloned->meshes) = this->meshes;
	cloned->skeleton = this->skeleton;
	return cloned;
}

bool SkinMeshComponent::init(IResourceManager& resourceManager, const Model& basePose, const wchar_t* namePrefix)
{
	if(basePose.fileId().getString().empty())
		return false;

	Path newPath = Path(std::wstring(namePrefix) + basePose.fileId().getString());
	ResourcePtr r = resourceManager.load(newPath);

	if(!r) {
//		r = basePose.clone();
		resourceManager.cache(r);
	}

	const_cast<ModelPtr&>(meshes) = dynamic_cast<Model*>(r.get());
	return meshes.get() != nullptr;
}

void SkinMeshComponent::render()
{
	Entity* e = entity();
	if(!e || !e->enabled)
		return;

	mTmpPose = pose;

	// NOTE: If the inverse was already baked into the animation track, we can skip this multiplication
	for(size_t i=0; i<pose.transforms.size(); ++i)
		mTmpPose.transforms[i] *= skeleton->basePoseInverse[i];

	{	// Perform skinning
		Model::MeshAndMaterial* i = meshes->mMeshes.begin();
		Model::MeshAndMaterial* j = basePoseMeshes->mMeshes.begin();
		for(; i != meshes->mMeshes.end() && j != basePoseMeshes->mMeshes.end(); i = i->next(), j = j->next()) {
			Mesh* m = i->mesh.get();
			MCD::skinning(mTmpPose, *m, *j->mesh, 5, 6, m->normalAttrIdx);
		}
	}

	meshes->draw();
}

}	// namespace MCD
