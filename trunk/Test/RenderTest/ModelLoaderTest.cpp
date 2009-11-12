#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/EditableMesh.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/System/ResourceLoader.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include <fstream>

using namespace MCD;

TEST(ModelLoaderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=ModelLoaderTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mAngle(0)
		{
			std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./Media/"));
			mResourceManager.reset(new DefaultResourceManager(*fs));
			fs.release();
		}

		void loadModel(const wchar_t* fileId)
		{
			const wchar_t* args = L"editable=true";
			mModel = dynamic_cast<Model*>(mResourceManager->load(fileId, IResourceManager::NonBlock, 0, args).get());
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager->processLoadingEvents();

			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			const float scale = 0.2f;
			glScalef(scale, scale, scale);

			if(!mModel)
				return;

			mModel->draw();

			// verify the effect of 'editable=true' arg
			for(Model::MeshAndMaterial* mnm = mModel->mMeshes.begin();
				mnm != mModel->mMeshes.end(); 
				mnm = mnm->next())
			{
				EditableMesh* eMesh = dynamic_cast<EditableMesh*>(mnm->mesh.get());
				MCD_ASSERT(eMesh && eMesh->builder != nullptr);
				(void)eMesh;	// Remove warning in RELEASE mode
			}
		}

		ModelPtr mModel;
		float mAngle;

		std::auto_ptr<DefaultResourceManager> mResourceManager;
	};	// TestWindow

	{
		TestWindow window;

//		window.loadModel(L"testbox.mesh");
		window.loadModel(L"Scene/City/scene.3ds");

		window.mainLoop();
	}

	CHECK(true);
}
