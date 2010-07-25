#include "Pch.h"
#include "SkinMesh.h"
#include "SkeletonAnimation.h"
#include "Effect.h"
#include "Mesh.h"
#include "Model.h"
#include "Skinning.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Math/Skeleton.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/ThreadedCpuProfiler.h"
#include "../../3Party/glew/glew.h"

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

	// Find the Component in the src tree that corresponding to srcComponent->skeletonAnimation
	skeletonAnimation = dynamic_cast<SkeletonAnimationComponent*>(
		ComponentPreorderIterator::componentByOffset(dest, ComponentPreorderIterator::offsetFrom(src, *srcComponent->skeletonAnimation))
	);

	return true;
}

bool SkinMeshComponent::init(IResourceManager& resourceManager, const Model& basePose, const char* nameSuffix)
{
	if(basePose.fileId().getString().empty())
		return false;

	Path newPath = Path(basePose.fileId().getString() + nameSuffix);
	ResourcePtr r = resourceManager.load(newPath);

	if(!r) {
		r = cloneModel(basePose, newPath);
		resourceManager.cache(r);	// Make each unique basePose has an uniqe shadow mesh as a temporary for skinning
	}

	const_cast<ModelPtr&>(meshes) = dynamic_cast<Model*>(r.get());
	const_cast<ModelPtr&>(basePoseMeshes) = const_cast<Model*>(&basePose);

	return meshes.get() != nullptr;
}

void SkinMeshComponent::render2(void* context)
{
	Entity* e = entity();
	if(!e || !e->enabled || !basePoseMeshes || !meshes || !skeleton || !skeletonAnimation)
		return;

	glPushMatrix();
	glMultTransposeMatrixf(e->worldTransform().getPtr());

	mTmpPose = skeletonAnimation->pose;
	SkeletonPose visualizePose = mTmpPose;

	// NOTE: If the inverse was already baked into the animation track, we can skip this multiplication
	for(size_t i=0; i<mTmpPose.transforms.size(); ++i)
		mTmpPose.transforms[i] *= skeleton->basePoseInverse[i];

	{	// Perform skinning
		ThreadedCpuProfiler::Scope cpuProfiler("SkinMeshComponent::cpu skinning");

		Model::MeshAndMaterial* i = meshes->mMeshes.begin();
		Model::MeshAndMaterial* j = basePoseMeshes->mMeshes.begin();
		for(; i != meshes->mMeshes.end() && j != basePoseMeshes->mMeshes.end(); i = i->next(), j = j->next()) {
			Mesh* m = i->mesh.get();
			if(!m || !j->mesh)
				continue;

			// TODO: Remove the search for attribute index
			int8_t blendIndexIdx = -1;
			int8_t blendWeightIdx = -1;
			for(size_t k=0; k<m->attributeCount; ++k) {
				if(m->attributes[k].format.semantic == StringHash("jointIndex"))
					blendIndexIdx = uint8_t(k);
				if(m->attributes[k].format.semantic == StringHash("jointWeight"))
					blendWeightIdx = uint8_t(k);
			}

			if(blendIndexIdx != -1 && blendWeightIdx != -1)
				MCD::skinning(mTmpPose, *m, *j->mesh, blendIndexIdx, blendWeightIdx,
					j->mesh->attributes[blendIndexIdx].format.componentCount,
					m->findAttributeBySemantic(VertexFormat::get("normal").semantic)
				);
		}
	}

	meshes->draw();

/*	const Vec4f colors[4] = {
		Vec4f(1, 0, 0, 1),
		Vec4f(0, 1, 0, 1),
		Vec4f(0, 0, 1, 1),
		Vec4f(1, 1, 0, 1)
	};

	// Joint visualization
	for(size_t i=0; i<skeleton->basePose.jointCount(); ++i)
	{
		const size_t childIdx = i;
		const size_t parentIdx = skeleton->parents[i];

		if(childIdx == parentIdx)
			continue;

		Vec3f p1 = visualizePose.transforms[childIdx].translation();
		Vec3f p2 = visualizePose.transforms[parentIdx].translation();

		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glColor4fv(colors[i % 4].data);

		glBegin(GL_LINES);
			glVertex3fv(p1.data);
			glVertex3fv(p2.data);
		glEnd();

		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
	}*/

	glPopMatrix();
}

}	// namespace MCD
