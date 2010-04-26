#ifndef __MCD_CORE_SYSTEM_STRINGHASH__
#define __MCD_CORE_SYSTEM_STRINGHASH__

#include "../ShareLib.h"
#include "NonCopyable.h"
#include <vector>

namespace MCD {

/*!	A string class that ensure only one memory block is allocated for each unique string,
	much like immutable string class in some language like C# and Python.
	This class is intended for fast string comparison, as a look up key etc.

	This class is thread safe.
 */
class MCD_CORE_API FixString
{
public:
	explicit FixString(const char* str);

	~FixString();

	sal_maybenull const char* c_str() const;

	sal_maybenull operator const char*() const {
		return c_str();
	}

	bool operator==(const FixString& rhs) const	{	return c_str() == rhs.c_str();	}
	bool operator> (const FixString& rhs) const	{	return c_str() > rhs.c_str();	}
	bool operator< (const FixString& rhs) const	{	return c_str() < rhs.c_str();	}

	struct Node;

protected:
	const Node* const mNode;
};	// FixString

/*!	A special designed hash table for storing a set of strings.
	The implmentation use a linked list on each bucket.
 */
class MCD_CORE_API StringHashSet : Noncopyable
{
public:
	//!	A default bucket size of 128 is used.
	StringHashSet();

	~StringHashSet();

	sal_maybenull const char* find(uint32_t hashValue) const;

	/*!	Add a string into the hash table.
		The input string will be copy into a memory region manged by the table.
		Returns existing string pointer if it's already present in the table.
		Returns null if it detected that 2 strings are mapped into the same hash.
	 */
	sal_maybenull const char* add(uint32_t hashValue, sal_in_z const char* stringValue);

	void remove(uint32_t hashValue);

	void clear();

	//!	A larger bucket size trade memory space for search efficiency.
	void resizeBucket(size_t bucketSize);

protected:
	//!	This struct and the actual stirng will share the same block of memory
	struct Node
	{
		uint32_t hashValue;
		Node* next;
		const char* stringValue() const {
			return reinterpret_cast<const char*>(this + 1);
		}
	};	// Node

	std::vector<Node*> mBuckets;
};	// StringHashSet

/*!	A simple string hash class.
	Reference http://www.humus.name/index.php?page=Comments&ID=296
 */
class MCD_CORE_API StringHash
{
protected:
	StringHash() {}	// Accessed by derived-classes only

	typedef uint32_t HashType;

	// A small utility for defining the "const char (&Type)[N]" argument
	template<int N>
	struct Str { typedef const char (&Type)[N]; };

	template<int N> inline HashType sdbm(typename Str<N>::Type str)
	{
		// hash(i) = hash(i - 1) * 65599 + str[i]
		// Reference: http://www.cse.yorku.ca/~oz/hash.html sdbm
		return sdbm<N-1>((typename Str<N-1>::Type)str) * 65599 + str[N-2];
	}

	// "1" = char[2]
	template<> inline HashType sdbm<2>(typename Str<2>::Type str) {
		return str[0];
	}

public:
	uint32_t hash;

	/*!	The overloaded constructors allow hashing a string literal without run-time cost (in release mode)
		"1" is equals to char[2] since the '\0' is included.
	 */
	MCD_INLINE2 StringHash::StringHash(const char (&str)[2]) {	hash = sdbm<2>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[3]) {	hash = sdbm<3>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[4]) {	hash = sdbm<4>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[5]) {	hash = sdbm<5>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[6]) {	hash = sdbm<6>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[7]) {	hash = sdbm<7>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[8]) {	hash = sdbm<8>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[9]) {	hash = sdbm<9>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[10]) {	hash = sdbm<10>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[11]) {	hash = sdbm<11>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[12]) {	hash = sdbm<12>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[13]) {	hash = sdbm<13>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[14]) {	hash = sdbm<14>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[15]) {	hash = sdbm<15>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[16]) {	hash = sdbm<16>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[17]) {	hash = sdbm<17>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[18]) {	hash = sdbm<18>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[19]) {	hash = sdbm<19>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[20]) {	hash = sdbm<20>(str);	}
	MCD_INLINE2 StringHash::StringHash(const char (&str)[21]) {	hash = sdbm<21>(str);	}

	//!	If len is 0 then the length is auto detected.
	StringHash(sal_in_z const char* buf, size_t len);
	StringHash(sal_in_z const wchar_t* buf, size_t len);

	operator uint32_t() const {	return hash;	}

	bool operator==(const StringHash& rhs) const	{	return hash == rhs.hash;	}
	bool operator> (const StringHash& rhs) const	{	return hash > rhs.hash;		}
	bool operator< (const StringHash& rhs) const	{	return hash < rhs.hash;		}
};	// StringHash

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_STRINGHASH__
