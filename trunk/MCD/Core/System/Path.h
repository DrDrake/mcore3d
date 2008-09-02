#ifndef __MCD_CORE_SYSTEM_PATH__
#define __MCD_CORE_SYSTEM_PATH__

#include "../ShareLib.h"
#include "Platform.h"
#include <string>

namespace MCD {

//! A path class that is similar the one provided in boost.
class MCD_CORE_API Path
{
public:
	typedef wchar_t char_type;
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

	//! Beware that comparison may not be accurate if the path are not normalized.
	bool operator==(const Path& rhs) const {
		return mStr == rhs.mStr;
	}

	bool operator!=(const Path& rhs) const {
		return mStr != rhs.mStr;
	}

	bool operator<(const Path& rhs) const {
		return mStr < rhs.mStr;
	}

	/*!	Get the current path of the running process.
		\note It will return an empty path if the operation fail.
	 */
	static Path getCurrentPath();

	//! Set the current path of the running process.
	static void setCurrentPath(const Path& path);

private:
	string_type mStr;
};	// Path

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_PATH__
