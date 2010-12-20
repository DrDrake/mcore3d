#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Math/Quaternion.h"
#include "../../MCD/Render/Animation.h"
#include "../../MCD/Render/TransformAnimator.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Mesh.h"

using namespace MCD;

namespace {

class AnimationTestFixture
{
	static const size_t keyCount = 10;
	static const size_t clipDuration = 2;
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
			createRandomBox();
	}

	void createBox(const Vec3f& position, float initialAnimationTime)
	{
		// Setup entity 1
		Entity* e1 = mBoxesNode->addFirstChild(new Entity);
		Entity* e2 = e1->addFirstChild(new Entity);

		e1->localTransform.setTranslation(position);

		{	// Add component
			MeshComponent* c = e2->addComponent(new MeshComponent);
			c->mesh = mesh;
		}

		{	SimpleAnimationComponent* c = e2->addComponent(new SimpleAnimationComponent);
			{	AnimationBlendTree::ClipNode* n = new AnimationBlendTree::ClipNode;
				n->state.clip = animationClip;
				c->blendTree.nodes.push_back(n);
				n->localRefTime = AnimationUpdaterComponent::worldTime() - initialAnimationTime;
			}

			TransformAnimator* c2 = e2->addComponent(new TransformAnimator);
			c2->affectingEntities.push_back(e2);
			c2->animation = c;
			c2->trackOffset = 0;
			c2->trackPerEntity = 3;
		}
	}

	void createRandomBox()
	{
		Vec3f pos(Mathf::random(), Mathf::random(), Mathf::random());
		pos = (pos * 2 - 1) * cBoxCount/10;
		createBox(pos, Mathf::random() * animationClip->length);
	}

	void destroyRandomBox()
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
		animationClip->framerate = 30;
		animationClip->length = animationClip->framerate * clipDuration;

		// Assign the position of each frame
		for(size_t i=0; i<cTrackCount; ++i) {
			AnimationClip::Keys keys = animationClip->getKeysForTrack(i);
			for(size_t j=0; j<keys.size; ++j)
				keys[j].pos = float(j) * animationClip->length / keyCount;
		}

		// Setup position animation
		AnimationClip::Keys keys = animationClip->getKeysForTrack(0);
		for(size_t i=0; i<keyCount; ++i) {
			Vec3f position(Mathf::random(), Mathf::random(), Mathf::random());
			keys[i].cast<Vec3f>() = position * 5;
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
			Vec3f scale(Mathf::random() * 3);
			keys[i].cast<Vec3f>() = scale;
		}
		keys[keyCount-1].v = keys[0].v;
	}

	Framework framework;
	EntityPtr mBoxesNode;
	MeshPtr mesh;
	Timer mTimer;	// Simulate a reload of the AnimationClip every few seconds.
	AnimationClipPtr animationClip;
};	// AnimationTestFixture

}	// namespace

TEST_FIXTURE(AnimationTestFixture, Render)
{
	Entity& root = framework.rootEntity();
	CameraComponent* sceneCamera = root.findComponentInChildrenExactType<CameraComponent>();

	sceneCamera->entity()->localTransform.translateBy(Vec3f(0, 0, 200));

	while(true)
	{
		if(mTimer.get().asSecond() > 2) {
			loadAnimationClip();
			mTimer.reset();
		}

		if(Mathf::random() > 0.002)
			createRandomBox();
		if(Mathf::random() > 0.002)
			destroyRandomBox();

		Event e;
		framework.update(e);

		if(e.Type == Event::Closed)
			break;
	}
}
