#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Math/AnimationInstance.h"
#include "../../MCD/Core/Math/Quaternion.h"
#include "../../MCD/Core/System/TaskPool.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/PlaneMeshBuilder.h"
#include "../../MCD/Component/Render/AnimationComponent.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Render/SkeletonAnimationComponent.h"
#include "../../MCD/Component/Render/SkinMeshComponent.h"

using namespace MCD;

namespace {

// Create a cylinder alone the z-axis
MeshPtr buildCylinder(float radius, float length, uint8_t jointCount)
{
	const size_t segCount = 10;
	const float diameter = radius * Mathf::cPi() * 2;

	// We make a plan mesh first and then adjust it's vertex to form a cyclinder
	PlaneMeshBuilder builder(diameter, length, segCount, jointCount, false);
	StrideArray<Vec3f> position = builder.getAttributeAs<Vec3f>(builder.posId);
	StrideArray<Vec3f> normal = builder.getAttributeAs<Vec3f>(builder.normalId);

	// Adjust the position and normal
	for(size_t i=0; i<position.size; ++i) {
		// Map the linear range [-diameter/2, diameter/2] into -Pi to Pi degree.
		const float angle = position[i].x / (diameter/2) * Mathf::cPi();
		Mathf::sinCos(angle, normal[i].x, normal[i].y);
		position[i].x = normal[i].x * radius;
		position[i].y = normal[i].y * radius;
		position[i].z += length / 2;
	}

	// Assign the joint index and weight
	// NOTE: All getAttributeAs() should appear after declareAttribute(), otherwise the pointer may become invalid.
	int jointIdxId = builder.declareAttribute(VertexFormat::get("jointIndex"), 2);
	int jointWeightId = builder.declareAttribute(VertexFormat::get("jointWeight"), 2);
	StrideArray<Vec4<uint8_t> > jointIdx = builder.getAttributeAs<Vec4<uint8_t> >(jointIdxId);
	StrideArray<Vec4f> jointWeight = builder.getAttributeAs<Vec4f>(jointWeightId);

	// Re-acquire the position pointer
	position = builder.getAttributeAs<Vec3f>(builder.posId);

	const float jointLength = length / jointCount;

	for(size_t i=0; i<jointIdx.size; ++i) {
		// Find the 4 nearest joints
		float z = position[i].z;// + length / 2;
		int nearestJointIdx = int(z * jointLength);
		jointIdx[i][0] = uint8_t(Math<int>::clamp(nearestJointIdx - 0, 0, jointCount - 1));
		jointIdx[i][1] = uint8_t(Math<int>::clamp(nearestJointIdx - 1, 0, jointCount - 1));
		jointIdx[i][2] = uint8_t(Math<int>::clamp(nearestJointIdx + 1, 0, jointCount - 1));
		jointIdx[i][3] = uint8_t(Math<int>::clamp(nearestJointIdx + 2, 0, jointCount - 1));

		jointWeight[i][0] = 0.5f;
		jointWeight[i][1] = 0.2f;
		jointWeight[i][2] = 0.2f;
		jointWeight[i][3] = 0.1f;
	}

	MeshPtr ret = new Mesh("");
	MCD_VERIFY(ret->create(builder, Mesh::Static));
	return ret;
}

Quaternionf randomQuaternion()
{
	Quaternionf q(Mathf::random() * 2 -1, Mathf::random() * 2 -1, Mathf::random() * 2 -1, Mathf::random() * 2 -1);
	q /= q.length();
	return q;
}

SkeletonAnimationPtr buildSkeletonAnimation(float totalLength, uint8_t jointCount)
{
	SkeletonAnimationPtr skeletonAnimation = new SkeletonAnimation("");
	AnimationTrackPtr track = new AnimationTrack("");

	MCD_ASSUME(totalLength > 0);
	MCD_ASSUME(jointCount > 0);

	const size_t frameCount = 2;
	const size_t subtrackCount = jointCount * 2;
	const float jointLength = totalLength / jointCount;

	{	// Setup the skeleton
		SkeletonPtr skeleton = new Skeleton("");
		skeletonAnimation->skeleton = skeleton;
		skeleton->init(jointCount);

		skeleton->parents[0] = 0;
		for(size_t i=1; i<jointCount; ++i)
			skeleton->parents[i] = i-1;

		skeleton->basePose.init(jointCount);
		for(size_t i=0; i<jointCount; ++i)
			skeleton->basePose.transforms[i].setTranslation(Vec3f(0, 0, i * jointLength));

		skeleton->initBasePoseInverse();
	}

	{	// Setup the skeleton animation
		AnimationTrack::ScopedWriteLock lock(*track);

		// Number of sub-tracks = number of joint * attribute count (which is 2 because of translation and rotation)
		std::vector<size_t> tmp(subtrackCount, frameCount);

		// Use only one frame for translation, since the length of the bone is constant
		for(size_t i=0; i<subtrackCount; ++i)
			if(i % 2 == 0)
				tmp[i] = 1;

		MCD_VERIFY(track->init(StrideArray<const size_t>(&tmp[0], subtrackCount)));

		// Setting up the transform for each joint relative to it's parent joint.
		for(size_t i=0; i<track->subtrackCount(); ++i) {
			AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(i);

			// Setup time
			for(size_t j=0; j<frames.size; ++j)
				frames[j].time = float(j);

			// Setup translation
			if(i % 2 == 0) {
				track->subtracks[i].flag = AnimationTrack::Linear;
				for(size_t j=0; j<frames.size; ++j) {
					reinterpret_cast<Vec3f&>(frames[j]) = Vec3f(0, 0, jointLength);
				}
			}

			// Setup rotation
			else {
				track->subtracks[i].flag = AnimationTrack::Slerp;
				for(size_t j=0; j<frames.size; ++j) {
					Quaternionf& q = reinterpret_cast<Quaternionf&>(frames[j]);
					q.fromAxisAngle(Vec3f::c100, Mathf::cPi() * j / 10);	// Rotate around x-axis anti-clockwise for each key frame
				}
			}
		}
	}

	{	// Setup animation instance
		MCD_VERIFY(skeletonAnimation->anim.addTrack(*track));
	}

	return skeletonAnimation;
}

class TestWindow : public BasicGlWindow
{
	static const size_t cFrameCount = 50;

#ifndef NDEBUG
	static const size_t cTubeCount = 100;
#else
	static const size_t cTubeCount = 1000;
#endif

public:
	TestWindow()
		:
		BasicGlWindow("title=SkeletonAnimationComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		animationTrack = new AnimationTrack("");
		loadAnimationTrack();

		const float cTubeLength = 20;
		const uint8_t cTubeSwegmentCount = 20;

		{	// Setup the tube skin mesh
			MeshPtr mesh = buildCylinder(2, cTubeLength, cTubeSwegmentCount);
			mBasePoseMesh = new Model("BasePose");
			std::auto_ptr<Model::MeshAndMaterial> mm(new Model::MeshAndMaterial);
			mm->mesh = mesh;
			mBasePoseMesh->mMeshes.pushBack(*mm);
			mm.release();
		}

		{	// Setup the skeleton animation
			mSkeletonAnimation = buildSkeletonAnimation(cTubeLength, cTubeSwegmentCount);
		}

		{	// Setup the dynamic group
			mDynamicGroup = new Entity;
			mDynamicGroup->asChildOf(&mRootNode);
		}

		{	// Setup the animation updater
			std::auto_ptr<Entity> e(new Entity);
			e->asChildOf(&mRootNode);
			animationUpdater = new AnimationUpdaterComponent(&taskPool);
			e->addComponent(animationUpdater.get());
			e.release();
		}

		{	// Setup the skeleton animation updater
			std::auto_ptr<Entity> e(new Entity);
			e->asChildOf(&mRootNode);
			skeletonAnimationUpdater = new SkeletonAnimationUpdaterComponent(&taskPool);
			e->addComponent(skeletonAnimationUpdater.get());
			e.release();
		}

		// Create some tubes
		for(size_t i=0; i<cTubeCount; ++i)
			createARandomTube();
	}

	void createTube(const Vec3f& position, float initialAnimationTime)
	{
		if(!mDynamicGroup)
			return;

		EntityPtr e1(new Entity);	// e1 determine the general position
		EntityPtr e2(new Entity);	// e2 holds node animation
		EntityPtr e3(new Entity);	// e3 holds skeleton animation and skin mesh

		e1->asChildOf(mDynamicGroup.getNotNull());
		e2->asChildOf(e1.getNotNull());
		e3->asChildOf(e2.getNotNull());

		e1->localTransform.setTranslation(position);

		{	AnimationComponent* c = new AnimationComponent(*animationUpdater);
			e2->addComponent(c);
			MCD_VERIFY(c->animationInstance.addTrack(*animationTrack));
			c->animationInstance.time = initialAnimationTime;
		}

		// Setup skin mesh and skeleton animation components
		{
			SkeletonAnimationComponent* sa = new SkeletonAnimationComponent(*skeletonAnimationUpdater);
			e3->addComponent(sa);
			sa->pose.init(mSkeletonAnimation->skeleton->basePose.jointCount());
			sa->skeletonAnimation.anim = mSkeletonAnimation->anim;
			sa->skeletonAnimation.anim.time = initialAnimationTime;
			sa->skeletonAnimation.skeleton = mSkeletonAnimation->skeleton;

			SkinMeshComponent* sm = new SkinMeshComponent();
			e3->addComponent(sm);
			MCD_VERIFY(sm->init(mResourceManager, *mBasePoseMesh));
			sm->skeleton = mSkeletonAnimation->skeleton;
			sm->skeletonAnimation = sa;
		}

		taskPool.setThreadCount(1);
	}

	void createARandomTube()
	{
		Vec3f pos(Mathf::random(), Mathf::random(), Mathf::random());
		pos = (pos * 2 - 1) * cTubeCount/10;

#ifndef NDEBUG
		pos *= 10;
#endif

		createTube(pos, Mathf::random() * cFrameCount);
	}

	void destroyARandomTube()
	{
		// Search for a random Entity to remove
		Entity* e = mDynamicGroup->firstChild();
		size_t idx = rand() % cTubeCount;

		while(idx--) {
			if(!e) return;
			e = e->nextSibling();
		}
		Entity::destroy(e);
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		if(mTimer.get().asSecond() > 2) {
			loadAnimationTrack();
			mTimer.reset();
		}

		for(size_t i=0; i<cTubeCount/10; ++i) {
			createARandomTube();
			destroyARandomTube();
		}

		mRootNode.localTransform.setTranslation(Vec3f(0, 0, -200));
		RenderableComponent::traverseEntities(&mRootNode);
		BehaviourComponent::traverseEntities(&mRootNode, deltaTime);
	}

	void loadAnimationTrack()
	{
		// Manually creat the animation track
		AnimationTrack::ScopedWriteLock lock(*animationTrack);

		const size_t cSubtrackCount = AnimationComponent::subtrackPerEntity;
		size_t tmp[cSubtrackCount] = { cFrameCount };
		for(size_t i=0; i<cSubtrackCount; ++i)
			tmp[i] = cFrameCount;

		MCD_VERIFY(animationTrack->init(StrideArray<const size_t>(tmp, cSubtrackCount)));

		// Assign the time of each frame
		for(size_t i=0; i<cSubtrackCount; ++i) {
			AnimationTrack::KeyFrames frames = animationTrack->getKeyFramesForSubtrack(i);
			for(size_t j=0; j<frames.size; ++j)
				frames[j].time = float(j);
		}

		// Setup position animation
		AnimationTrack::KeyFrames frames = animationTrack->getKeyFramesForSubtrack(0);
		for(size_t i=0; i<cFrameCount; ++i) {
			Vec3f position(Mathf::random(), Mathf::random(), Mathf::random());
			position *= 5;
			reinterpret_cast<Vec3f&>(frames[i]) = position;
		}
		reinterpret_cast<Vec3f&>(frames[cFrameCount-1]) = reinterpret_cast<Vec3f&>(frames[0]);

		// Setup rotation animation
		animationTrack->subtracks[1].flag = AnimationTrack::Slerp;
		frames = animationTrack->getKeyFramesForSubtrack(1);
		for(size_t i=0; i<cFrameCount; ++i)
			reinterpret_cast<Quaternionf&>(frames[i]) = randomQuaternion();
		reinterpret_cast<Quaternionf&>(frames[cFrameCount-1]) = reinterpret_cast<Quaternionf&>(frames[0]);

		// Setup scaling animation
		frames = animationTrack->getKeyFramesForSubtrack(2);
		for(size_t i=0; i<cFrameCount; ++i) {
			Vec3f scale(Mathf::random() * 2);
			reinterpret_cast<Vec3f&>(frames[i]) = scale;
		}
		reinterpret_cast<Vec3f&>(frames[cFrameCount-1]) = reinterpret_cast<Vec3f&>(frames[0]);
	}

	AnimationUpdaterComponentPtr animationUpdater;
	SkeletonAnimationUpdaterComponentPtr skeletonAnimationUpdater;

	Entity mRootNode;
	EntityPtr mDynamicGroup;	//!< We will add/remove entities under this
	Timer mTimer;	// Simulate a reload of the AnimationTrack every few seconds.
	AnimationTrackPtr animationTrack;
	SkeletonAnimationPtr mSkeletonAnimation;
	ModelPtr mBasePoseMesh;
	DefaultResourceManager mResourceManager;
	TaskPool taskPool;
};	// TestWindow

}	// namespace

TEST(SkeletonAnimationComponentTest)
{
	TestWindow window;
	window.mainLoop();
}
