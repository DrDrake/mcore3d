#include "Pch.h"
#include "../../../MCD/Core/System/Path.h"
#include <stdexcept>

using namespace MCD;

TEST(Basic_PathTest)
{
	{	Path emptyPath;
		CHECK_EQUAL(L"", emptyPath.getString());
	}

	{	Path path(L"./");
		CHECK_EQUAL(L"./", path.getString());
	}

	{	Path path(std::wstring(L"./"));
		CHECK_EQUAL(L"./", path.getString());

		CHECK(path == Path(L"./"));
		CHECK(!(path != Path(L"./")));
		CHECK(Path(L"") < path);
	}
}

TEST(RootName_PathTest)
{
	const wchar_t* data[][2] = {
		{L"",		L""},
		{L"",		L"./"},
		{L"",		L"../"},
		{L"",		L"a/b/c"},
		{L"C:",		L"C:"},
		{L"C:",		L"C:/"},
		{L"C:",		L"C:\\"},
		{L"C:",		L"C:\\B"},
		{L"game:",	L"game:/"},	// XBox use game:
		{L"game:",	L"game:\\"},
		{L"http:",	L"http://"},
		{L"",		L"/"},
		{L"",		L"/home"},
	};

	for(size_t i=0; i<sizeof(data)/sizeof(const wchar_t*)/2; ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).getRootName());
}

TEST(RootDirectory_PathTest)
{
	const wchar_t* data[][2] = {
		{L"",	L""},
		{L"",	L"./"},
		{L"",	L"../"},
		{L"",	L"a/b/c"},
		{L"/",	L"C:"},
		{L"/",	L"C:/"},
		{L"/",	L"C:\\"},
		{L"/",	L"C:\\B"},
		{L"/",	L"game:/"},
		{L"/",	L"game:\\"},
		{L"/",	L"http://"},
		{L"/",	L"/"},
		{L"/",	L"/home"},
	};

	for(size_t i=0; i<sizeof(data)/sizeof(const wchar_t*)/2; ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).getRootDirectory());
}

TEST(Leaf_PathTest)
{
	const wchar_t* data[][2] = {
		{L"",		L""},
		{L"/",		L"./"},
		{L"/",		L"../"},
		{L"c",		L"a/b/c"},
		{L"C:",		L"C:"},
		{L"/",		L"C:/"},
		{L"\\",		L"C:\\"},
		{L"\\B",	L"C:\\B"},
		{L"/",		L"game:/"},
		{L"\\",		L"game:\\"},
		{L"/",		L"http://"},
		{L"/",		L"/"},
		{L"home",	L"/home"},
		{L"a.txt",	L"a.txt"},
	};

	for(size_t i=0; i<sizeof(data)/sizeof(const wchar_t*)/2; ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).getLeaf());
}

TEST(Branch_PathTest)
{
	const wchar_t* data[][2] = {
		{L"",		L""},
		{L".",		L"./"},
		{L"..",		L"../"},
		{L"a/b",	L"a/b/c"},
		{L"",		L"C:"},
		{L"C:",		L"C:/"},
		{L"C:",		L"C:\\"},
		{L"C:",		L"C:\\B"},
		{L"game:",	L"game:/"},
		{L"game:",	L"game:\\"},
		{L"http:",	L"http://"},
		{L"",		L"/"},
		{L"",		L"/home"},
		{L"",		L"a.txt"},
	};

	for(size_t i=0; i<sizeof(data)/sizeof(const wchar_t*)/2; ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).getBranchPath().getString());
}

TEST(GetExtension_PathTest)
{
	const wchar_t* data[][2] = {
		{L"",		L""},
		{L"txt",	L"a.txt"},
		{L"",		L"a.txt/"},
		{L"txt",	L"a.b.txt"},
	};

	for(size_t i=0; i<sizeof(data)/sizeof(const wchar_t*)/2; ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).getExtension());
}

TEST(RemoveExtension_PathTest)
{
	const wchar_t* data[][2] = {
		{L"",		L""},
		{L"a",		L"a.txt"},
		{L"a.txt/",	L"a.txt/"},
		{L"a.b",	L"a.b.txt"},
	};

	for(size_t i=0; i<sizeof(data)/sizeof(const wchar_t*)/2; ++i) {
		Path p(data[i][1]);
		p.removeExtension();
		CHECK_EQUAL(data[i][0], p.getString());
	}
}

TEST(Normalize_PathTest)
{
	const wchar_t* data[][2] = {
		{L"",			L""},			// Should keep untouched
		{L"",			L"./"},
		{L"C:",			L"C:"},
		{L"C:/",		L"C:\\"},
		{L"C:/B",		L"C:\\B"},
		{L"file:///",	L"file:///"},
		{L"file:///a",	L"file:///a"},
		{L"/",			L"/"},
		{L".a",			L".a"},
		{L"a.txt",		L"a.txt"},
		{L"/home",		L"/home"},
		{L"/home",		L"/home/"},		// Remove trailing slash
		{L"/home",		L"/home/."},	// Collapse .
		{L"/home",		L"/home/./"},
		{L"/home/bar",	L"/home/./bar"},
		{L"a",			L"./a/"},
		{L"a/b.txt",	L"./a/b.txt"},
		{L"",			L"././"},
		{L"/",			L"/home/.."},	// Collapse ..
		{L"/",			L"/home/../"},
		{L"/bar",		L"/home/../bar"},
		{L"C:/",		L"C:/A/B/../.."},
		{L"C:/",		L"C:\\A\\B/../.."},
		{L"..",			L"../"},
		{L"../..",		L"../../"},
	};

	for(size_t i=0; i<sizeof(data)/sizeof(const wchar_t*)/2; ++i)
		CHECK_EQUAL(data[i][0], Path(data[i][1]).normalize().getString());
}

TEST(Append_PathTest)
{
	const wchar_t* data[][3] = {
		{L"",			L"",	L""},
		{L"",			L"./",	L""},
		{L"/a",			L"/",	L"a"},
		{L"a/b",		L"a",	L"b"},
		{L"a",			L"a/b",	L"../"},
		{L"",			L"a/b",	L"../../"},
	};

	for(size_t i=0; i<sizeof(data)/sizeof(const wchar_t*)/3; ++i)
		CHECK_EQUAL(data[i][0], (Path(data[i][1]) / data[i][2]).getString());

	// Appending a path with root to a non-empty path should throw an error
	CHECK_THROW(Path(L"a/")/=Path(L"/"), std::runtime_error);
}

TEST(CurrentPath_PathTest)
{
	// Get and then set the current path (should do nothing)
	Path current = Path::getCurrentPath();
	Path currentBackup = current;
	Path::setCurrentPath(current);
	CHECK(current == currentBackup);

	// Try to set an invalid path
	CHECK_THROW(Path::setCurrentPath(L"abc:\\"), std::runtime_error);
	// Try to set an non-existing path
	CHECK_THROW(Path::setCurrentPath(L"./_not_existing"), std::runtime_error);
}

TEST(Iterator_PathIteratorTest)
{
	const wchar_t* data[][2] = {
		{L"a/b/c",	L"a, a/b, a/b/c, "},	// Begin without slash
		{L"/a/b/c",	L"/a, /a/b, /a/b/c, "},	// Begin with slash
		{L"/a/b/c/",L"/a, /a/b, /a/b/c, "},	// End with slash
	};

	for(size_t i=0; i<sizeof(data)/sizeof(const wchar_t*)/2; ++i) {
		Path path(data[i][0]);
		PathIterator itr(path);

		std::wstring result;
		while(true) {
			std::wstring s = itr.next();
			if(s.empty())
				break;
			result += s;
			result += L", ";
		}

		CHECK_EQUAL(data[i][1], result);
	}
}
