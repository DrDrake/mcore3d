#include "Pch.h"
#include "../../../SGE/Core/System/RawFileSystem.h"
#include "../../../SGE/Core/System/Resource.h"
#include "../../../SGE/Core/System/ResourceLoader.h"
#include "../../../SGE/Core/System/ResourceManager.h"

using namespace SGE;

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
	sal_override ResourcePtr createResource(const Path& path) {
		return new Resource(path);
	}

	sal_override IResourceLoader* createLoader() {
		return new FakeLoader;
	}
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
	manager.associateFactory(L"dummy", new FakeFactory);	// Register
	manager.associateFactory(L"dummy", new FakeFactory);	// Assign an other instance of factory to the same extension
	manager.associateFactory(L"dummy", nullptr);			// Use null to remove the mapping
	manager.associateFactory(L"cpp", new FakeFactory);

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
}

TEST(Negative_ResourceManagerTest)
{
	{	// File type not found (resulting null resource)
		std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
		ResourceManager manager(*fs);
		fs.release();
		manager.associateFactory(L"txt", new FakeFactory);
		ResourcePtr resource = manager.load(L"Main.cpp", false, 0);
	}

	{	// File not found (resulting "Abort" message)
		std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
		ResourceManager manager(*fs);
		fs.release();
		manager.associateFactory(L"cpp", new FakeFactory);
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
