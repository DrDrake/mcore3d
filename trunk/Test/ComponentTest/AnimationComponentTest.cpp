#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Math/AnimationInstance.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Render/AnimationComponent.h"

using namespace MCD;

namespace {

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=AnimationComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		{	// Manually creat the animation track
			animationTrack = new AnimationTrack(L"");
			animationTrack->init(5, 2);

			for(size_t i=0; i<animationTrack->keyframeCount(); ++i)
				animationTrack->keyframeTimes[i] = float(i);

			AnimationTrack::KeyFrames frames = animationTrack->getKeyFramesForSubtrack(0);
			reinterpret_cast<Vec3f&>(frames[0]) = Vec3f( 1, 0,  0);
			reinterpret_cast<Vec3f&>(frames[1]) = Vec3f( 0, 0, -1);
			reinterpret_cast<Vec3f&>(frames[2]) = Vec3f(-1, 0,  0);
			reinterpret_cast<Vec3f&>(frames[3]) = Vec3f( 0, 0,  1);
			reinterpret_cast<Vec3f&>(frames[4]) = Vec3f( 1, 0,  0);

			frames = animationTrack->getKeyFramesForSubtrack(1);
			for(size_t i=0; i<5; ++i) {
				// TODO: Generate valid random quaternion.
				Vec4f q(Mathf::random(), Mathf::random(), Mathf::random(), Mathf::random());
				q /= q.length();
				reinterpret_cast<Vec4f&>(frames[0]) = q;
			}
		}

		{	// Setup the chamfer box mesh
			mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);
		}

		const size_t boxCount = 1000;
		for(size_t i=0; i<boxCount; ++i) {
			Vec3f pos(Mathf::random(), Mathf::random(), Mathf::random());
			pos = (pos * 2 - 1) * boxCount/10;;
			createBox(pos, Mathf::random() * 4);
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

		{	AnimationComponent* c = new AnimationComponent;
			e2->addComponent(c);
			c->animationInstance.addTrack(*animationTrack);
			c->animationInstance.time = initialAnimationTime;
		}

		e1.release();
		e2.release();
	}

	sal_override void update(float deltaTime)
	{
		GLfloat lightPos[] = { 200, 200, 200, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
		mResourceManager.processLoadingEvents();

		mRootNode.localTransform.setTranslation(Vec3f(0, 0, -200));
		RenderableComponent::traverseEntities(&mRootNode);
		BehaviourComponent::traverseEntities(&mRootNode);
	}

	Entity mRootNode;
	MeshPtr mesh;
	AnimationTrackPtr animationTrack;
	DefaultResourceManager mResourceManager;
};	// TestWindow

}	// namespace

TEST(AnimationComponentTest)
{
	TestWindow window;
	window.mainLoop();
}
