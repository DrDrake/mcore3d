#include "Pch.h"
#include "../MCD/Binding/Binding.h"
#include "../MCD/Binding/Entity.h"
#include "../MCD/Core/Entity/Entity.h"
#include "../MCD/Core/Entity/BehaviourComponent.h"
#include "../MCD/Render/ChamferBox.h"
#include "../MCD/Render/Effect.h"
#include "../MCD/Component/Render/MeshComponent.h"
#include "../Test/RenderTest/BasicGlWindow.h"
#include "../Test/RenderTest/DefaultResourceManager.h"
#include "../3Party/jkbind/Declarator.h"

#ifdef MCD_VC
#	ifndef NDEBUG
#		pragma comment(lib, "jkbindd")
#		pragma comment(lib, "squirreld")
#	else
#		pragma comment(lib, "jkbind")
#		pragma comment(lib, "squirrel")
#	endif
#endif

using namespace MCD;

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=MCD Launcher;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		MCD_ASSERT(TestWindow::singleton == nullptr);

		TestWindow::singleton = this;

		mRootNode = new Entity();
		mRootNode->name = L"root";

		{	// Setup entity 1
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"ChamferBox 1";
			e->asChildOf(mRootNode);
			e->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver4(), 0));

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);
			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

			// Add component
			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
			e->addComponent(c);

			e.release();
		}
	}

	sal_override ~TestWindow()
	{
		// The Entity tree must be destroyed before the script VM.
		delete mRootNode;
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		BehaviourComponent::traverseEntities(mRootNode);
		RenderableComponent::traverseEntities(mRootNode);
	}

// Script usage:
	void scriptBindingSetup();

	sal_notnull Entity* rootNode() {
		return mRootNode;
	}

	static TestWindow* getSinleton() {
		return TestWindow::singleton;
	}

	static TestWindow* singleton;

// Attributes:
	Entity* mRootNode;
	ScriptVM mScriptVM;
	DefaultResourceManager mResourceManager;
};	// TestWindow

TestWindow* TestWindow::singleton = nullptr;

namespace script {

SCRIPT_CLASS_DECLAR(TestWindow);
SCRIPT_CLASS_REGISTER_NAME(TestWindow, "MainWindow")
	.enableGetset(L"MainWindow")
	.method<objNoCare>(L"_getrootEntity", &TestWindow::rootNode)
;}

}	// namespace script

void TestWindow::scriptBindingSetup()
{
	using namespace script;

	VMCore* v = (VMCore*)sq_getforeignptr(HSQUIRRELVM(mScriptVM.getImplementationHandle()));

	script::RootDeclarator root(v);
	root.declareFunction<objNoCare>(L"_getMainWindow", &TestWindow::getSinleton);

	script::ClassTraits<TestWindow>::bind(v);

	// Setup some global variable for easy access in script.
	mScriptVM.runScript(
		L"gMainWindow <- _getMainWindow();"
		L"gRootEntity <- gMainWindow.rootEntity;"

		L"{"
		L"local e = Entity();"
		L"e.name = \"Input\";"
		L"gRootEntity.addChild(e);"
		L"}"
	);
}

int main()
{
	TestWindow window;
	window.scriptBindingSetup();
	window.mainLoop();

	return 0;
}
