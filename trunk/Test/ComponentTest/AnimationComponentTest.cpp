#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Math/AnimationInstance.h"
#include "../../MCD/Core/Math/Quaternion.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Render/AnimationComponent.h"

using namespace MCD;

namespace {

class TestWindow : public BasicGlWindow
{
	static const size_t cFrameCount = 50;

public:
	TestWindow()
		:
		BasicGlWindow(L"title=AnimationComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		animationTrack = new AnimationTrack(L"");
		loadAnimationTrack();

		{	// Setup the chamfer box mesh
			mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);
		}

		const size_t boxCount = 1000;
		for(size_t i=0; i<boxCount; ++i) {
			Vec3f pos(Mathf::random(), Mathf::random(), Mathf::random());
			pos = (pos * 2 - 1) * boxCount/10;
			createBox(pos, Mathf::random() * cFrameCount);
		}
	}

	void createBox(const Vec3f& position, float initialAnimationTime)
	{
		// Setup entity 1
		std::auto_ptr<Entity> e1(new Entity);
		std::auto_ptr<Entity> e2(new Entity);

		e1->asChildOf(&mRootNode);
		e2->asChildOf(e1.get());

		e1->localTransform.setTranslation(position);
		e2->localTransform = Mat44f(Mat33f::makeXYZRotation(0, Mathf::cPiOver4(), 0));

		{	// Add component
			MeshComponent* c = new MeshComponent;
			e2->addComponent(c);
			c->mesh = mesh;
		}

		{	AnimationComponent* c = new AnimationComponent(animationThread);
			e2->addComponent(c);
			c->animationInstance.addTrack(*animationTrack);
			c->animationInstance.time = initialAnimationTime;
		}

		e1.release();
		e2.release();
	}

	Quaternionf randomQuaternion()
	{
		Quaternionf q(Mathf::random(), Mathf::random(), Mathf::random(), Mathf::random());
		q /= q.length();
		return q;
	}

	sal_override void update(float deltaTime)
	{
		GLfloat lightPos[] = { 200, 200, 200, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
		mResourceManager.processLoadingEvents();

		if(mTimer.get().asSecond() > 2) {
			loadAnimationTrack();
			mTimer.reset();
		}

		mRootNode.localTransform.setTranslation(Vec3f(0, 0, -200));
		RenderableComponent::traverseEntities(&mRootNode);
		BehaviourComponent::traverseEntities(&mRootNode, float(mDeltaTimer.getDelta().asSecond()));
	}

	void loadAnimationTrack()
	{
		// Manually creat the animation track
		animationTrack->acquireWriteLock();

		MCD_VERIFY(animationTrack->init(cFrameCount, 3));

		for(size_t i=0; i<animationTrack->keyframeCount(); ++i)
			animationTrack->keyframeTimes[i] = float(i);

		// Setup position animation
		AnimationTrack::KeyFrames frames = animationTrack->getKeyFramesForSubtrack(0);
		for(size_t i=0; i<cFrameCount; ++i) {
			Vec3f position(Mathf::random(), Mathf::random(), Mathf::random());
			position *= 5;
			reinterpret_cast<Vec3f&>(frames[i]) = position;
		}
		reinterpret_cast<Vec3f&>(frames[cFrameCount-1]) = reinterpret_cast<Vec3f&>(frames[0]);

		// Setup rotation animation
		animationTrack->subtrackFlags[1] = AnimationTrack::Slerp;
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

		animationTrack->releaseWriteLock();
	}

	// The animation thread must destroy after mRootNode.
	AnimationThread animationThread;
	Entity mRootNode;
	MeshPtr mesh;
	Timer mTimer;	// Simulate a reload of the AnimationTrack every few seconds.
	DeltaTimer mDeltaTimer;
	AnimationTrackPtr animationTrack;
	DefaultResourceManager mResourceManager;
};	// TestWindow

}	// namespace

TEST(AnimationComponentTest)
{
	TestWindow window;
	window.mainLoop();
}
