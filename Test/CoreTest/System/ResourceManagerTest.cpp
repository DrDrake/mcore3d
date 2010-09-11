#include "Pch.h"
#include "../../../MCD/Core/System/RawFileSystem.h"
#include "../../../MCD/Core/System/Resource.h"
#include "../../../MCD/Core/System/ResourceLoader.h"
#include "../../../MCD/Core/System/ResourceManager.h"

using namespace MCD;

namespace {

class FakeLoader : public IResourceLoader
{
protected:
	sal_override sal_checkreturn LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_maybenull const char* args=nullptr)
	{
		return is ? Loaded : Aborted;
	}

	sal_override void commit(Resource&) {}
};	// FakeLoader

class FakeFactory : public ResourceManager::IFactory
{
public:
	FakeFactory(const char* extension) : mExtension(extension) {}

	sal_override ResourcePtr createResource(const Path& fileId, const char* args) {
		if(fileId.getExtension() == mExtension)
			return new Resource(fileId);
		return nullptr;
	}

	sal_override IResourceLoaderPtr createLoader() {
		return new FakeLoader;
	}

	const char* mExtension;
};	// FakeFactory

}	// namespace

TEST(Basic_ResourceManagerTest)
{
	{	// Creating the manager and destroy it immediatly
		std::auto_ptr<IFileSystem> fs(new RawFileSystem("./"));
		ResourceManager manager(*fs);
		fs.release();
	}

	std::auto_ptr<IFileSystem> fs(new RawFileSystem("./"));
	ResourceManager manager(*fs);
	fs.release();

	// Register factory
	manager.addFactory(new FakeFactory("cpp"));

	{	// Even file not found, it still return something.
		CHECK(manager.load("__fileNotFound__.cpp"));

		// Fail if the file extension type not found
		CHECK(!manager.load("__fileNotFound__.xxx"));
	}

	ResourcePtr resource = manager.load("Main.cpp");
	CHECK(resource != nullptr);

	if(!resource)
		return;

	while(true) {	// Poll for event
		IResourceLoaderPtr loader = manager.popEvent();
		if(!loader) continue;

		if(loader->loadingState() != IResourceLoader::Loaded)
			CHECK(false);

		break;
	}

	{	// Get a resource with the same id should return the same resource
		ResourcePtr resource2 = manager.load("Main.cpp");
		CHECK_EQUAL(resource, resource2);

		// And without generating any event since we have a cache hit
		CHECK(!manager.popEvent());
	}

	{	// Try to load another resource
		ResourcePtr resource2 = manager.load("ResourceManadgerTest.cpp", 1);
		CHECK(resource2);
		CHECK(resource != resource2);
		manager.popEvent();
	}

	{	// Load it twice, to trigger the code that handle garbage collection
		CHECK(manager.load("ResourceManadgerTest.cpp", 1));
		manager.popEvent();
		CHECK(manager.load("ResourceManadgerTest.cpp", 1));
	}

	{	// After trying to load another resource, a garbage collection take place
		// nothing can be verify in this test, but it will trigger some code to run
		// so that we will have a higher code coverage
		manager.load("Main.cpp");
		while(manager.popEvent()) {}
	}

	{	// Manually cache a resource
		CHECK(!manager.cache(nullptr));	// Do nothing

		{	// Without fileId collision
			ResourcePtr res = new Resource("abc");
			while(manager.popEvent()) {}
			manager.cache(res);
			const IResourceLoaderPtr e = manager.popEvent();
			CHECK_EQUAL(res, e->resource());
		}

		{	// With fileId collision, returning the old resource
			ResourcePtr res = new Resource("Main.cpp");
			ResourcePtr old = manager.cache(res);
			CHECK_EQUAL(resource, old);
			resource = res;
		}
	}

	{	// Main.cpp should be cached, loading it again should not generate events
		manager.load("Main.cpp", 1);	// Ensure the resource is fully loaded
		while(manager.popEvent()) {}
		manager.load("Main.cpp", 0);
		CHECK(!manager.popEvent());
		manager.load("Main.cpp", 1);
		CHECK(!manager.popEvent());

		// But if we uncache it, then a new resource will be loaded
		CHECK_EQUAL(resource, manager.uncache("Main.cpp"));
		ResourcePtr resource2 = manager.load("Main.cpp", 1);
		manager.popEvent();
		CHECK(resource != resource2);
		resource = resource2;	// Keep the resource for further testing
	}

	{	// Relaoding

		// The same resource object should be returned.
		while(manager.popEvent()) {}
		CHECK_EQUAL(resource, manager.reload("Main.cpp", 1));
		manager.popEvent();

		// But a new resource object will be returned if it is uncached before.
		CHECK_EQUAL(resource, manager.uncache("Main.cpp"));
		ResourcePtr p = manager.reload("Main.cpp");
		CHECK(p && p != resource);
		resource = p;	// Keep the resource for further testing

		CHECK(manager.uncache("ResourceManadgerTest.cpp") == nullptr);
		CHECK(manager.reload("ResourceManadgerTest.cpp") != nullptr);
	}

	{	// Issue a non-blocing load, and then a blocking load on the same resource
		CHECK_EQUAL(resource, manager.uncache("Main.cpp"));
		resource = manager.load("Main.cpp", 0);
		resource = manager.load("Main.cpp", 1);
		CHECK(resource->commitCount() == 1);
	}

	{	// Removal of all loader factories
		manager.removeAllFactory();

		{	// The cached resource are still there
			ResourcePtr resource2 = manager.load("Main.cpp");
			CHECK(resource2 != nullptr);
		}

		// But fail to load anything without the loader factories
		{	// The cached resource are still there
			ResourcePtr resource2 = manager.load("__fileNotFound__.cpp");
			CHECK(resource2 == nullptr);
		}
	}
}

#include "../../../MCD/Core/Entity/Entity.h"
#include "../../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../../MCD/Core/Entity/SystemComponent.h"

namespace {

class MyBehaviourComponent : public BehaviourComponent {
public:
	sal_override void update(float dt) {}
};	// MyBehaviourComponent

}	// namespace

TEST(Callback_ResourceManagerTest)
{
	std::auto_ptr<IFileSystem> fs(new RawFileSystem("./"));
	ResourceManager manager(*fs);
	fs.release();

	// Register factory
	manager.addFactory(new FakeFactory("cpp"));

	Entity e1, e2;
	ResourceManagerComponent* mgrComponent = new ResourceManagerComponent(manager);
	e2.addComponent(mgrComponent);
	MyBehaviourComponent* c = new MyBehaviourComponent;
	e2.addComponent(c);

	const Path testPath("Main.cpp");

	{	// Register before loaded
		mgrComponent->update();
		mgrComponent->registerCallback(testPath, *c, false, 0);
		ResourcePtr resource = manager.load(testPath);
		CHECK(resource);

		e2.enabled = false;
		while(!e2.enabled)
			mgrComponent->update();
	}

	{	// Adding callback where it's already loaded should also work correctly
		ResourcePtr resource = manager.load(testPath, 1);
		CHECK(resource);

		e2.enabled = false;
		mgrComponent->registerCallback(testPath, *c, false, 0);
		while(!e2.enabled)
			mgrComponent->update();
	}

	{	// Adding dependency
		const Path testPath2("Pch.cpp");
		ResourcePtr resource1 = manager.load(testPath, 1);
		ResourcePtr resource2 = manager.load(testPath2, 1);

		IResourceLoaderPtr loader1 = manager.getLoader(testPath);
		IResourceLoaderPtr loader2 = manager.getLoader(testPath2);
		loader1->dependsOn(loader2);

		e2.enabled = false;
		mgrComponent->registerCallback(testPath, *c, true, 0);
		while(!e2.enabled)
			mgrComponent->update();
	}
}

TEST(Negative_ResourceManagerTest)
{
	{	// File type not found (resulting null resource)
		std::auto_ptr<IFileSystem> fs(new RawFileSystem("./"));
		ResourceManager manager(*fs);
		fs.release();
		manager.addFactory(new FakeFactory("txt"));	// FakeFactory will not response to *.cpp
		ResourcePtr resource = manager.load("Main.cpp");
		CHECK(!resource);
	}

	{	// File not found (resulting "Abort" message)
		std::auto_ptr<IFileSystem> fs(new RawFileSystem("./"));
		ResourceManager manager(*fs);
		fs.release();
		manager.addFactory(new FakeFactory("cpp"));
		ResourcePtr resource = manager.load("__fileNotFound__.cpp");

		while(true) {	// Poll for event
			const IResourceLoaderPtr event = manager.popEvent();
			if(!event) continue;

			if(event->loadingState() != IResourceLoader::Aborted)
				CHECK(false);

			break;
		}
	}
}
