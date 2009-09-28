#include "Pch.h"

#include "../../Test/RenderTest/BasicGlWindow.h"
#include "../../Test/RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Math/Mat44.h"
#include "../Fbx/FbxModelLoader.h"

using namespace MCD;

namespace TestFbxModelLoader
{

class TestApp : public BasicGlWindow
{
public:
	DefaultResourceManager mResMgr;
	ModelPtr mModel;

	TestApp()
		: BasicGlWindow( L"title=TestFbxModelLoader;width=800;height=600;FSAA=1" )
		, mResMgr(*createDefaultFileSystem())
	{
		mCamera.moveForward(-10.0f);

		mResMgr.addFactory(new FbxModelLoaderFactory(mResMgr));

		// open fbx file
		std::wstring fbxfilepath = (Path::getCurrentPath() / L"media/CornellBox.fbx").getString();

		//std::wstring fbxfilepath = (Path::getCurrentPath() / L"media/scene02_cave_master.fbx").getString();

		mModel = dynamic_cast<Model*>(mResMgr.load(fbxfilepath).get());

		GLfloat ambientLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	}

	sal_override void update(float deltaTime)
	{
		mResMgr.processLoadingEvents();

		// NOTE: this should be set per-frame since the light position
		// stored in opengl is view-dependent
		GLfloat lightPos[] = { 200, 200, 200, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		// render the triangle buffers
		if(nullptr != mModel)
			mModel->draw();
	}

};	// class TestApp

}	// namespace TestFbxModelLoader

TEST(TestFbxModelLoader)
{
	TestFbxModelLoader::TestApp app;
	app.mainLoop();
}