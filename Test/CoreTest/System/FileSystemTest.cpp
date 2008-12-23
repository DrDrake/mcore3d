#include "Pch.h"
#include "../../../MCD/Core/System/RawFileSystem.h"
#include "../../../MCD/Core/System/RawFileSystemMonitor.h"
#include "../../../MCD/Core/System/ZipFileSystem.h"
#include <stdexcept>

using namespace MCD;

TEST(RawFileSystemTest)
{
	RawFileSystem fs(L"./");

	// Queries
	CHECK(fs.getRoot() == Path::getCurrentPath());
	CHECK(fs.isExists(L"."));
	CHECK(fs.isDirectory(L"."));
	CHECK_THROW(fs.isDirectory(L"./__not_exist__"), std::runtime_error);

	CHECK_THROW(fs.getSize(L"./"), std::runtime_error);

	// Making directory
	fs.makeDir(L"a/b/c");	// Intermediate directories will be created as well
	CHECK_THROW(fs.makeDir(L"a"), std::runtime_error);

	// Remove directory
	fs.remove(L"a");
	CHECK_THROW(fs.remove(L"a"), std::runtime_error);

	// Open file
	{	std::auto_ptr<std::ostream> os = fs.openWrite(L"ha.txt");
		CHECK(os.get() != nullptr);
		*os << "Hello!";
	}

	{	std::auto_ptr<std::istream> is = fs.openRead(L"ha.txt");
		CHECK(is.get() != nullptr);
	}

	{	// Ok to use absolute path
		std::auto_ptr<std::istream> is = fs.openRead(Path::getCurrentPath() / L"ha.txt");
		CHECK(is.get() != nullptr);
	}

	// Check the last write time
	CHECK(fs.getLastWriteTime(L"ha.txt") > 0);

	// Remove file
	fs.remove(L"ha.txt");

	// Set a new root which not exist
	CHECK(!fs.setRoot(L"__not_exist__"));
}

#ifdef MCD_VC

TEST(RawFileSystemMonitorTest)
{
	RawFileSystemMonitor watcher(L"./", true);

	RawFileSystem fs(L"./");

	{	std::auto_ptr<std::ostream> os = fs.openWrite(L"1.txt");
		*os << "abcd";
	}
	{	std::auto_ptr<std::ostream> os = fs.openWrite(L"2.txt");
		*os << "abcd";
	}

	CHECK_EQUAL(L"1.txt", watcher.getChangedFile());
	CHECK_EQUAL(L"2.txt", watcher.getChangedFile());
	fs.remove(L"1.txt");
	fs.remove(L"2.txt");
}

#endif	// MCD_VC

TEST(ZipFileSystemTest)
{
	ZipFileSystem fs(L"./TestData/test.zip");

	// Queries
	CHECK(fs.getRoot() == Path::getCurrentPath() / L"TestData/test.zip");
	CHECK(!fs.isExists(L"."));
	CHECK(fs.isExists(L"./welcome"));
	CHECK(fs.isDirectory(L"welcome"));
	CHECK(fs.isDirectory(L"./welcome"));
	CHECK_THROW(fs.isDirectory(L"./__not_exist__"), std::runtime_error);

	CHECK_EQUAL(112, fs.getSize(L"Pch.h"));

	// Open file
	{	std::auto_ptr<std::istream> is = fs.openRead(L"welcome/hello.txt");
		CHECK(is.get() != nullptr);
		std::string tmp;

		std::stringstream ss;
		*is >> ss.rdbuf();

		// Note that the stream doesn't perform line ending conversion
		CHECK_EQUAL("How are you?\r\nI am fine!", ss.str());
	}

	// Check the last write time
	CHECK(fs.getLastWriteTime(L"Pch.h") > 0);

	// Set a new root which not exist
	CHECK(!fs.setRoot(L"__not_exist__"));
}
