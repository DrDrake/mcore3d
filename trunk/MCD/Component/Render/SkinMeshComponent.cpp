#include "Pch.h"
#include "SkinMeshComponent.h"
#include "SkeletonAnimationComponent.h"
#include "../../Render/Effect.h"
#include "../../Render/Mesh.h"
#include "../../Render/Model.h"
#include "../../Render/Skinning.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/Math/Skeleton.h"
#include "../../Core/System/ResourceManager.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

// TODO: Move this function back to Model's member, if somehow a generic clone function is implemented later.
static Model* cloneModel(const Model& model, const Path& newPath)
{
	Model* ret = new Model(newPath);

	// We need to deep copy the Mesh and shallow copy the Effect
	for(const Model::MeshAndMaterial* i = model.mMeshes.begin(); i != model.mMeshes.end(); i = i->next()) {
		Model::MeshAndMaterial* m = new Model::MeshAndMaterial;
		m->mesh = i->mesh->clone("", Mesh::Stream);
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
	cloned->skeletonAnimation = this->skeletonAnimation;	// This will be re-assigned in postClone()
	return cloned;
}

bool SkinMeshComponent::postClone(const Entity& src, Entity& dest)
{
	// Find the Component in the src tree that corresponding to this
	SkinMeshComponent* srcComponent = dynamic_cast<SkinMeshComponent*>(
		ComponentPreorderIterator::componentByOffset(src, ComponentPreorderIterator::offsetFrom(dest, *this))
	);

	if(!srcComponent)
		return false;
	if(!srcComponent->skeletonAnimation)
		return true;

	// Find the Component in the src tree that corresponding to referenceToAnother
	skeletonAnimation = dynamic_cast<SkeletonAnimationComponent*>(
		ComponentPreorderIterator::componentByOffset(dest, ComponentPreorderIterator::offsetFrom(src, *srcComponent->skeletonAnimation))
	);

	return true;
}

bool SkinMeshComponent::init(IResourceManager& resourceManager, const Model& basePose, const char* namePrefix)
{
	if(basePose.fileId().getString().empty())
		return false;

	Path newPath = Path(std::string(namePrefix) + basePose.fileId().getString());
	ResourcePtr r = resourceManager.load(newPath);

	if(!r) {
		r = cloneModel(basePose, newPath);
		resourceManager.cache(r);	// Make each unique basePose has an uniqe shadow mesh as a temporary for skinning
	}

	const_cast<ModelPtr&>(meshes) = dynamic_cast<Model*>(r.get());
	const_cast<ModelPtr&>(basePoseMeshes) = const_cast<Model*>(&basePose);

	return meshes.get() != nullptr;
}

void SkinMeshComponent::render()
{
	Entity* e = entity();
	if(!e || !e->enabled || !basePoseMeshes || !meshes || !skeleton || !skeletonAnimation)
		return;

	glPushMatrix();
	glMultTransposeMatrixf(e->worldTransform().getPtr());

	mTmpPose = skeletonAnimation->pose;

	// NOTE: If the inverse was already baked into the animation track, we can skip this multiplication
	for(size_t i=0; i<mTmpPose.transforms.size(); ++i)
		mTmpPose.transforms[i] *= skeleton->basePoseInverse[i];

	{	// Perform skinning
		Model::MeshAndMaterial* i = meshes->mMeshes.begin();
		Model::MeshAndMaterial* j = basePoseMeshes->mMeshes.begin();
		for(; i != meshes->mMeshes.end() && j != basePoseMeshes->mMeshes.end(); i = i->next(), j = j->next()) {
			Mesh* m = i->mesh.get();
			if(!m || !j->mesh)
				continue;
			MCD::skinning(mTmpPose, *m, *j->mesh, 4, 5, m->normalAttrIdx);	// TODO: Remove the hardcoded constants
		}
	}

	meshes->draw();

	glPopMatrix();
}

}	// namespace MCD
