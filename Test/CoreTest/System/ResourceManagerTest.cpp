#include "Pch.h"
#include "../../../MCD/Core/System/RawFileSystem.h"
#include "../../../MCD/Core/System/Resource.h"
#include "../../../MCD/Core/System/ResourceLoader.h"
#include "../../../MCD/Core/System/ResourceManager.h"

using namespace MCD;

namespace {

class FakeLoader : public IResourceLoader
{
public:
	FakeLoader() : mState(NotLoaded) {}

protected:
	sal_override sal_checkreturn LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_maybenull const wchar_t* args=nullptr)
	{
		if(!is)
			return mState = Aborted;

		return mState = Loaded;
	}

	sal_override void commit(Resource&) {}

	sal_override LoadingState getLoadingState() const {
		return mState;
	}

	volatile LoadingState mState;
};	// FakeLoader

class FakeFactory : public ResourceManager::IFactory
{
public:
	FakeFactory(const wchar_t* extension) : mExtension(extension) {}

	sal_override ResourcePtr createResource(const Path& fileId, const wchar_t* args) {
		if(fileId.getExtension() == mExtension)
			return new Resource(fileId);
		return nullptr;
	}

	sal_override IResourceLoader* createLoader() {
		return new FakeLoader;
	}

	const wchar_t* mExtension;
};	// FakeFactory

}	// namespace

TEST(Basic_ResourceManagerTest)
{
	{	// Creating the manager and destroy it immediatly
		std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
		ResourceManager manager(*fs);
		fs.release();
	}

	std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
	ResourceManager manager(*fs);
	fs.release();

	// Register factory
	manager.addFactory(new FakeFactory(L"cpp"));

	ResourcePtr resource = manager.load(L"Main.cpp");
	CHECK(resource != nullptr);

	if(!resource)
		return;

	while(true) {	// Poll for event
		ResourceManager::Event event = manager.popEvent();
		if(!event.loader) continue;

		if(event.loader->getLoadingState() != IResourceLoader::Loaded)
			CHECK(false);

		event.loader->commit(*event.resource);
		break;
	}

	{	// Get a resource with the same id should return the same resource
		ResourcePtr resource2 = manager.load(L"Main.cpp");
		CHECK_EQUAL(resource, resource2);

		// And without generating any event since we have a cache hit
		CHECK(!manager.popEvent().resource);
		CHECK(!manager.popEvent().loader);
	}

	{	// Try to load another resource
		ResourcePtr resource2 = manager.load(L"ResourceManadgerTest.cpp", true);
		CHECK(resource != resource2);
		ResourceManager::Event event = manager.popEvent();
		CHECK(event.loader != nullptr);
	}

	{	// Load it twice, to trigger the code that handle garbage collection
		CHECK(manager.load(L"ResourceManadgerTest.cpp", true));
		manager.popEvent();
		CHECK(manager.load(L"ResourceManadgerTest.cpp", true));
	}

	{	// After trying to load another resource, a garbage collection take place
		// nothing can be verify in this test, but it will trigger some code to run
		// so that we will have a higher code coverage
		manager.load(L"Main.cpp");
		while(manager.popEvent().loader) {}
	}

	{	// Manually cache a resource
		manager.cache(nullptr);	// Do nothing

		{	// Without fileId collision
			ResourcePtr res = new Resource(L"abc");
			manager.cache(res);
			ResourceManager::Event e = manager.popEvent();
			CHECK_EQUAL(res.get(), e.resource.get());
			CHECK(!e.loader);
		}

		{	// With fileId collision, returning the old resource
			ResourcePtr res = new Resource(L"Main.cpp");
			ResourcePtr old = manager.cache(res);
			CHECK_EQUAL(resource, old);
			resource = res;
		}
	}

	{	// Main.cpp should be cached, loading it again should not generate events
		while(manager.popEvent().loader) {}
		manager.load(L"Main.cpp", true);
		CHECK(manager.popEvent().loader == nullptr);

		// But if we uncache it, then a new resource will be loaded
		manager.uncache(L"Main.cpp");
		ResourcePtr resource2 = manager.load(L"Main.cpp", true);
		CHECK(manager.popEvent().loader != nullptr);
		CHECK(resource != resource2);
		resource = resource2;	// Keep the resource for further testing
	}

	{	// Relaoding
		while(manager.popEvent().loader) {}
		CHECK_EQUAL(resource, manager.reload(L"Main.cpp", true));
		CHECK(manager.popEvent().loader != nullptr);

		manager.uncache(L"ResourceManadgerTest.cpp");
		CHECK(manager.reload(L"ResourceManadgerTest.cpp") != nullptr);
	}

	{	// Removal of all loader factories
		manager.removeAllFactory();

		{	// The cached resource are still there
			ResourcePtr resource2 = manager.load(L"Main.cpp");
			CHECK(resource2 != nullptr);
		}

		// But fail to load anything without the loader factories
		{	// The cached resource are still there
			ResourcePtr resource2 = manager.load(L"__fileNotFound__.cpp");
			CHECK(resource2 == nullptr);
		}
	}
}

namespace {

class FakeCallback : public ResourceManagerCallback
{
public:
	sal_override void doCallback()
	{
		++count;
	}

	static size_t count;
};	// FakeCallback

size_t FakeCallback::count = 0;

}	// namespace

TEST(Callback_ResourceManagerTest)
{
	std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
	ResourceManager manager(*fs);
	fs.release();

	// Register factory
	manager.addFactory(new FakeFactory(L"cpp"));

	// Create and register callback
	FakeCallback* callback = new FakeCallback;
	callback->addDependency(L"Main.cpp");
	callback->addDependency(L"ResourceManadgerTest.cpp");
	manager.addCallback(callback);

	callback = new FakeCallback;
	callback->addDependency(L"Main.cpp");
	manager.addCallback(callback);

	// Will not trigger any callback
	FakeCallback::count = 0;
	manager.load(L"ResourceManadgerTest.cpp", true);
	manager.doCallbacks(manager.popEvent());
	CHECK_EQUAL(0u, FakeCallback::count);

	// Will trigger the two callbacks
	FakeCallback::count = 0;
	ResourcePtr resource = manager.load(L"Main.cpp", true);
	manager.doCallbacks(manager.popEvent());
	CHECK_EQUAL(2u, FakeCallback::count);

	// Adding a new callback where all dependency is already loaded should also work correctly
	FakeCallback::count = 0;
	callback = new FakeCallback;
	callback->addDependency(L"Main.cpp");
	manager.addCallback(callback);
	manager.doCallbacks(manager.popEvent());
	CHECK_EQUAL(1u, FakeCallback::count);
}

TEST(MajorDependency_Callback_ResourceManagerTest)
{
	std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
	ResourceManager manager(*fs);
	fs.release();

	// Register factory
	manager.addFactory(new FakeFactory(L"cpp"));

	ResourcePtr resource1, resource2;

	// Create and register callback
	{	resource1 = manager.load(L"Main.cpp", true);
		FakeCallback* callback = new FakeCallback;
		callback->setMajorDependency(L"Main.cpp");
		manager.addCallback(callback);
	}

	{	resource2 = manager.load(L"ResourceManadgerTest.cpp", true);
		FakeCallback* callback = new FakeCallback;
		callback->setMajorDependency(L"ResourceManadgerTest.cpp");
		manager.addCallback(callback);
	}

	FakeCallback::count = 0;
	ResourceManager::Event e = manager.popEvent();
	e.loader->commit(*e.resource);
	manager.doCallbacks(e);
	// If addDependency() is used instead of setMajorDependency(),
	// FakeCallback::count will be 2 instead of 1.
	CHECK_EQUAL(1u, FakeCallback::count);

	e = manager.popEvent();
	e.loader->commit(*e.resource);
	manager.doCallbacks(e);
	CHECK_EQUAL(2u, FakeCallback::count);
}

TEST(Negative_ResourceManagerTest)
{
	{	// File type not found (resulting null resource)
		std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
		ResourceManager manager(*fs);
		fs.release();
		manager.addFactory(new FakeFactory(L"txt"));	// FakeFactory will not response to *.cpp
		ResourcePtr resource = manager.load(L"Main.cpp", false, 0);
		CHECK(!resource);
	}

	{	// File not found (resulting "Abort" message)
		std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
		ResourceManager manager(*fs);
		fs.release();
		manager.addFactory(new FakeFactory(L"cpp"));
		ResourcePtr resource = manager.load(L"__fileNotFound__.cpp", false, 0);

		while(true) {	// Poll for event
			ResourceManager::Event event = manager.popEvent();
			if(!event.loader) continue;

			if(event.loader->getLoadingState() != IResourceLoader::Aborted)
				CHECK(false);

			break;
		}
	}
}
