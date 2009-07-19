#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/TangentSpaceBuilder.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Render/EntityPrototype.h"
#include "../../MCD/Render/Mesh.h"

using namespace MCD;

namespace NormalMappingTest {

class TestWindow : public BasicGlWindow
{
private:
	EntityPrototypePtr mEntProto;
	EffectPtr mEffect;

public:
	TestWindow()
		:
		BasicGlWindow(L"title=NormalMappingTest;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		this->mCameraVelocity = 0.5f;
		this->mCamera.frustum.near = 0.01f;
		this->mCamera.frustum.setFov(60);

		// load normal mapping effect
		mEffect = static_cast<Effect*>(mResourceManager.load(L"Material/normalmapping.fx.xml").get());

		{
			//mEntProto = dynamic_cast<EntityPrototype*>(mResourceManager.load(L"3M00696/buelllightning.3DS").get());
			mEntProto = dynamic_cast<EntityPrototype*>(mResourceManager.load(L"Scene/City/scene.3ds", false, 0, L"tangents=true").get());
		}
		{	// Setup entity 1
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"ChamferBox 1";
			e->asChildOf(&mRootNode);
			e->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver4(), 0));

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
			TangentSpaceBuilder().compute(chamferBoxBuilder, Mesh::TextureCoord0, Mesh::TextureCoord1);

			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

			// Add component
			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = mEffect;
			e->addComponent(c);

			e.release();
		}

		{	// Setup entity 2
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"Sphere 1";
			e->asChildOf(&mRootNode);
			e->localTransform.setTranslation(Vec3f(1, 0, 0));

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
			TangentSpaceBuilder().compute(chamferBoxBuilder, Mesh::TextureCoord0, Mesh::TextureCoord1);
			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

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

		if(mEntProto)
		{
			glPushMatrix();
			glScalef(0.01f, 0.01f, 0.01f);

			// NOTE: The material in mEffect may be null, if mEffect haven't commit yet.
			Material2* mat = mEffect->material.get();
			if(mat) for(size_t i=0; i<mat->getPassCount(); ++i)
			{
				mat->preRender(i);
				//mEntProto->draw();
				RenderableComponent::traverseEntities(mEntProto->entity.get());
				mat->postRender(i);
			}
			else
			{
				RenderableComponent::traverseEntities(mEntProto->entity.get());
			}

			glPopMatrix();
		}

		glTranslatef(0.0f, -10.0f, 0.0f);

		RenderableComponent::traverseEntities(&mRootNode);
	}

	Entity mRootNode;

	DefaultResourceManager mResourceManager;
};	// TestWindow

}	// namespace NormalMappingTest

#include "../../MCD/Core/System/MemoryProfiler.h"

TEST(NormalMappingTest)
{
	MemoryProfiler::singleton().setEnable(true);

	{	NormalMappingTest::TestWindow window;
		window.mainLoop();

		std::string s = MemoryProfiler::singleton().defaultReport(30);
		std::cout << s << std::endl;
	}

	std::string s = MemoryProfiler::singleton().defaultReport(30);
	std::cout << s << std::endl;
}
