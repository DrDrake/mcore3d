#include "Pch.h"
#include "StringHash.h"
#include "Atomic.h"
#include "Mutex.h"

namespace MCD {

StringHashSet::StringHashSet() {
	resizeBucket(128);
}

StringHashSet::~StringHashSet() {
	clear();
}

const char* StringHashSet::find(uint32_t hashValue) const
{
	const size_t index = hashValue % mBuckets.size();
	for(const Node* n = mBuckets[index]; n; n = n->next) {
		if(n->hashValue != hashValue)
			continue;
		return n->stringValue();
	}
	return nullptr;
}

const char* StringHashSet::add(uint32_t hashValue, sal_in_z const char* stringValue)
{
	if(!stringValue)
		return nullptr;

	const size_t index = hashValue % mBuckets.size();

	// Find any string with the same hash value
	for(Node* n = mBuckets[index]; n; n = n->next)
		if(n->hashValue == hashValue)
			return strcmp(n->stringValue(), stringValue) == 0 ? n->stringValue() : nullptr;

	const size_t length = strlen(stringValue) + 1;
	if(Node* n = (Node*)malloc(sizeof(Node) + length)) {
		memcpy((void*)n->stringValue(), stringValue, length);
		n->hashValue = hashValue;

		n->next = mBuckets[index];
		mBuckets[index] = n;
		return n->stringValue();
	}

	return nullptr;
}

void StringHashSet::remove(uint32_t hashValue)
{
	const size_t index = hashValue % mBuckets.size();
	Node* last = nullptr;
	for(Node* n = mBuckets[index]; n; last = n, n = n->next) {
		if(n->hashValue == hashValue) {
			if(last)
				last->next = n->next;
			else
				mBuckets[index] = n->next;
			free(n);
			return;
		}
	}
}

void StringHashSet::clear()
{
	for(size_t i=0; i<mBuckets.size(); ++i) {
		for(Node* n = mBuckets[i]; n; ) {
			Node* next = n->next;
			free(n);
			n = next;
		}
		mBuckets[i] = nullptr;
	}
}

void StringHashSet::resizeBucket(size_t bucketSize)
{
	std::vector<Node*> newBuckets(bucketSize, nullptr);

	for(size_t i=0; i<mBuckets.size(); ++i) {
		for(Node* n = mBuckets[i]; n; ) {
			Node* next = n->next;
			const size_t index = n->hashValue % bucketSize;
			n->next = newBuckets[index];
			newBuckets[index] = n;
			n = next;
		}
	}

	std::swap(newBuckets, mBuckets);
}

StringHash::StringHash(const char* buf, size_t len)
{
	hash = 0;
	len = len == 0 ? size_t(-1) : len;
    
    for(size_t i=0; i<len && buf[i] != '\0'; ++i) {
		//hash = hash * 65599 + buf[i];
		hash = buf[i] + (hash << 6) + (hash << 16) - hash;
	}
}

StringHash::StringHash(const wchar_t* buf, size_t len)
{
	hash = 0;
	len = len == 0 ? size_t(-1) : len;

    for(size_t i=0; i<len && buf[i] != L'\0'; ++i) {
		//hash = hash * 65599 + buf[i];
		hash = buf[i] + (hash << 6) + (hash << 16) - hash;
	}
}

struct FixString::Node
{
	uint32_t hashValue;
	Node* next;
	AtomicInteger refCount;
	const char* stringValue() const {
		return reinterpret_cast<const char*>(this + 1);
	}
};	// Node

namespace {

class FixStringHashTable
{
public:
	typedef FixString::Node Node;

	FixStringHashTable() : mCount(0) {
		ScopeLock lock(mMutex);
		resizeBucket(1);
	}

	~FixStringHashTable()
	{
		ScopeLock lock(mMutex);
		MCD_ASSERT(mCount == 0 && "All instance of FixString should be destroyed before FixStringHashTable");
	}

	Node* find(uint32_t hashValue) const
	{
		const size_t index = hashValue % mBuckets.size();
		for(Node* n = mBuckets[index]; n; n = n->next) {
			if(n->hashValue != hashValue)
				continue;
			return n;
		}
		return nullptr;
	}

	Node* add(sal_in_z const char* str)
	{
		if(!str)
			return nullptr;

		const uint32_t hashValue = StringHash(str, 0).hash;
		const size_t index = hashValue % mBuckets.size();

		ScopeLock lock(mMutex);

		// Find any string with the same hash value
		for(Node* n = mBuckets[index]; n; n = n->next) {
			if(n->hashValue != hashValue)
				continue;
			if(strcmp(n->stringValue(), str) == 0) {
				++n->refCount;
				return n;
			}
			else {
				MCD_ASSERT(false && "String hash collision in FixString");
				return nullptr;
			}
		}

		const size_t length = strlen(str) + 1;
		if(Node* n = (Node*)malloc(sizeof(Node) + length)) {
			memcpy((void*)n->stringValue(), str, length);
			n->hashValue = hashValue;
			n->refCount = 1;

			n->next = mBuckets[index];
			mBuckets[index] = n;
			++mCount;

			// Enlarge the bucket if necessary
			if(mCount * 2 > mBuckets.size() * 3)
				resizeBucket(mBuckets.size() * 2);

			return n;
		}
		return nullptr;
	}

	void remove(Node& node)
	{
		if(--node.refCount != 0)
			return;

		const size_t index = node.hashValue % mBuckets.size();
		Node* last = nullptr;

		ScopeLock lock(mMutex);
		for(Node* n = mBuckets[index]; n; last = n, n = n->next) {
			if(n->hashValue == node.hashValue) {
				if(last)
					last->next = n->next;
				else
					mBuckets[index] = n->next;
				free(n);
				--mCount;
				return;
			}
		}
		MCD_ASSERT(false);
	}

	void resizeBucket(size_t bucketSize)
	{
		std::vector<Node*> newBuckets(bucketSize, nullptr);

		MCD_ASSERT(mMutex.isLocked());
		for(size_t i=0; i<mBuckets.size(); ++i) {
			for(Node* n = mBuckets[i]; n; ) {
				Node* next = n->next;
				const size_t index = n->hashValue % bucketSize;
				n->next = newBuckets[index];
				newBuckets[index] = n;
				n = next;
			}
		}

		std::swap(newBuckets, mBuckets);
	}

	Mutex mMutex;
	size_t mCount;	//!< The actuall number of elements in this table, can be <=> mBuckets.size()
	std::vector<Node*> mBuckets;
};	// FixStringHashTable

static FixStringHashTable gFixStringHashTable;

}	// namespace

FixString::FixString()
	: mNode(nullptr)
{}

FixString::FixString(const char* str)
	: mNode(gFixStringHashTable.add(str))
{}

FixString::FixString(uint32_t hashValue)
	: mNode(gFixStringHashTable.find(hashValue))
{
	if(mNode) ++mNode->refCount;
}

FixString::FixString(const FixString& rhs)
	: mNode(rhs.mNode)
{
	if(mNode) ++mNode->refCount;
}

FixString::~FixString() {
	if(mNode) gFixStringHashTable.remove(*mNode);
}

FixString& FixString::operator=(const FixString& rhs)
{
	if(mNode) gFixStringHashTable.remove(*mNode);
	mNode = rhs.mNode;
	if(mNode) ++mNode->refCount;
	return *this;
}

FixString& FixString::operator=(const StringHash& stringHash)
{
	FixString tmp(stringHash);
	return *this = tmp;
}

const char* FixString::c_str() const {
	return mNode ? mNode->stringValue() : nullptr;
}

uint32_t FixString::hashValue() const {
	return mNode ? mNode->hashValue : 0;
}

bool FixString::operator==(const StringHash& h) const	{	return mNode ? mNode->hashValue == h : false;	}
bool FixString::operator==(const FixString& rhs) const	{	return hashValue() == rhs.hashValue();	}
bool FixString::operator> (const FixString& rhs) const	{	return hashValue() > rhs.hashValue();	}
bool FixString::operator< (const FixString& rhs) const	{	return hashValue() < rhs.hashValue();	}

bool FixString::empty() const {
	return mNode ? *mNode->stringValue() == '\0' : false;
}

}	// namespace MCD
