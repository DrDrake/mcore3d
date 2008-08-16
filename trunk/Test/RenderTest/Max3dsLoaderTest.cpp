#include "Pch.h"
#include "../../MCD/Render/MeshBuilder.h"

using namespace MCD;

#include "../../MCD/Render/Max3dsLoader.h"
#include "../../MCD/Render/DdsLoader.h"
#include "../../MCD/Render/JpegLoader.h"
#include "../../MCD/Render/PngLoader.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/ResourceManager.h"
#include <fstream>

TEST(Sphere_MeshBuilderTest)
{
	class JpegFactory : public ResourceManager::IFactory
	{
	public:
		sal_override ResourcePtr createResource(const Path& path) {
			return new Texture(path);
		}

		sal_override IResourceLoader* createLoader() {
			return new JpegLoader;
		}
	};	// JpegFactory

	class PngFactory : public ResourceManager::IFactory
	{
	public:
		sal_override ResourcePtr createResource(const Path& path) {
			return new Texture(path);
		}

		sal_override IResourceLoader* createLoader() {
			return new PngLoader;
		}
	};	// JpegFactory

	class DdsFactory : public ResourceManager::IFactory
	{
	public:
		sal_override ResourcePtr createResource(const Path& path) {
			return new Texture(path);
		}

		sal_override IResourceLoader* createLoader() {
			return new DdsLoader;
		}
	};	// DdsFactory

	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L""), mAngle(0)
		{
			std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
			mResourceManager.reset(new ResourceManager(*fs));
			fs.release();
			mResourceManager->associateFactory(L"dds", new DdsFactory);
			mResourceManager->associateFactory(L"jpg", new JpegFactory);	
			mResourceManager->associateFactory(L"JPG", new JpegFactory);
			mResourceManager->associateFactory(L"png", new PngFactory);
		}

		bool load3ds(const char* fileName)
		{
			std::ifstream is(fileName, std::ios_base::binary);
			if(!is.is_open())
				return false;

			Max3dsLoader loader(is, mResourceManager.get());
			loader.commit(mModel, MeshBuilder::Static);

			return true;
		}

		void processResourceLoadingEvents()
		{
			while(true) {ResourceManager::Event e = mResourceManager->popEvent();
			if(e.loader) {
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

			const float scale = 0.3f;
			glScalef(scale, scale, scale);

			mModel.draw();
		}

		Model mModel;
		float mAngle;

		std::auto_ptr<ResourceManager> mResourceManager;
	};

	{
		TestWindow window;

//		window.load3ds("titanic.3DS");
//		window.load3ds("titanic2.3DS");
//		window.load3ds("spaceship.3DS");
//		window.load3ds("box.3DS");
		window.load3ds("ship^kiy.3ds");
//		window.load3ds("Alfa Romeo.3ds");
//		window.load3ds("Nissan350Z.3ds");
//		window.load3ds("Nathalie aguilera Boing 747.3DS");
//		window.load3ds("Dog 1 N280708.3ds");
//		window.load3ds("Leon N300708.3DS");
//		window.load3ds("Ford N120208.3ds");
//		window.load3ds("musai.3DS");
//		window.load3ds("Media/House/house.3ds");
//		window.load3ds("FockeWulf 189A/fw189.3ds");

		// Set up and enable light 0
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

		GLfloat lightPos[] = { -50.f, 0.0f, 40.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		window.mainLoop();
	}

	CHECK(true);
}
