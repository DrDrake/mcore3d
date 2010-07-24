#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Component/Render/EntityPrototypeLoader.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Render/Mesh.h"

using namespace MCD;

namespace NormalMappingTest {

class TestWindow : public BasicGlWindow
{
private:
	PrefabPtr mPrefab;
	EffectPtr mEffect;

public:
	TestWindow()
		:
		BasicGlWindow("title=NormalMappingTest;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		this->mCameraVelocity = 0.5f;
		this->mCamera.frustum.near = 0.01f;
		this->mCamera.frustum.setFov(60);

		// load normal mapping effect
		mEffect = static_cast<Effect*>(mResourceManager.load("Material/normalmapping.fx.xml").get());

		{	// Override the default loader of *.3ds file
			mResourceManager.addFactory(new EntityPrototypeLoaderFactory(mResourceManager));

			// TODO: Remove the use of Entity in render test
			mPrefab = dynamic_cast<Prefab*>(mResourceManager.load("Scene/City/scene.3ds", IResourceManager::NonBlock, 0, "loadAsEntity=true;tangents=true").get());
		}

		{	// Setup entity 1
			std::auto_ptr<Entity> e(new Entity);
			e->name = "ChamferBox 1";
			e->asChildOf(&mRootNode);
			e->localTransform = Mat44f(Mat33f::makeXYZRotation(0, Mathf::cPiOver4(), 0));

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh("");
			MCD_VERIFY(mesh->create(ChamferBoxBuilder(0.4f, 3, true), Mesh::Static));

			// Add component
			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = mEffect;
			e->addComponent(c);

			e.release();
		}

		{	// Setup entity 2
			std::auto_ptr<Entity> e(new Entity);
			e->name = "Sphere 1";
			e->asChildOf(&mRootNode);
			e->localTransform.setTranslation(Vec3f(1, 0, 0));

			// Setup the chamfer box mesh as a shpere
			MeshPtr mesh = new Mesh("");
			MCD_VERIFY(mesh->create(ChamferBoxBuilder(1.0f, 5, true), Mesh::Static));

			// Add component
			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = mEffect;
			e->addComponent(c);

			e.release();
		}
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		if(mPrefab)
		{
			glPushMatrix();
			glScalef(0.01f, 0.01f, 0.01f);

			// NOTE: The material in mEffect may be null, if mEffect haven't commit yet.
			Material* mat = mEffect->material.get();
			if(mat) for(size_t i=0; i<mat->getPassCount(); ++i)
			{
				mat->preRender(i);
				RenderableComponent::traverseEntities(mPrefab->entity.get());
				mat->postRender(i);
			}
			else
			{
				RenderableComponent::traverseEntities(mPrefab->entity.get());
			}

			glPopMatrix();
		}

		RenderableComponent::traverseEntities(&mRootNode);
	}

	Entity mRootNode;

	DefaultResourceManager mResourceManager;
};	// TestWindow

}	// namespace NormalMappingTest

TEST(NormalMappingTest)
{
	NormalMappingTest::TestWindow window;
	window.mainLoop();
}
