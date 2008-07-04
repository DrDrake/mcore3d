#include "Pch.h"
#include "../../../MCD/Core/System/RawFileSystem.h"
#include <stdexcept>

using namespace MCD;

TEST(RawFileSystemTest)
{
	RawFileSystem fs(L"./");

	// Queries
	CHECK(fs.getRoot() == Path::getCurrentPath());
	CHECK(fs.isExists(L"."));
	CHECK(fs.isDirectory(L"."));

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
}
