#include "Pch.h"
#include "Path.h"
#include "ErrorCode.h"
#include "PlatformInclude.h"
#include "StrUtility.h"
#include <stdexcept>

namespace MCD {

namespace {

typedef Path::string_type string_type;
typedef Path::string_type::size_type size_type;

#define L(x)	L ## x

static bool IsSlash(Path::char_type c) {
	return c == L('/') || c == L('\\');
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

	size_type pos = path.find(L(':'));

	if(pos != string_type::npos)
		return FindNonSlash(path, pos+1);

	return path[0] == L('/') ? 1 : 0;
}

}	// namespace

Path::string_type Path::getRootName() const
{
	size_type pos(mStr.find(L(':')));
	if(pos != string_type::npos)
		return mStr.substr(0, pos + 1);

	return string_type();
}

Path::string_type Path::getRootDirectory() const
{
	if(mStr.empty())
		return string_type();
	return (mStr[0] == L('/') || !getRootName().empty()) ?	L("/") : L("");
}

static const wchar_t cSlash = L('/');

// POSIX & Windows cases:	"", "/", "/foo", "foo", "foo/bar"
// Windows only cases:		"c:", "c:/", "c:foo", "c:/foo",
//							"prn:", "//share", "//share/", "//share/foo"
static size_t leafPos(const Path::string_type& str, size_t endPos) // endPos is past-the-end position
{
	// return 0 if str itself is leaf (or empty)
	if(endPos && str[endPos - 1] == cSlash)
		return endPos - 1;

	size_t pos(str.find_last_of(cSlash, endPos - 1));
#ifdef _WIN32
	if(pos == Path::string_type::npos)
		pos = str.find_last_of(':', endPos - 2);
#endif	// _WIN32

	return (pos == Path::string_type::npos	// path itself must be a leaf (or empty)
#ifdef _WIN32
		|| (pos == 1 && str[0] == cSlash)	// or share
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
	if(endPos && mStr[endPos - 1] == cSlash /*&& !is_absolute_root(mStr, endPos)*/)
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
		if(c == L'.') {
			++i;	// Skip the dot character
			return mStr.substr(i, mStr.size() - i);
		}
		else if(c == L'\\' || c == L'/')
			break;
	}

	return L"";
}

void Path::removeExtension()
{
	// Scan from the end for the "." character
	// Return if '\' or '/' encountered
	size_type i = mStr.size();

	while(i--) {
		const char_type c = mStr[i];
		if(c == L'.') {
			mStr.resize(i);
			return;
		}
		else if(c == L'\\' || c == L'/')
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
			mStr[i] = L('/');

	size_type end, beg(0), start = FindRelativePathPos(mStr);

	while( (beg = mStr.find(L("/.."), beg)) != string_type::npos ) {
		end = beg + 3;
		if( (beg == 1 && mStr[0] == L('.'))
			|| (beg == 2 && mStr[0] == L('.') && mStr[1] == L('.'))
			// NOTE: The following 2 lines of code cause problem in Rational Purify,
			// and the purpose of it is not totally understand yet.
/*			|| (beg > 2 && mStr[beg-3] == L('/')
			&& mStr[beg-2] == L('.') && mStr[beg-1] == L('.'))*/ )
		{
			beg = end;
			continue;
		}
		if(end < mStr.size()) {
			if(mStr[end] == L('/'))
				++end;
			else {	// name starts with ..
				beg = end;
				continue;
			}
		}

		// end is one past end of substr to be erased; now set beg
		while(beg > start && mStr[--beg] != L('/')) {}
		if(mStr[beg] == L('/'))
			++beg;
		mStr.erase(beg, end - beg);
		if(beg)
			--beg;
	}

	// Remove all "./" (note that remove from the back is more efficient)
	end = string_type::npos;
	while( (beg = mStr.rfind(L("./"), end)) != string_type::npos ) {
		if(beg == 0 || mStr[beg-1] != L('.'))	// Not confuse with "../"
			mStr.erase(beg, 2);
		else
			end = beg - 1;
	}

	// Remove trailing '.'
	if(!mStr.empty() && mStr[mStr.size()-1] == L('.'))
		mStr.resize(mStr.size() - 1);

	// Remove trailing '/' but not the root path "/"
	beg = mStr.rfind(L("/"));
	if(beg != string_type::npos && beg > start && beg == mStr.size() -1)
		mStr.resize(mStr.size() - 1);

	return *this;
}

Path& Path::operator/=(const Path& rhs)
{
	normalize();

	if(!mStr.empty() && rhs.hasRootDirectory())
		throw std::runtime_error("should not append a path with root directory unless lhs is empty");

	if(!mStr.empty() && mStr[mStr.size()-1] != L('/'))
		mStr += L("/");
	mStr += rhs.mStr;
	normalize();

	return *this;
}

Path Path::getCurrentPath()
{
#ifdef MCD_VC
	// We use GetCurrentDirectoryW since it directly support unicode

	DWORD sz;
	char_type dummy[1];	// Use a dummy to avoid a warning in Intel Thread Checker: passing nullptr to GetCurrentDirectoryW

	// Query the required buffer size
	if((sz = ::GetCurrentDirectoryW(0, dummy)) == 0)
		return Path();

	char_type* buf = (char_type*)MCD_STACKALLOCA(sz * sizeof(char_type));
	if(::GetCurrentDirectoryW(sz, buf) == 0)
		return Path();

	Path tmp(buf);
	MCD_STACKFREE(buf);
	return tmp.normalize();
#else
	// For other system we assume it have a UTF-8 locale so wStrToStr will work as expected

	char* buffer = getcwd( nullptr, 0);

	if(!buffer)
		return Path();

	Path ret;
	if(strToWStr(buffer, ret.mStr))
		return ret.normalize();
	else
		return Path();
#endif
}

void Path::setCurrentPath(const Path& path)
{
#ifdef MCD_VC
	if(::SetCurrentDirectoryW(path.getString().c_str()) == false) {
		std::string narrowStr;
		MCD_VERIFY(wStrToStr(path.getString().c_str(), narrowStr));
#else
	std::string narrowStr;
	MCD_VERIFY(wStrToStr(path.getString().c_str(), narrowStr));
	if(chdir(narrowStr.c_str()) != 0) {
#endif
		throw std::runtime_error(
			MCD::getErrorMessage(("Unable to set current path to '" + narrowStr + "': ").c_str(), MCD::getLastError())
		);
	}
}

// We will first normalize the incomming path and then append an extra "/"
// so that it work naturally with the next() function.
PathIterator::PathIterator(const Path& path)
	: mStr(Path(path).normalize().getString() + L"/"), currentIndex(0)
{
	// Handle the case when there is a root directory
	if(mStr[0] == L'/')
		++currentIndex;
}

Path::string_type PathIterator::next()
{
	currentIndex = mStr.find(L"/", currentIndex);
	if(currentIndex == Path::string_type::npos)
		return Path::string_type();
	return mStr.substr(0, currentIndex++);
}

}	// namespace MCD
