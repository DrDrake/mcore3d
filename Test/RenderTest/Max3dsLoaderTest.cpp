#include "Pch.h"
#include "../../MCD/Render/TextureLoaderFactory.h"
#include "../../MCD/Render/Max3dsLoader.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/System/ResourceLoader.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/StrUtility.h"
#include <fstream>

using namespace MCD;

class Max3dsLoaderFactory : public ResourceManager::IFactory
{
public:
	Max3dsLoaderFactory(ResourceManager& resourceManager)
		: mResourceManager(resourceManager)
	{
	}

	sal_override ResourcePtr createResource(const Path& fileId)
	{
		if(wstrCaseCmp(fileId.getExtension().c_str(), L"3ds") == 0)
			return new Model(fileId);
		return nullptr;
	}

	sal_override IResourceLoader* createLoader()
	{
		return new Max3dsLoader(&mResourceManager);
	}

private:
	ResourceManager& mResourceManager;
};	// Max3dsLoaderFactory

TEST(Sphere_MeshBuilderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(), mAngle(0)
		{
			std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./Media/"));
			mResourceManager.reset(new ResourceManager(*fs));
			fs.release();
			mResourceManager->addFactory(new Max3dsLoaderFactory(*mResourceManager));
			mResourceManager->addFactory(new BitmapLoaderFactory);
			mResourceManager->addFactory(new DdsLoaderFactory);
			mResourceManager->addFactory(new JpegLoaderFactory);
			mResourceManager->addFactory(new PngLoaderFactory);
		}

		void load3ds(const wchar_t* fileId)
		{
			mModel = dynamic_cast<Model*>(mResourceManager->load(fileId).get());
		}

		void processResourceLoadingEvents()
		{
			while(true) {
				ResourceManager::Event e = mResourceManager->popEvent();
				if(e.loader) {
					if(e.loader->getLoadingState() == IResourceLoader::Aborted)
						std::wcout << L"Resource:" << e.resource->fileId().getString() << L" failed to load" << std::endl;
					// Allow one resource to commit for each frame
					e.loader->commit(*e.resource);
				} else
					break;
			}
		}

		sal_override void update(float deltaTime)
		{
			processResourceLoadingEvents();

			glTranslatef(0.0f, 0.0f, -50.0f);
			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			const float scale = 1.0f;
			glScalef(scale, scale, scale);

			mModel->draw();
		}

		ModelPtr mModel;
		float mAngle;

		std::auto_ptr<ResourceManager> mResourceManager;
	};

	{
		TestWindow window;

//		window.load3ds(L"titanic.3DS");
//		window.load3ds(L"titanic2.3DS");
//		window.load3ds(L"spaceship.3DS");
//		window.load3ds(L"box.3DS");
//		window.load3ds(L"ship^kiy.3ds");
//		window.load3ds(L"Alfa Romeo.3ds");
//		window.load3ds(L"Nissan350Z.3ds");
//		window.load3ds(L"Nathalie aguilera Boing 747.3DS");
//		window.load3ds(L"Dog 1 N280708.3ds");
//		window.load3ds(L"Leon N300708.3DS");
//		window.load3ds(L"Ford N120208.3ds");
		window.load3ds(L"musai.3DS");
//		window.load3ds(L"Media/House/house.3ds");
//		window.load3ds(L"Media/fw189/fw189.3ds");

		window.mainLoop();
	}

	CHECK(true);
}
