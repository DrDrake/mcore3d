#include "Pch.h"
#include "../../../MCD/Core/System/LinkList.h"
#include "../../../MCD/Core/System/Map.h"
#include "../../../MCD/Core/System/Macros.h"
#include "../../../MCD/Core/System/StrUtility.h"
#include "../../../MCD/Core/System/Utility.h"
#include <stdexcept>
#include <vector>

using namespace MCD;

namespace {

struct FooNode : public MapBase<int>::Node<FooNode>
{
	typedef MapBase<int>::Node<FooNode> Super;
	FooNode(int keyAndVal)
		: Super(keyAndVal), mVal(keyAndVal)
	{}
	int mVal;
};	// FooNode

}	// namespace

TEST(Basic_MapTest)
{
	static const size_t cTestCount = 100;
	Map<FooNode> map;
	std::vector<int> list;
	list.reserve(cTestCount);

	CHECK(map.isEmpty());
	CHECK_EQUAL(0u, map.elementCount());

	// Returns null if the key not found
	CHECK(map.find(0) == nullptr);
	CHECK(map.findMin() == nullptr);
	CHECK(map.findMax() == nullptr);
	CHECK(map.findSmaller(rand()) == nullptr);
	CHECK(map.findBigger(rand()) == nullptr);
	CHECK(map.findExactSmaller(rand()) == nullptr);
	CHECK(map.findExactBigger(rand()) == nullptr);

	{	// Insert a value and check again
		FooNode* node = new FooNode(1);
		map.insert(*node);
		CHECK(map.find(0) == nullptr);
		CHECK(map.find(1) == node);
		CHECK(map.findMin() == node);
		CHECK(map.findMax() == node);
		CHECK(map.findSmaller(1) == nullptr);
		CHECK(map.findBigger(1) == nullptr);
		CHECK(map.findSmaller(2) == node);
		CHECK(map.findBigger(0) == node);
		CHECK(map.findExactSmaller(1) == node);
		CHECK(map.findExactSmaller(2) == node);
		CHECK(map.findExactBigger(1) == node);
		CHECK(map.findExactBigger(0) == node);

		CHECK(!map.isEmpty());
		CHECK_EQUAL(1u, map.elementCount());
	}

	{	// Insert one more value and check again
		FooNode* node = new FooNode(10);
		map.insert(*node);
		CHECK(map.find(0) == nullptr);
		CHECK(map.find(10) == node);
		CHECK(map.findMin() != node);
		CHECK(map.findMax() == node);
		CHECK(map.findSmaller(10) != node);
		CHECK(map.findBigger(10) == nullptr);
		CHECK(map.findSmaller(11) == node);
		CHECK(map.findBigger(9) == node);
		CHECK(map.findExactSmaller(10) == node);
		CHECK(map.findExactSmaller(11) == node);
		CHECK(map.findExactBigger(10) == node);
		CHECK(map.findExactBigger(9) == node);

		CHECK(!map.isEmpty());
		CHECK_EQUAL(2u, map.elementCount());
	}

	{	// Change the key value
		FooNode* node = map.findMin();
		CHECK(node != map.findMax());
		node->setKey(100);
		CHECK_EQUAL(node, map.findMax());
	}

	{	// Remove the first node from the map
		FooNode* node = map.findMin();
		node->removeThis();
		delete node;
		CHECK(!map.isEmpty());
		CHECK_EQUAL(1u, map.elementCount());
	}

	{	// Destroy the last node from the map
		FooNode* node = map.findMin();
		node->destroyThis();
		CHECK(map.isEmpty());
		CHECK_EQUAL(0u, map.elementCount());
	}

	// Generate a random list of number and add to the map
	for(size_t i=0; i<cTestCount; ++i) {
		int val = ::rand();
		map.insert(*(new FooNode(val)));
		list.push_back(val);
	}

	map.assertValid();
	CHECK_EQUAL(cTestCount, map.elementCount());

	// Try to find all the values
	for(size_t i=0; i<cTestCount; ++i) {
		FooNode* node = map.find(list[i]);
		CHECK(node != nullptr);
	}

	{	// Check that the map is strictly sorted
		int prev;

		// Increasing order
		FooNode* node = map.findMin();
		if(node) {
			prev = node->mVal;
			for(; node; node = node->next()) {
				CHECK(node->mVal >= prev);
			}
		}

		// Decreasing order
		node = map.findMax();
		if(node) {
			prev = node->mVal;
			for(; node; node = node->prev()) {
				CHECK(node->mVal <= prev);
			}
		}
	}
}

TEST(RemoveAll_MapTest)
{
	static const size_t cCount = 10;
	Map<FooNode> map;
	std::vector<FooNode*> list;

	for(size_t i=0; i<cCount; ++i) {
		std::auto_ptr<FooNode> ptr(new FooNode(i));
		map.insert(*ptr);
		CHECK_EQUAL(&map, ptr->getMap());
		list.push_back(ptr.release());
	}

	map.removeAll();
	CHECK(map.isEmpty());

	for(size_t i=0; i<cCount; ++i) {
		delete list[i];
	}
}

namespace {

struct StringKeyNode : public MapBase<std::string>::Node<StringKeyNode>
{
	typedef MapBase<std::string>::Node<StringKeyNode> Super;
	StringKeyNode(const std::string& keyAndVal)
		: Super(keyAndVal), mVal(keyAndVal)
	{}
	std::string mVal;
};	// FooNode

}	// namespace

// Test for a more complex key type
TEST(StringKey_MapTest)
{
	static const size_t cTestCount = 100;
	Map<StringKeyNode> map;
	std::vector<std::string> list;
	list.reserve(cTestCount);

	// Generate a random list of string and add to the map
	for(size_t i=0; i<cTestCount; ++i) {
		int val = ::rand();
		StringKeyNode* node = new StringKeyNode(int2Str(val));
		map.insert(*node);
		list.push_back(node->mVal);
	}

	// Try to find all the values
	for(size_t i=0; i<cTestCount; ++i) {
		StringKeyNode* node = map.find(list[i]);
		CHECK(node != nullptr);
	}
}

namespace {

// Using the Map class to implement a bi-directional map
struct BiDirMapNode
{
	BiDirMapNode(int id, const std::string& str)
		: mId(id), mStr(str)
	{
	}

	struct Integer : public MapBase<int>::NodeBase {
		explicit Integer(int key) : NodeBase(key) {}
		MCD_DECLAR_GET_OUTER_OBJ(BiDirMapNode, mId);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	} mId;

	struct String : public MapBase<std::string>::NodeBase {
		explicit String(const std::string& key) : NodeBase(key) {}
		MCD_DECLAR_GET_OUTER_OBJ(BiDirMapNode, mStr);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	} mStr;
};	// BiDirMapNode

}	// namespace

TEST(BiDirectional_MapTest)
{
	static const size_t cTestCount = 100;
	Map<BiDirMapNode::Integer> idToStr;
	Map<BiDirMapNode::String> strToId;
	std::vector<int> list;
	list.reserve(cTestCount);

	// Generate a random list of integer/string and add to the map
	for(size_t i=0; i<cTestCount; ++i) {
		int val = ::rand();
		BiDirMapNode* node = new BiDirMapNode(val, int2Str(val));
		idToStr.insert(node->mId);
		strToId.insert(node->mStr);
		list.push_back(val);
	}

	// Try to find all the values
	for(size_t i=0; i<cTestCount; ++i) {
		BiDirMapNode* node = idToStr.find(list[i])->getOuterSafe();
		throwIfNull(node);
		const std::string& str = node->mStr.getKey();
		CHECK_EQUAL(int2Str(list[i]), str);

		// Should get back the same node using the string as the key
		CHECK_EQUAL(node, strToId.find(str)->getOuterSafe());
	}
}

namespace {

// Using Map together with LinkList to implement an unordered map
struct UnOrderedMapNode
{
	UnOrderedMapNode(int id)
		: mMap(id)
	{
	}

	// The map part which provide key mapping
	struct Map : public MapBase<int>::NodeBase {
		explicit Map(int key) : NodeBase(key) {}
		MCD_DECLAR_GET_OUTER_OBJ(UnOrderedMapNode, mMap);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	} mMap;

	// The list part which remember the insertion order
	struct List : public LinkListBase::Node<List> {
		MCD_DECLAR_GET_OUTER_OBJ(UnOrderedMapNode, mList);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	} mList;
};	// BiDirMapNode

}	// namespace

TEST(UnOrdered_MapTest)
{
	static const size_t cTestCount = 100;
	Map<UnOrderedMapNode::Map> map;
	LinkList<UnOrderedMapNode::List> list;
	std::vector<int> vec;
	vec.reserve(cTestCount);

	// Generate a random list of integer and add to the map
	for(size_t i=0; i<cTestCount; ++i) {
		int val = ::rand();
		std::auto_ptr<UnOrderedMapNode> node(new UnOrderedMapNode(val));
		bool inserted = map.insertUnique(node->mMap);

		if(inserted) {
			list.pushBack(node->mList);
			vec.push_back(val);
			node.release();
		}
	}

	// Iterate though the unordered map
	UnOrderedMapNode* node = list.front().getOuterSafe();
	for(size_t i=0; i<vec.size(); ++i) {
		throwIfNull(node);

		// Check the insertion order is the same
		CHECK_EQUAL(vec[i], node->mMap.getKey());

		// Check that it can be found in the map
		CHECK_EQUAL(node, map.find(vec[i])->getOuterSafe());

		node = node->mList.next()->getOuterSafe();
	}
}
