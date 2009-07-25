#include "Pch.h"
#include "../MCD/Binding/Binding.h"
#include "../MCD/Binding/Entity.h"
#include "../MCD/Binding/InputComponent.h"
#include "../MCD/Binding/ScriptComponentManager.h"
#include "../MCD/Binding/System.h"
#include "../MCD/Core/Entity/Entity.h"
#include "../MCD/Core/System/FileSystemCollection.h"
#include "../MCD/Core/System/RawFileSystem.h"
#include "../MCD/Render/ChamferBox.h"
#include "../MCD/Render/Effect.h"
#include "../MCD/Render/Mesh.h"
#include "../MCD/Component/Render/EntityPrototypeLoader.h"
#include "../MCD/Component/Render/MeshComponent.h"
#include "../MCD/Component/Input/WinMessageInputComponent.h"
#include "../Test/RenderTest/BasicGlWindow.h"
#include "../Test/RenderTest/DefaultResourceManager.h"

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

//! Call a script function when a list of resources were loaded.
class ResourceLoadCallback : public ResourceManagerCallback
{
public:
	sal_override void doCallback()
	{
		if(onLoaded.isAssigned())
			onLoaded.call();
	}

	script::Event<void> onLoaded;
};	// Callback

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=Launcher;width=800;height=600;fullscreen=0;FSAA=4"),
		fileSystem(*createDefaultFileSystem()),
		mResourceManager(fileSystem),
		mScriptComponentManager(fileSystem)
	{
		MCD_ASSERT(TestWindow::singleton == nullptr);
		TestWindow::singleton = this;

		FileSystemCollection& fs = dynamic_cast<FileSystemCollection&>(fileSystem);
		fs.addFileSystem(*(new RawFileSystem(L"")));

		// Override the default loader of *.3ds file
		mResourceManager.addFactory(new EntityPrototypeLoaderFactory(mResourceManager));

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
			MeshComponent* c = new MeshComponent();
			c->mesh = mesh;
			c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
			e->addComponent(c);

			e.release();
		}

		{	// Setup input component
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"Input";
			e->asChildOf(mRootNode);

			WinMessageInputComponent* c = new WinMessageInputComponent();
			mInputComponent = c;
			c->attachTo(*this);
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

		mScriptComponentManager.updateScriptComponents();

		BehaviourComponent::traverseEntities(mRootNode);
		RenderableComponent::traverseEntities(mRootNode);
	}

// Script usage:
	void scriptBindingSetup();

	sal_notnull Entity* rootNode() {
		return mRootNode;
	}

	sal_notnull Entity* loadEntity(const wchar_t* filePath) {
		Entity* e = new Entity();
		EntityPrototypeLoader::addEntityAfterLoad(e, mResourceManager, filePath);
		return e;
	}

	sal_notnull InputComponent* inputComponent() {
		return mInputComponent;
	}

	sal_notnull IResourceManager* resourceManager() {
		return &mResourceManager;
	}

	static TestWindow* getSinleton() {
		return TestWindow::singleton;
	}

	static TestWindow* singleton;

	//! Perform callback when the supplied list of resource get loaded
	static int addCallback(HSQUIRRELVM vm)
	{
		script::detail::StackHandler sa(vm);
		int nparams = sa.getParamCount();

		static const wchar_t errorMsg[] = L"addCallback() expecting a string or an array of string as parameter";

		if(nparams < 1)
			return sa.throwError(errorMsg);

		return 1;
	}

// Attributes:
	Entity* mRootNode;
	InputComponent* mInputComponent;

	IFileSystem& fileSystem;
	DefaultResourceManager mResourceManager;
	ScriptComponentManager mScriptComponentManager;
};	// TestWindow

TestWindow* TestWindow::singleton = nullptr;

namespace script {

SCRIPT_CLASS_DECLAR(TestWindow);
SCRIPT_CLASS_REGISTER_NAME(TestWindow, "MainWindow")
	.enableGetset(L"MainWindow")
	.method<objNoCare>(L"_getrootEntity", &TestWindow::rootNode)
	.method(L"loadEntity", &TestWindow::loadEntity)
	.method<objNoCare>(L"_getinputComponent", &TestWindow::inputComponent)
	.method<objNoCare>(L"_getresourceManager", &TestWindow::resourceManager)
	.rawMethod(L"addCallback", &TestWindow::addCallback)
;}

SCRIPT_CLASS_DECLAR(ResourceLoadCallback);
SCRIPT_CLASS_REGISTER(ResourceLoadCallback)
	.declareClass<ResourceLoadCallback, IResourceManagerCallback>(L"ResourceLoadCallback")
	.constructor()
	.scriptEvent(L"onLoaded", &ResourceLoadCallback::onLoaded)
;}

}	// namespace script

void TestWindow::scriptBindingSetup()
{
	mScriptComponentManager.registerRootEntity(*mRootNode);

	using namespace script;

	VMCore* v = (VMCore*)sq_getforeignptr(HSQUIRRELVM(mScriptComponentManager.vm.getImplementationHandle()));

	script::RootDeclarator root(v);
	root.declareFunction<objNoCare>(L"_getMainWindow", &TestWindow::getSinleton);

	script::ClassTraits<TestWindow>::bind(v);
	script::ClassTraits<ResourceLoadCallback>::bind(v);

	// Setup some global variable for easy access in script.
	mScriptComponentManager.vm.runScript(
		L"gMainWindow <- _getMainWindow();\n"

		L"function loadEntity(filePath) { return gMainWindow.loadEntity(filePath); }\n"

		L"gInput <- gMainWindow.inputComponent;\n"

		L"resourceManager <- gMainWindow.resourceManager;\n"

		L"function addResourceCallback(filePaths, functor) {\n"
		L"	local callback = ResourceLoadCallback();\n"
		L"	if(typeof filePaths == \"string\")\n"
		L"		callback.addDependency(filePaths);\n"
		L"	else foreach(path in filePaths)\n"
		L"		callback.addDependency(path);\n"
		L"	callback.onLoaded().setHandler(functor);\n"
		L"	resourceManager.addCallback(callback);\n"
		L"}\n"
	);

	// TODO: Let user supply a command line argument to choose the startup script
	mScriptComponentManager.doFile(L"init.nut", true);
}

int main()
{
	TestWindow window;
	window.scriptBindingSetup();
	window.mainLoop();

	return 0;
}
