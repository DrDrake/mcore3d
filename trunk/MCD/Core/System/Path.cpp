#include "Pch.h"
#include "Path.h"
#include "ErrorCode.h"
#include "Log.h"
#include "PlatformInclude.h"
#include "StrUtility.h"
#include <string.h>	// For strcmp

namespace MCD {

namespace {

typedef Path::string_type string_type;
typedef Path::string_type::size_type size_type;

static bool IsSlash(Path::char_type c) {
	return c == '/' || c == '\\';
}

//! Find the offset of the first non-slash character in the path, starting from $initialOffset
static size_type FindNonSlash(const string_type& path, size_type initialOffset=0)
{
	for(size_type i=initialOffset, count=path.size(); i<count; ++i) {
		if(!IsSlash(path[i]))
			return i;
	}
	return string_type::npos;
}

//! Find the offset of the relative path
static size_type FindRelativePathPos(const string_type& path)
{
	if(path.empty())
		return string_type::npos;

	size_type pos = path.find(':');

	if(pos != string_type::npos)
		return FindNonSlash(path, pos+1);

	return path[0] == '/' ? 1 : 0;
}

}	// namespace

Path::string_type Path::getRootName() const
{
	size_type pos(mStr.find(':'));
	if(pos != string_type::npos)
		return mStr.substr(0, pos + 1);

	return string_type();
}

Path::string_type Path::getRootDirectory() const
{
	if(mStr.empty())
		return string_type();
	return (mStr[0] == '/' || !getRootName().empty()) ?	"/" : "";
}

// POSIX & Windows cases:	"", "/", "/foo", "foo", "foo/bar"
// Windows only cases:		"c:", "c:/", "c:foo", "c:/foo",
//							"prn:", "//share", "//share/", "//share/foo"
static size_t leafPos(const Path::string_type& str, size_t endPos) // endPos is past-the-end position
{
	// return 0 if str itself is leaf (or empty)
	if(endPos && IsSlash(str[endPos - 1]))
		return endPos - 1;

	size_t pos = Path::string_type::npos;
	for(size_t i=str.size(); i--;)
	{
		if(IsSlash(str[i])) {
			pos = i;
			break;
		}
	}

#ifdef _WIN32
	if(pos == Path::string_type::npos)
		pos = str.find_last_of(':', endPos - 2);
#endif	// _WIN32

	return (pos == Path::string_type::npos	// path itself must be a leaf (or empty)
#ifdef _WIN32
		|| (pos == 1 && IsSlash(str[0]))	// or share
#endif	// _WIN32
		) ? 0							// so leaf is entire string
		: pos + 1;						// or starts after delimiter
}

Path::string_type Path::getLeaf() const
{
	return mStr.substr(leafPos(mStr, mStr.size()));
}

Path Path::getBranchPath() const
{
	size_t endPos(leafPos(mStr, mStr.size()));

	// Skip a '/' unless it is a root directory
	if(endPos && IsSlash(mStr[endPos - 1]) /*&& !is_absolute_root(mStr, endPos)*/)
		--endPos;
	return Path(mStr.substr(0, endPos));
}

Path::string_type Path::getExtension() const
{
	// Scan from the end for the "." character
	// Return if '\' or '/' encountered
	size_type i = mStr.size();

	while(i--) {
		const char_type c = mStr[i];
		if(c == '.') {
			++i;	// Skip the dot character
			return mStr.substr(i, mStr.size() - i);
		}
		else if(c == '\\' || c == '/')
			break;
	}

	return "";
}

void Path::removeExtension()
{
	// Scan from the end for the "." character
	// Return if '\' or '/' encountered
	size_type i = mStr.size();

	while(i--) {
		const char_type c = mStr[i];
		if(c == '.') {
			mStr.resize(i);
			return;
		}
		else if(c == '\\' || c == '/')
			break;
	}
}

Path& Path::normalize()
{
	if(mStr.empty())
		return *this;

	// Unify all '\' into '/'
	for(size_type i=0, count=mStr.size(); i<count; ++i)
		if(mStr[i] == '\\')
			mStr[i] = '/';

	size_type end, beg(0), start = FindRelativePathPos(mStr);

	while( (beg = mStr.find("/..", beg)) != string_type::npos ) {
		end = beg + 3;
		if( (beg == 1 && mStr[0] == '.')
			|| (beg == 2 && mStr[0] == '.' && mStr[1] == '.')
			// NOTE: The following 2 lines of code cause problem in Rational Purify,
			// and the purpose of it is not totally understand yet.
/*			|| (beg > 2 && mStr[beg-3] == '/'
			&& mStr[beg-2] == '.' && mStr[beg-1] == '.')*/ )
		{
			beg = end;
			continue;
		}
		if(end < mStr.size()) {
			if(mStr[end] == '/')
				++end;
			else {	// name starts with ..
				beg = end;
				continue;
			}
		}

		// end is one past end of substr to be erased; now set beg
		while(beg > start && mStr[--beg] != '/') {}
		if(mStr[beg] == '/')
			++beg;
		mStr.erase(beg, end - beg);
		if(beg)
			--beg;
	}

	// Remove all "./" (note that remove from the back is more efficient)
	end = string_type::npos;
	while( (beg = mStr.rfind("./", end)) != string_type::npos ) {
		if(beg == 0 || mStr[beg-1] != '.')	// Not confuse with "../"
			mStr.erase(beg, 2);
		else
			end = beg - 1;
	}

	// Remove trailing '.'
	if(!mStr.empty() && mStr[mStr.size()-1] == '.') {
		if(mStr.size() == 1 || mStr[mStr.size()-2] != '.')	// If not ".."
			mStr.resize(mStr.size() - 1);
	}

	// Remove trailing '/' but not the root path "/"
	beg = mStr.rfind("/");
	if(beg != string_type::npos && beg > start && beg == mStr.size() -1)
		mStr.resize(mStr.size() - 1);

	return *this;
}

Path& Path::operator/=(const Path& rhs)
{
	normalize();

	if(!mStr.empty() && rhs.hasRootDirectory()) {
		Log::write(Log::Warn, "should not append a path with root directory unless lhs is empty");
		return *this;
	}

	if(!mStr.empty() && mStr[mStr.size()-1] != '/')
		mStr += "/";
	mStr += rhs.mStr;
	normalize();

	return *this;
}

int Path::compare(const Path& rhs) const
{
#ifdef _WIN32
	return strCaseCmp(mStr.c_str(), rhs.mStr.c_str());
#else
	return ::strcmp(mStr.c_str(), rhs.mStr.c_str());
#endif
}

Path Path::getCurrentPath()
{
#ifdef MCD_VC
	// We use GetCurrentDirectoryW since it directly support unicode

	DWORD sz;
	wchar_t dummy[1];	// Use a dummy to avoid a warning in Intel Thread Checker: passing nullptr to GetCurrentDirectoryW

	// Query the required buffer size
	if((sz = ::GetCurrentDirectoryW(0, dummy)) == 0)
		return Path();

	wchar_t* buf = (wchar_t*)MCD_STACKALLOCA(sz * sizeof(wchar_t));
	if(::GetCurrentDirectoryW(sz, buf) == 0)
		return Path();

	std::string utf8Str;
	MCD_VERIFY(wStrToUtf8(buf, utf8Str));
	Path tmp(utf8Str);
	MCD_STACKFREE(buf);
	return tmp.normalize();
#else
	// For other system we assume it have a UTF-8 locale

	char* buffer = getcwd( nullptr, 0);

	if(!buffer)
		return Path();

	Path ret(buffer);
	return ret.normalize();
#endif
}

bool Path::setCurrentPath(const Path& path)
{
#ifdef MCD_VC
	std::wstring wideString;
	MCD_VERIFY(utf8ToWStr(path.getString(), wideString));
	if(::SetCurrentDirectoryW(wideString.c_str()) == false) {
#else
	if(chdir(path.getString().c_str()) != 0) {
#endif
		return false;
	}
	return true;
}

// We will first normalize the incomming path and then append an extra "/"
// so that it work naturally with the next() function.
PathIterator::PathIterator(const Path& path)
	: mStr(Path(path).normalize().getString() + "/"), currentIndex(0)
{
	// Handle the case when there is a root directory
	if(mStr[0] == '/')
		++currentIndex;
}

Path::string_type PathIterator::next(bool returnFullPath)
{
	size_t oldIndex = currentIndex;
	currentIndex = mStr.find("/", currentIndex);
	if(currentIndex == Path::string_type::npos)
		return Path::string_type();
	if(returnFullPath)
		return mStr.substr(0, currentIndex++);
	else
		return mStr.substr(oldIndex, (currentIndex++) - oldIndex);
}

}	// namespace MCD
