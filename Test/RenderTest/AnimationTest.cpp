#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Math/AnimationState.h"
#include "../../MCD/Core/Math/Quaternion.h"
#include "../../MCD/Render/Animation.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Renderer.h"

using namespace MCD;

namespace {

class AnimationTestFixture
{
	static const size_t keyCount = 50;
	static const size_t cBoxCount = 1000;

public:
	AnimationTestFixture()
	{
		MCD_VERIFY(framework.initWindow("title=AnimationComponentTest;width=800;height=600;fullscreen=0;FSAA=4"));

		animationClip = new AnimationClip("");
		loadAnimationClip();

		mBoxesNode = framework.sceneLayer().addFirstChild("Boxes");

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
		// Setup entity 1
		Entity* e1 = mBoxesNode->addFirstChild(new Entity);
		Entity* e2 = e1->addFirstChild(new Entity);

		e1->localTransform.setTranslation(position);
		e2->localTransform = Mat44f(Mat33f::makeXYZRotation(0, Mathf::cPiOver4(), 0));

		{	// Add component
			MeshComponent* c = e2->addComponent(new MeshComponent);
			c->mesh = mesh;
		}

		{	AnimationComponent* c = e2->addComponent(new AnimationComponent);
			c->animations.resize(1);
			AnimationState& a = c->animations[0];
			a.clip = animationClip;
			a.worldRefTime = AnimationUpdaterComponent::worldTime() + initialAnimationTime;
		}
	}

	void createARandomBox()
	{
		Vec3f pos(Mathf::random(), Mathf::random(), Mathf::random());
		pos = (pos * 2 - 1) * cBoxCount/10;
		createBox(pos, Mathf::random() * keyCount);
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

	void loadAnimationClip()
	{
		// Manually creat the animation track
		const size_t cTrackCount = 3;
		std::vector<size_t> tmp(cTrackCount, keyCount);

		MCD_VERIFY(animationClip->init(StrideArray<const size_t>(&tmp[0], cTrackCount)));

		// Assign the position of each frame
		for(size_t i=0; i<cTrackCount; ++i) {
			AnimationClip::Keys keys = animationClip->getKeysForTrack(i);
			for(size_t j=0; j<keys.size; ++j)
				keys[j].pos = float(j);
		}

		// Setup position animation
		AnimationClip::Keys keys = animationClip->getKeysForTrack(0);
		for(size_t i=0; i<keyCount; ++i) {
			Vec3f position(Mathf::random(), Mathf::random(), Mathf::random());
			position *= 5;
			keys[i].cast<Vec3f>() = position;
		}
		keys[keyCount-1].v = keys[0].v;

		// Setup rotation animation
		animationClip->tracks[1].flag = AnimationClip::Slerp;
		keys = animationClip->getKeysForTrack(1);
		for(size_t i=0; i<keyCount; ++i)
			keys[i].cast<Quaternionf>() = randomQuaternion();
		keys[keyCount-1].v = keys[0].v;

		// Setup scaling animation
		keys = animationClip->getKeysForTrack(2);
		for(size_t i=0; i<keyCount; ++i) {
			Vec3f scale(Mathf::random() * 2);
			keys[i].cast<Vec3f>() = scale;
		}
		keys[keyCount-1].v = keys[0].v;
	}

	Framework framework;
	EntityPtr mBoxesNode;
	MeshPtr mesh;
	Timer mTimer;	// Simulate a reload of the AnimationClip every few seconds.
	AnimationClipPtr animationClip;
};	// TestWindow

}	// namespace

TEST_FIXTURE(AnimationTestFixture, Render)
{
	Entity& root = framework.rootEntity();
	RendererComponent* renderer = root.findComponentInChildrenExactType<RendererComponent>();
	CameraComponent* sceneCamera = root.findComponentInChildrenExactType<CameraComponent>();

	sceneCamera->entity()->localTransform.translateBy(Vec3f(0, 0, 200));

	while(true)
	{
		Event e;
		framework.update(e);

		if(e.Type == Event::Closed)
			break;

		if(mTimer.get().asSecond() > 2) {
			loadAnimationClip();
			mTimer.reset();
		}

		if(Mathf::random() > 0.002)
			createARandomBox();
		if(Mathf::random() > 0.002)
			destroyARandomBox();

		renderer->render(root);
	}
}
