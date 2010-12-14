#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Sprite.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"

using namespace MCD;

// A simple AI for the bear
class RandomWalkAI : public BehaviourComponent
{
public:
	RandomWalkAI(Vec2f bound_, AnimationBlendTree::ClipNode* n)
		: bound(bound_), elaspedTime(-1), walkingSpeed(1), clipNode(n)
	{}

	sal_override void update(float dt) {
		elaspedTime += dt;

		Mat44f& m = entity()->localTransform;

		m.translateBy(Vec3f(walkingSpeed * dt, 0, 0));
		m.setScale(Vec3f(walkingSpeed > 0 ?-1.0f : 1, 1, 1));

		if(elaspedTime > 3 || elaspedTime < 0) {
			const float maxSpeed = 100;
			if(Mathf::random() > 0.5f || elaspedTime < 0)
				walkingSpeed = ((Mathf::random() - 0.5f) * 2) * maxSpeed;
			if(fabs(walkingSpeed) < 10)
				walkingSpeed = 0;
			clipNode->state.rate = (walkingSpeed == 0) ? 0 : 1.0f;
			elaspedTime = 0;
		}

		const Vec3f trans = m.translation();
		if(trans.x < 0) {
			m.setTranslation(Vec3f(0, trans.y, trans.z));
			walkingSpeed *= -1;
		}
		if(trans.x > bound.x) {
			m.setTranslation(Vec3f(bound.x, trans.y, trans.z));
			walkingSpeed *= -1;
		}
	}

	Vec2f bound;
	float elaspedTime;
	float walkingSpeed;	// Can be negative
	AnimationBlendTree::ClipNode* clipNode;
};	// RandomWalkAI

class SpriteTestFixture
{
public:
	SpriteTestFixture()
	{
		::srand(ticksSinceMachineStartup() & 0xFFFFFFFF);

		MCD_VERIFY(framework.addFileSystem("Media"));
		MCD_VERIFY(framework.initWindow("title=SpriteTest;width=800;height=600;fullscreen=0;FSAA=8"));

		{	// Setup the animation clip
			animationClip = new AnimationClip("");

			const size_t cTrackCount = 1;
			const size_t keyCount = 9;	// 9 Keys to give a 8 frame loop
			std::vector<size_t> tmp(cTrackCount, keyCount);

			MCD_VERIFY(animationClip->init(StrideArray<const size_t>(&tmp[0], cTrackCount)));
			animationClip->framerate = 12;
			animationClip->length = 8;
			animationClip->tracks[0].flag = AnimationClip::Step;

			// Assign the position of each frame
			for(size_t i=0; i<cTrackCount; ++i) {
				AnimationClip::Keys keys = animationClip->getKeysForTrack(i);
				for(size_t j=0; j<keys.size; ++j)
					keys[j].pos = float(j);
			}

			// Setup texture rectangle animation
			AnimationClip::Keys keys = animationClip->getKeysForTrack(0);
			const float w = 233;
			const float h = 145;
			Array<float, 9> x = {   1,   1, 1,   1, 235, 235, 235, 235, -100 };
			Array<float, 9> y = { 147, 439, 1, 293, 293,   1, 439, 147, -100 };
			for(size_t i=0; i<keys.size; ++i)
				keys[i].v = Vec4f(x[i], y[i], x[i]+w, y[i]+h);
		}

		{	// Setup bear sprite atlas, all bear sprite share the same atlas.
			bears = framework.guiLayer().addFirstChild("Bears");
			SpriteAtlasComponent* atlas = bears->addComponent(new SpriteAtlasComponent);

			// Texture source from:
			// http://www.raywenderlich.com/1271/how-to-use-animations-and-sprite-sheets-in-cocos2d
			ResourceManager& resourceManager = framework.resourceManager();
			atlas->textureAtlas = resourceManager.loadAs<Texture>("AnimBear.png");
		}
	}

	EntityPtr createBear()
	{
		Entity* e = new Entity("Bear");

		SimpleAnimationComponent* anim = e->addComponent(new SimpleAnimationComponent);
		AnimationBlendTree::ClipNode* clipNode = new AnimationBlendTree::ClipNode;
		clipNode->state.clip = animationClip;
		anim->blendTree.nodes.push_back(clipNode);

		SpriteComponent* sprite = e->addComponent(new SpriteComponent);
		sprite->animation = anim;
		sprite->anchor = Vec2f(0.5f, 0.5f);

		// Randomize the color
		sprite->color = ColorRGBAf(Mathf::random()*0.5f+0.5f, Mathf::random()*0.5f+0.5f, Mathf::random()*0.5f+0.5f, 1);

		const float w = 233;
		const float h = 145;
		sprite->width = w;
		sprite->height = h;
		sprite->textureRect = Vec4f(1, 1, 1+w, 1+h);

		const Vec2f bound(800, 600);

		e->addComponent(new RandomWalkAI(bound, clipNode));

		// Randomize the initial position
		e->localTransform = Mat44f::makeTranslation(
			Vec3f(Mathf::random() * bound.x, Mathf::random() * bound.y, Mathf::random())
		);

		// Randomize the initial size
		const float size = 0.5f + Mathf::random()/2;
		e->localTransform.scaleBy(Vec3f(size));

		return e;
	}

	Framework framework;
	EntityPtr bears;
	AnimationClipPtr animationClip;
};	// SpriteTestFixture

TEST_FIXTURE(SpriteTestFixture, Render)
{
	for(size_t i=0; i<100; ++i)
		bears->addFirstChild(createBear());

	framework.mainLoop();
	CHECK(true);
}
