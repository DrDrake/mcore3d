#include "Pch.h"
#include "../../../MCD/Core/System/RawFileSystem.h"
#include "../../../MCD/Core/System/RawFileSystemMonitor.h"
#include "../../../MCD/Core/System/ZipFileSystem.h"
#include <stdexcept>

using namespace MCD;

TEST(RawFileSystemTest)
{
	RawFileSystem fs("./");

	// Queries
	CHECK(fs.getRoot() == Path::getCurrentPath());
	CHECK(fs.isExists("."));
	CHECK(fs.isDirectory("."));
	CHECK_THROW(fs.isDirectory("./__not_exist__"), std::runtime_error);

	CHECK_THROW(fs.getSize("./"), std::runtime_error);

	// Making directory
	CHECK(fs.makeDir("a/b/c"));	// Intermediate directories will be created as well
	CHECK(!fs.makeDir("a"));

	// Remove directory
#if !defined(MCD_WIN32)
	// Recursive remove not implemented yet for POSIX system
	CHECK(fs.remove("a/b/c"));
	CHECK(fs.remove("a/b"));
#endif
	CHECK(fs.remove("a"));
	CHECK(!fs.remove("a"));

	// Open file
	{	std::auto_ptr<std::ostream> os = fs.openWrite("ha.txt");
		CHECK(os.get() != nullptr);
		*os << "Hello!";
	}

	{	std::auto_ptr<std::istream> is = fs.openRead("ha.txt");
		CHECK(is.get() != nullptr);
	}

	{	// Ok to use absolute path
		std::auto_ptr<std::istream> is = fs.openRead(Path::getCurrentPath() / "ha.txt");
		CHECK(is.get() != nullptr);
	}

	// Check the last write time
	CHECK(fs.getLastWriteTime("ha.txt") > 0);

	// Remove file
	CHECK(fs.remove("ha.txt"));

	// Set a new root which not exist
	CHECK(!fs.setRoot("__not_exist__"));
}

#ifdef MCD_VC

TEST(RawFileSystemMonitorTest)
{
	RawFileSystemMonitor monitor("./", true);

	RawFileSystem fs("./");

	{	std::auto_ptr<std::ostream> os = fs.openWrite("1.txt");
		*os << "abcd";
	}
	{	std::auto_ptr<std::ostream> os = fs.openWrite("2.txt");
		*os << "abcd";
	}

	CHECK_EQUAL("1.txt", monitor.getChangedFile());
	CHECK_EQUAL("2.txt", monitor.getChangedFile());
	CHECK(fs.remove("1.txt"));
	CHECK(fs.remove("2.txt"));
}

TEST(RawFileSystemListingTest)
{
	RawFileSystem fs("./");

	{	void* c = fs.openFirstChildFolder("");
		Path p;
		do {
			p = fs.getNextSiblingFolder(c);
		} while(!p.getString().empty());
		fs.closeFirstChildFolder(c);
	}

	{	void* c = fs.openFirstFileInFolder("");
		Path p;
		do {
			p = fs.getNextFileInFolder(c);
		} while(!p.getString().empty());
		fs.closeFirstFileInFolder(c);
	}

	CHECK(true);
}

#endif	// MCD_VC

TEST(ZipFileSystemTest)
{
	{	// Non-existing zip file
		CHECK_THROW(ZipFileSystem("__not_exist__.zip"), std::runtime_error);
	}

	{	// Failed setRoot
		ZipFileSystem fs("./TestData/test.zip");

		// Failure of setRoot will not alter the state of the zip file system
		CHECK(!fs.setRoot("__not_exist__.zip"));
		CHECK(fs.getRoot() == Path::getCurrentPath() / "TestData/test.zip");
		CHECK(fs.isDirectory("welcome"));
	}

	ZipFileSystem fs("./TestData/test.zip");

	// Queries
	CHECK(fs.getRoot() == Path::getCurrentPath() / "TestData/test.zip");
	CHECK(!fs.isExists("."));
	CHECK(fs.isExists("./welcome"));
	CHECK(fs.isDirectory("welcome"));
	CHECK(fs.isDirectory("./welcome"));
	CHECK(!fs.isDirectory("Pch.h"));
	CHECK_THROW(fs.isDirectory("./__not_exist__"), std::runtime_error);

	CHECK_EQUAL(112u, fs.getSize("Pch.h"));

	{	// Open file
		std::auto_ptr<std::istream> is = fs.openRead("welcome/hello.txt");
		CHECK(is.get() != nullptr);
		std::string tmp;

		std::stringstream ss;
		*is >> ss.rdbuf();

		// Note that the stream doesn't perform line ending conversion
		CHECK_EQUAL("How are you?\r\nI am fine!", ss.str());
	}

	{	// Open non-existing item and then existing item
		std::auto_ptr<std::istream> is = fs.openRead("__not_exist__.txt");
		CHECK(!is.get());

		is = fs.openRead("welcome/hello.txt");
		CHECK(is.get());
	}

	// Check the last write time
	CHECK(fs.getLastWriteTime("Pch.h") > 0);

	// Set a new root which not exist
	CHECK(!fs.setRoot("__not_exist__"));

	{	// Lising files
		void* c = fs.openFirstFileInFolder("");
		CHECK_EQUAL(std::string("Pch.h"), fs.getNextFileInFolder(c).getString());
		CHECK(fs.getNextFileInFolder(c).getString().empty());
		fs.closeFirstFileInFolder(c);

		c = fs.openFirstFileInFolder("./");
		CHECK_EQUAL(std::string("Pch.h"), fs.getNextFileInFolder(c).getString());
		CHECK(fs.getNextFileInFolder(c).getString().empty());
		fs.closeFirstFileInFolder(c);

		c = fs.openFirstFileInFolder("welcome");
		CHECK_EQUAL(std::string("welcome/hello.txt"), fs.getNextFileInFolder(c).getString());
		CHECK(fs.getNextFileInFolder(c).getString().empty());
		fs.closeFirstFileInFolder(c);

		c = fs.openFirstFileInFolder("./welcome");
		CHECK_EQUAL(std::string("welcome/hello.txt"), fs.getNextFileInFolder(c).getString());
		CHECK(fs.getNextFileInFolder(c).getString().empty());
		fs.closeFirstFileInFolder(c);
	}
}

#include "../../../MCD/Core/System/TaskPool.h"
#include <stdlib.h>

namespace {

AtomicInteger gCounter = 0;

class ReadTask : public MCD::TaskPool::Task
{
public:
	ReadTask(ZipFileSystem& zfs) : Task(0), fs(zfs) {}

protected:
	sal_override void run(Thread& thread) throw()
	{
		// Make the test more throught by randomly holding the life-time of ZipFileSystem's impl
		std::auto_ptr<std::istream> holder = (rand() % 2 == 0) ? fs.openRead("welcome/hello.txt") : std::auto_ptr<std::istream>(nullptr);

		for(size_t i=0; i<10; ++i) {
			// Open file
			std::auto_ptr<std::istream> is = fs.openRead("welcome/hello.txt");
			std::string tmp;

			std::stringstream ss;
			*is >> ss.rdbuf();

			MCD_ASSERT(ss.str() == "How are you?\r\nI am fine!");

			gCounter++;
		}

		delete this;
	}

	ZipFileSystem& fs;
};	// ReadTask

}	// namespace

TEST(MultiThread_ZipFileSystemTest)
{
	ZipFileSystem fs("./TestData/test.zip");

	TaskPool taskPool;
	taskPool.setThreadCount(3);

	for(size_t i=0; i<10; ++i)
		CHECK(taskPool.enqueue(*new ReadTask(fs)));

	// Wait until all tasks finish
	while(gCounter != 100)
		mSleep(0);
}
