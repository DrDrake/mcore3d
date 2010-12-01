#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Render/Sprite.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"

using namespace MCD;

TEST(SpriteTest)
{
	Framework framework;
	CHECK(framework.addFileSystem("Media"));
	CHECK(framework.initWindow("title=DisplayListTest;width=800;height=600;fullscreen=0;FSAA=8"));

	ResourceManager& resourceManager = framework.resourceManager();
	Entity& gui = framework.guiLayer();

	AnimationClipPtr animationClip = new AnimationClip("");

	{	// Setup the animation clip
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

	{
		Entity* e = gui.addFirstChild("Bear");
		e->localTransform.scaleBy(Vec3f(-1, 1, 1));
		e->localTransform.translateBy(Vec3f(200, 200, 0));
		e->localTransform.rotateBy(Vec3f(0, 0, 1), 3.14159f/4);
		SpriteAtlasComponent* atlas = e->addComponent(new SpriteAtlasComponent);

		SimpleAnimationComponent* anim = e->addComponent(new SimpleAnimationComponent);
		{	AnimationBlendTree::ClipNode* n = new AnimationBlendTree::ClipNode;
			n->state.clip = animationClip;
			anim->blendTree.nodes.push_back(n);
		}

		// Texture source from:
		// http://www.raywenderlich.com/1271/how-to-use-animations-and-sprite-sheets-in-cocos2d
		atlas->textureAtlas = resourceManager.loadAs<Texture>("AnimBear.png");

		SpriteComponent* sprite = e->addComponent(new SpriteComponent);
		sprite->animation = anim;
		sprite->anchor = Vec2f(0.5f, 0.5f);

		const float w = 233;
		const float h = 145;
		sprite->width = w;
		sprite->height = h;
		sprite->textureRect = Vec4f(1, 1, 1+w, 1+h);
	}

	framework.mainLoop();
	CHECK(true);
}
