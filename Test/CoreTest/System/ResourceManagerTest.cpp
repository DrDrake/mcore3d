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
	sal_override sal_checkreturn LoadingState load(sal_maybenull std::istream* is)
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

	sal_override ResourcePtr createResource(const Path& fileId) {
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

	{	// After trying to load another resource, a garbage collection take place
		// nothing can be verify in this test, but it will trigger some code to run
		// so that we will have a higher code coverage
		manager.load(L"Main.cpp");
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
