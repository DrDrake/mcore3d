#include "Pch.h"
#include "SkinMeshComponent.h"
#include "../../Render/Effect.h"
#include "../../Render/Mesh.h"
#include "../../Render/Model.h"
#include "../../Render/Skinning.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/Math/Skeleton.h"
#include "../../Core/System/ResourceManager.h"

namespace MCD {

// TODO: Move this function back to Model's member, if somehow a generic clone function is implemented later.
static Model* cloneModel(const Model& model)
{
	Model* ret = new Model(L"");

	// We need to deep copy the Mesh and shallow copy the Effect
	for(const Model::MeshAndMaterial* i = model.mMeshes.begin(); i != model.mMeshes.end(); i = i->next()) {
		Model::MeshAndMaterial* m = new Model::MeshAndMaterial;
		m->mesh = i->mesh->clone(L"", Mesh::Stream);
		m->effect = i->effect;
		ret->mMeshes.pushBack(*m);
	}

	return ret;
}

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
		r = cloneModel(basePose);
		resourceManager.cache(r);
	}

	const_cast<ModelPtr&>(meshes) = dynamic_cast<Model*>(r.get());
	const_cast<ModelPtr&>(basePoseMeshes) = const_cast<Model*>(&basePose);

	return meshes.get() != nullptr;
}

void SkinMeshComponent::render()
{
	Entity* e = entity();
	if(!e || !e->enabled || !basePoseMeshes || !meshes || !skeleton)
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
			if(!m || !j->mesh)
				continue;
			MCD::skinning(mTmpPose, *m, *j->mesh, 4, 5, m->normalAttrIdx);
		}
	}

	meshes->draw();
}

}	// namespace MCD
