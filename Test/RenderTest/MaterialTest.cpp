#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"

using namespace MCD;

TEST(MaterialTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow("title=MaterialTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			// Setup the chamfer box mesh
			mMesh = new Mesh("");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
			MCD_VERIFY(mMesh->create(chamferBoxBuilder, Mesh::Static));

			mEffect = static_cast<Effect*>(mResourceManager.load("Material/test.fx.xml").get());
		}

		sal_override void update(float deltaTime)
		{
			(void)deltaTime;
			mResourceManager.processLoadingEvents();

			glTranslatef(0.0f, 0.0f, -5.0f);

			Material* material = nullptr;
			if(mEffect && (material = mEffect->material.get()) != nullptr) {
				for(size_t i=0; i<material->getPassCount(); ++i) {
					material->preRender(i);
					mMesh->draw();
					material->postRender(i);
				}
			}
		}

		MCD::MeshPtr mMesh;
		EffectPtr mEffect;
		DefaultResourceManager mResourceManager;
	};	// TestWindow

	{
		TestWindow window;
		window.mainLoop();
	}
}
