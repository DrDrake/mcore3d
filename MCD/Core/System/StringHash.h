#ifndef __MCD_CORE_SYSTEM_STRINGHASH__
#define __MCD_CORE_SYSTEM_STRINGHASH__

#include "../ShareLib.h"
#include "NonCopyable.h"
#include <vector>

namespace MCD {

class StringHash;

/*!	A string class that ensure only one memory block is allocated for each unique string,
	much like immutable string class in some language like C# and Python.
	This class is intended for fast string comparison, as a look up key etc.

	Behind the scene, there is a single global hash table managing all instance of FixString.
	Whenever a FixString is created, it search for any existing string in the table that match
	with the input string; return that cached string if yes, otherwise the input string will
	be hashed and copyed into the global hash table.

	Every FixString instances are reference counted by the global hash table, once it's reference
	count become zero the FixString's corresponding entry in the hash table along with the string
	data will be deleted.

	This class is thread safe regarding the read/write to the global hash table.
 */
class MCD_CORE_API FixString
{
public:
	FixString();

	/*!	The input string will copyed and reference counted.
		Null input string will result an empty string.
	 */
	MCD_IMPLICIT FixString(sal_in_z_opt const char* str);

	/*!	Construct with an existing string in the global table, indexed with it's hash value.
		Make sure there is an other instance of FixString constructed with a string content first.
		An empty string is resulted if the hash value cannot be found.
	 */
	explicit FixString(uint32_t hashValue);

	FixString(const FixString& rhs);
	~FixString();

	FixString& operator=(const char* rhs);
	FixString& operator=(const FixString& rhs);
	FixString& operator=(const StringHash& stringHash);

	sal_notnull const char* c_str() const;
	sal_notnull operator const char*() const {	return c_str();	}

	uint32_t hashValue() const;

	size_t size() const;

	bool empty() const;

	bool operator==(const StringHash& stringHash) const;
	bool operator==(const FixString& rhs) const;
	bool operator> (const FixString& rhs) const;
	bool operator< (const FixString& rhs) const;

	struct Node;

protected:
	sal_notnull Node* mNode;
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

namespace StringHashDetail
{
	// A small utility for defining the "const char (&Type)[N]" argument
	template<int N>
	struct Str { typedef const char (&Type)[N]; };

	template<int N> MCD_INLINE2 uint32_t sdbm(typename Str<N>::Type str)
	{
		// hash(i) = hash(i - 1) * 65599 + str[i]
		// Reference: http://www.cse.yorku.ca/~oz/hash.html sdbm
		return sdbm<N-1>((typename Str<N-1>::Type)str) * 65599 + str[N-2];
	}

	// "1" = char[2]
	// NOTE: Template specialization have to be preformed in namespace scope.
	template<> MCD_INLINE2 uint32_t sdbm<2>(Str<2>::Type str) {
		return str[0];
	}
}	// StringHashDetail

/*!	A simple string hash class.
	Reference http://www.humus.name/index.php?page=Comments&ID=296
 */
class MCD_CORE_API StringHash
{
protected:
	StringHash() {}	// Accessed by derived-classes only

public:
	uint32_t hash;

	StringHash(uint32_t h) : hash(h) {}

	StringHash(const FixString& fixString) : hash(fixString.hashValue()) {}

	/*!	The overloaded constructors allow hashing a string literal without run-time cost (in release mode)
		"1" is equals to char[2] since the '\0' is included.
	 */
	MCD_INLINE2 StringHash(const char (&str)[2]) {	hash = StringHashDetail::sdbm<2>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[3]) {	hash = StringHashDetail::sdbm<3>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[4]) {	hash = StringHashDetail::sdbm<4>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[5]) {	hash = StringHashDetail::sdbm<5>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[6]) {	hash = StringHashDetail::sdbm<6>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[7]) {	hash = StringHashDetail::sdbm<7>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[8]) {	hash = StringHashDetail::sdbm<8>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[9]) {	hash = StringHashDetail::sdbm<9>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[10]) {	hash = StringHashDetail::sdbm<10>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[11]) {	hash = StringHashDetail::sdbm<11>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[12]) {	hash = StringHashDetail::sdbm<12>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[13]) {	hash = StringHashDetail::sdbm<13>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[14]) {	hash = StringHashDetail::sdbm<14>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[15]) {	hash = StringHashDetail::sdbm<15>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[16]) {	hash = StringHashDetail::sdbm<16>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[17]) {	hash = StringHashDetail::sdbm<17>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[18]) {	hash = StringHashDetail::sdbm<18>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[19]) {	hash = StringHashDetail::sdbm<19>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[20]) {	hash = StringHashDetail::sdbm<20>(str);	}
	MCD_INLINE2 StringHash(const char (&str)[21]) {	hash = StringHashDetail::sdbm<21>(str);	}

	//!	Parameter \em len define the maximum length of the input string buf, 0 for auto length detection.
	StringHash(sal_in_z const char* buf, size_t len);
	StringHash(sal_in_z const wchar_t* buf, size_t len);

	operator uint32_t() const {	return hash;	}

	bool operator==(const StringHash& rhs) const	{	return hash == rhs.hash;	}
	bool operator> (const StringHash& rhs) const	{	return hash > rhs.hash;		}
	bool operator< (const StringHash& rhs) const	{	return hash < rhs.hash;		}
};	// StringHash

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_STRINGHASH__
