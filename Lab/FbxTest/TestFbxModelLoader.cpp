#include "Pch.h"

#include "../../Test/RenderTest/BasicGlWindow.h"
#include "../../Test/RenderTest/DefaultResourceManager.h"

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
	}

	sal_override void update(float deltaTime)
	{
		mResMgr.processLoadingEvents();

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