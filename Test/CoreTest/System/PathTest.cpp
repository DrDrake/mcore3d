#include "Pch.h"
#include "../../../MCD/Core/System/Path.h"
#include "../../../MCD/Core/System/Utility.h"
#include <stdexcept>

using namespace MCD;

TEST(Basic_PathTest)
{
	{	Path emptyPath;
		CHECK_EQUAL("", emptyPath.getString());
	}

	{	Path path("./");
		CHECK_EQUAL("./", path.getString());
	}

	{	Path path(std::string("./"));
		CHECK_EQUAL("./", path.getString());

		CHECK(path == Path("./"));
		CHECK(!(path != Path("./")));
		CHECK(Path("") < path);
	}
}

TEST(Comparsion_PathTest)
{
	Path p1("abc");
	Path p2("aBc");

	// Case in-sensitive comparison on windows platform
#ifdef MCD_WIN32
	CHECK(p1 == p2);
#else
	CHECK(p1 != p2);
#endif

	Path p3("def");
	CHECK(p1 != p3);
	CHECK(p2 != p3);

	CHECK(Path("1") < Path("2"));
}

TEST(RootName_PathTest)
{
	const char* data[][2] = {
		{"",		""},
		{"",		"./"},
		{"",		"../"},
		{"",		"a/b/c"},
		{"C:",		"C:"},
		{"C:",		"C:/"},
		{"C:",		"C:\\"},
		{"C:",		"C:\\B"},
		{"game:",	"game:/"},	// XBox use game:
		{"game:",	"game:\\"},
		{"http:",	"http://"},
		{"",		"/"},
		{"",		"/home"},
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).getRootName());
}

TEST(RootDirectory_PathTest)
{
	const char* data[][2] = {
		{"",	""},
		{"",	"./"},
		{"",	"../"},
		{"",	"a/b/c"},
		{"/",	"C:"},
		{"/",	"C:/"},
		{"/",	"C:\\"},
		{"/",	"C:\\B"},
		{"/",	"game:/"},
		{"/",	"game:\\"},
		{"/",	"http://"},
		{"/",	"/"},
		{"/",	"/home"},
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).getRootDirectory());
}

TEST(Leaf_PathTest)
{
	const char* data[][2] = {
		{"",		""},
		{"/",		"./"},
		{"/",		"../"},
		{"c",		"a/b/c"},
#ifdef MCD_WIN32
		{"C:",		"C:"},
		{"/",		"C:/"},
		{"\\",		"C:\\"},
		{"B",		"C:/B"},
		{"B",		"C:\\B"},
		{"b",		"C:\\a/b"},
		{"b",		"C:\\a\\b"},
		{"/",		"game:/"},
		{"\\",		"game:\\"},
#endif
		{"/",		"http://"},
		{"/",		"/"},
		{"home",	"/home"},
		{"a.txt",	"a.txt"},
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).getLeaf());
}

TEST(Branch_PathTest)
{
	const char* data[][2] = {
		{"",		""},
		{".",		"./"},
		{"..",		"../"},
		{"a/b",		"a/b/c"},
#ifdef MCD_WIN32
		{"",		"C:"},
		{"C:",		"C:/"},
		{"C:",		"C:\\"},
		{"C:",		"C:\\B"},
		{"game:",	"game:/"},
		{"game:",	"game:\\"},
#endif
		{"http:",	"http://"},
		{"",		"/"},
		{"",		"/home"},
		{"",		"a.txt"},
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).getBranchPath().getString());
}

TEST(GetExtension_PathTest)
{
	const char* data[][2] = {
		{"",		""},
		{"txt",		"a.txt"},
		{"",		"a.txt/"},
		{"txt",		"a.b.txt"},
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).getExtension());
}

TEST(RemoveExtension_PathTest)
{
	const char* data[][2] = {
		{"",		""},
		{"a",		"a.txt"},
		{"a.txt/",	"a.txt/"},
		{"a.b",		"a.b.txt"},
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i) {
		Path p(data[i][1]);
		p.removeExtension();
		CHECK_EQUAL(data[i][0], p.getString());
	}
}

TEST(Normalize_PathTest)
{
	const char* data[][2] = {
		{"",			""},			// Should keep untouched
		{"",			"."},
		{"",			"./"},
		{"C:",			"C:"},
		{"C:/",			"C:\\"},		// Back slash to slash
		{"C:/B",		"C:\\B"},
		{"file:///",	"file:///"},
		{"file:///a",	"file:///a"},
		{"/",			"/"},
		{".a",			".a"},
		{"a.txt",		"a.txt"},
		{"/home",		"/home"},
		{"/home",		"/home/"},		// Remove trailing slash
		{"/home",		"/home/."},	// Collapse .
		{"/home",		"/home/./"},
		{"/home/bar",	"/home/./bar"},
		{"a",			"./a/"},
		{"a/b.txt",		"./a/b.txt"},
		{"",			"././"},
		{"/",			"/home/.."},	// Collapse ..
		{"/",			"/home/../"},
		{"/bar",		"/home/../bar"},
		{"C:/",			"C:/A/B/../.."},
		{"C:/",			"C:\\A\\B/../.."},
		{"..",			".."},
		{"../..",		"../.."},
		{"..",			"../"},
		{"../..",		"../../"},
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i) {
		Path nrm = Path(data[i][1]).normalize();
		CHECK_EQUAL(data[i][0], nrm.getString());

		// Second normalization should do nothing
		CHECK_EQUAL(data[i][0], nrm.normalize().getString());
	}
}

TEST(Append_PathTest)
{
	const char* data[][3] = {
		{"",		"",		""},
		{"",		"./",	""},
		{"/a",		"/",	"a"},
		{"a/b",		"a",	"b"},
		{"a",		"a/b",	"../"},
		{"",		"a/b",	"../../"},
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i)
		CHECK_EQUAL(data[i][0], (Path(data[i][1]) / data[i][2]).getString());

	// Appending a path with root to a non-empty path should throw an error
	CHECK_THROW(Path("a/")/=Path("/"), std::runtime_error);
}

TEST(CurrentPath_PathTest)
{
	// Get and then set the current path (should do nothing)
	Path current = Path::getCurrentPath();
	Path currentBackup = current;
	Path::setCurrentPath(current);
	CHECK(current == currentBackup);

	// Try to set an invalid path
	CHECK_THROW(Path::setCurrentPath("abc:\\"), std::runtime_error);
	// Try to set an non-existing path
	CHECK_THROW(Path::setCurrentPath("./_not_existing"), std::runtime_error);
}

TEST(Iterator_PathIteratorTest)
{
	const char* data[][2] = {
		{"a/b/c",	"a, a/b, a/b/c, "},		// Begin without slash
		{"/a/b/c",	"/a, /a/b, /a/b/c, "},		// Begin with slash
		{"/a/b/c/",	"/a, /a/b, /a/b/c, "},		// End with slash
		{"../../a/",".., ../.., ../../a, "},	// Test with ..
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i) {
		Path path(data[i][0]);
		PathIterator itr(path);

		std::string result;
		while(true) {
			std::string s = itr.next();
			if(s.empty())
				break;
			result += s;
			result += ", ";
		}

		CHECK_EQUAL(data[i][1], result);
	}
}
