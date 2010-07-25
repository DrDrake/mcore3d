#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Math/AnimationInstance.h"
#include "../../MCD/Core/Math/Quaternion.h"
#include "../../MCD/Render/Animation.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Renderer.h"

using namespace MCD;

namespace {

class AnimationTestFixture
{
	static const size_t cFrameCount = 50;
	static const size_t cBoxCount = 1000;

public:
	AnimationTestFixture()
	{
		framework.initWindow("title=AnimationComponentTest;width=800;height=600;fullscreen=0;FSAA=4");

		animationTrack = new AnimationTrack("");
		loadAnimationTrack();

		mBoxesNode = framework.sceneLayer().addChild(new Entity("Boxes"));

		{	// Setup the chamfer box mesh
			mesh = new Mesh("");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
			MCD_VERIFY(mesh->create(chamferBoxBuilder, Mesh::Static));
		}

		for(size_t i=0; i<cBoxCount; ++i)
			createARandomBox();
	}

	void createBox(const Vec3f& position, float initialAnimationTime)
	{
		Entity& system = framework.systemEntity();

		// Setup entity 1
		Entity* e1 = mBoxesNode->addChild(new Entity);
		Entity* e2 = e1->addChild(new Entity);

		e1->localTransform.setTranslation(position);
		e2->localTransform = Mat44f(Mat33f::makeXYZRotation(0, Mathf::cPiOver4(), 0));

		{	// Add component
			MeshComponent2* c = e2->addComponent(new MeshComponent2);
			c->mesh = mesh;
		}

		{	AnimationUpdaterComponent* updater = system.findComponentInChildrenExactType<AnimationUpdaterComponent>();
			MCD_ASSUME(updater);
			AnimationComponent* c = e2->addComponent(new AnimationComponent(*updater));
			MCD_VERIFY(c->animationInstance.addTrack(*animationTrack));
			c->animationInstance.time = initialAnimationTime;
		}
	}

	void createARandomBox()
	{
		Vec3f pos(Mathf::random(), Mathf::random(), Mathf::random());
		pos = (pos * 2 - 1) * cBoxCount/10;
		createBox(pos, Mathf::random() * cFrameCount);
	}

	void destroyARandomBox()
	{
		// Search for a random Entity to remove
		Entity* e = mBoxesNode->firstChild();
		size_t idx = rand() % cBoxCount;

		while(idx--) {
			if(!e) return;
			e = e->nextSibling();
		}
		Entity::destroy(e);
	}

	Quaternionf randomQuaternion()
	{
		Quaternionf q(Mathf::random() * 2 -1, Mathf::random() * 2 -1, Mathf::random() * 2 -1, Mathf::random() * 2 -1);
		q /= q.length();
		return q;
	}

	void loadAnimationTrack()
	{
		// Manually creat the animation track
		AnimationTrack::ScopedWriteLock lock(*animationTrack);

		const size_t cSubtrackCount = AnimationComponent::subtrackPerEntity;
		std::vector<size_t> tmp(cSubtrackCount, cFrameCount);

		MCD_VERIFY(animationTrack->init(StrideArray<const size_t>(&tmp[0], cSubtrackCount)));

		// Assign the position of each frame
		for(size_t i=0; i<cSubtrackCount; ++i) {
			AnimationTrack::KeyFrames frames = animationTrack->getKeyFramesForSubtrack(i);
			for(size_t j=0; j<frames.size; ++j)
				frames[j].pos = float(j);
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

	Framework framework;
	EntityPtr mBoxesNode;
	MeshPtr mesh;
	Timer mTimer;	// Simulate a reload of the AnimationTrack every few seconds.
	AnimationTrackPtr animationTrack;
};	// TestWindow

}	// namespace

TEST_FIXTURE(AnimationTestFixture, Render)
{
	DeltaTimer deltaTimer;
	Entity& root = framework.rootEntity();
	RendererComponent* renderer = root.findComponentInChildrenExactType<RendererComponent>();
	CameraComponent2* sceneCamera = root.findComponentInChildrenExactType<CameraComponent2>();

	sceneCamera->entity()->localTransform.translateBy(Vec3f(0, 0, 200));

	while(true)
	{
		Event e;
		framework.update(e);

		if(e.Type == Event::Closed)
			break;

		if(mTimer.get().asSecond() > 2) {
			loadAnimationTrack();
			mTimer.reset();
		}

		if(Mathf::random() > 0.002)
			createARandomBox();
		if(Mathf::random() > 0.002)
			destroyARandomBox();

		const float dt = (float)deltaTimer.getDelta().asSecond();
		BehaviourComponent::traverseEntities(&root, dt);
		renderer->render(root);
	}
}

struct EventResult {
	AnimationComponentPtr c;
	size_t virtualFrameIdx;
	size_t data;
};

static std::vector<EventResult> gEventCallbackResult;

TEST(Event_AnimationComponentTest)
{
	Entity e1, e2;

	AnimationUpdaterComponentPtr updater = new AnimationUpdaterComponent(nullptr);
	AnimationComponentPtr c = new AnimationComponent(*updater);

	e1.addComponent(updater.get());
	e2.addComponent(c.get());

	AnimationTrackPtr track = new AnimationTrack("track");
	CHECK(c->animationInstance.addTrack(*track, 1, 1, "wtrack"));

	{	AnimationTrack::ScopedWriteLock lock(*track);
		size_t tmp[] = { 3 };
		CHECK(track->init(StrideArray<const size_t>(tmp, 1)));

		AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(0);
		frames[0].pos = 0;
		frames[1].pos = 1;
		frames[2].pos = 2;
	}

	struct LocalClass {
		static void callback(AnimationComponent& c, size_t virtualFrameIdx, void* data) {
			EventResult result = { &c, virtualFrameIdx, size_t(data) };
			gEventCallbackResult.push_back(result);
		}
	};	// LocalClass
	c->defaultCallback = &LocalClass::callback;

	c->setEdgeEvent("noSuchTrack", 0, nullptr);	// Do nothing if the weighted track's name is not found
	c->setEdgeEvent("wtrack", 0, (void*)size_t(10));
	c->setEdgeEvent("wtrack", 2, (void*)size_t(20));

	gEventCallbackResult.clear();

	while(gEventCallbackResult.empty()) {
		updater->update(0);
	}

	CHECK_EQUAL(c, gEventCallbackResult[0].c);
	CHECK_EQUAL(0u, gEventCallbackResult[0].virtualFrameIdx);
	CHECK_EQUAL(10u, gEventCallbackResult[0].data);
}
