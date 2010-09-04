#ifndef __MCD_CORE_SYSTEM_PATH__
#define __MCD_CORE_SYSTEM_PATH__

#include "../ShareLib.h"
#include "Platform.h"
#include <string>

namespace MCD {

/*!	A path class that is similar the one provided in boost.
	\note Comparison of Path in windows is case in-sensitive.
 */
class MCD_CORE_API Path
{
public:
	typedef char char_type;
	typedef std::basic_string<char_type> string_type;

	Path() {}

	MCD_IMPLICIT Path(sal_in_z sal_notnull const char_type* path) : mStr(path) {}

	MCD_IMPLICIT Path(const string_type& path) : mStr(path) {}

	//! Get the internal string representation of the path.
	const string_type& getString() const {
		return mStr;
	}

	/*! Get the root name.
		"C:"	->	"C:"	\n
		"C:\"	->	"C:"	\n
		"C:\B"	->	"C:"	\n
		"/"		->	""		\n
		"/home"	->	""		\n
		"./"	->	""		\n
		""		->	""		\n
	 */
	string_type getRootName() const;

	/*! Get the root directory, it will only be empty or with the value '/'.
		"C:"	->	"/"		\n
		"C:\"	->	"/"		\n
		"C:\B"	->	"/"		\n
		"/"		->	"/"		\n
		"/home"	->	"/"		\n
		"./"	->	""		\n
		""		->	""		\n
	 */
	string_type getRootDirectory() const;

	bool hasRootDirectory() const {
		return !getRootDirectory().empty();
	}

	/*!	Get the leaf path.
		""				-> ""		\n
		"/"				-> "/"		\n
		"a.txt"			-> "a.txt"	\n
		"/home/a.b.txt"	-> "a.b.txt"\n
	 */
	string_type getLeaf() const;

	/*!	Get the branch path.
		""				-> ""		\n
		"/"				-> ""		\n
		"a.txt"			-> ""		\n
		"/home/a.b.txt"	-> "/home"	\n
	 */
	Path getBranchPath() const;

	/*!	Get the file extension.
		""				-> ""		\n
		"a.txt"			-> "txt"	\n
		"a.txt/"		-> ""		\n
		"a.b.txt"		-> "txt"	\n
	 */
	string_type getExtension() const;

	/*!	Remove the file extension in the path.
		"/home/a.b.txt" -> "/home/a.b"
	 */
	void removeExtension();

	/*!	Normalize the path to a standard form.
		"./././"		-> ""		\n
		"C:\"			-> "C:/"	\n
		"/home/./"		-> "/home"	\n
		"/home/../bar"	-> "/bar"	\n
	 */
	Path& normalize();

	/*! Append.
		"/a/b" / "./c"	->	"/a/b/c"	\n
		"/a/b" / "c"	->	"/a/b/c"	\n
		"/a/b" / "./c"	->	"/a/b/c"	\n
		"/a/b" / "../c"	->	"/a/c"		\n
		"/a/b" / "/c"	->	Error: cannot append with root	\n
	 */
	Path& operator/=(const Path& rhs);

	Path operator/(const Path& rhs) const {
		return Path(*this) /= rhs;
	}

	/*!	Beware that comparison may not be accurate if the path are not normalized.
		Also, comparsion on Windows platforms are case in-sensitive.
	 */
	bool operator==(const Path& rhs) const {
		return compare(rhs) == 0;
	}

	bool operator!=(const Path& rhs) const {
		return compare(rhs) != 0;
	}

	bool operator<(const Path& rhs) const {
		return compare(rhs) < 0;
	}

	//!	A comparison function that gives int as the result just like what strCaseCmp() does.
	int compare(const Path& rhs) const;

	/*!	Get the current path of the running process.
		\note It will return an empty path if the operation fail.
	 */
	static Path getCurrentPath();

	//! Set the current path of the running process.
	sal_checkreturn static bool setCurrentPath(const Path& path);

protected:
	string_type mStr;
};	// Path

/*!	To iterator the Path.
	Example:
	\code
	Path path(L"a/b/c");
	PathIterator i(path);

	while(true) {
		std::wstring s = i.next();
		cout << s << ", ";
		if(s.empty())
			break;
	}
	// Result:
	// a, a/b, a/b/c
	\endcode
 */
class MCD_CORE_API PathIterator
{
public:
	explicit PathIterator(const Path& path);

	Path getPath() const;

	/*!	Advance the iterator, and return it's previous result.
		\param returnFullPath
			true: "a/b/c" -> a, a/b, a/b/c
			false: "a/b/c" -> a, b, c
	 */
	Path::string_type next(bool returnFullPath=true);

protected:
	Path::string_type mStr;
	size_t currentIndex;
};	// PathIterator

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_PATH__
