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
	ZipFileSystem fs("./TestData/test.zip");

	// Queries
	CHECK(fs.getRoot() == Path::getCurrentPath() / "TestData/test.zip");
	CHECK(!fs.isExists("."));
	CHECK(fs.isExists("./welcome"));
	CHECK(fs.isDirectory("welcome"));
	CHECK(fs.isDirectory("./welcome"));
	CHECK_THROW(fs.isDirectory("./__not_exist__"), std::runtime_error);

	CHECK_EQUAL(112u, fs.getSize("Pch.h"));

	// Open file
	{	std::auto_ptr<std::istream> is = fs.openRead("welcome/hello.txt");
		CHECK(is.get() != nullptr);
		std::string tmp;

		std::stringstream ss;
		*is >> ss.rdbuf();

		// Note that the stream doesn't perform line ending conversion
		CHECK_EQUAL("How are you?\r\nI am fine!", ss.str());
	}

	// Check the last write time
	CHECK(fs.getLastWriteTime("Pch.h") > 0);

	// Set a new root which not exist
	CHECK(!fs.setRoot("__not_exist__"));
}
