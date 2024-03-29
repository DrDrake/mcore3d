#ifndef __MCD_CORE_SYSTEM_MAP__
#define __MCD_CORE_SYSTEM_MAP__

#include "../ShareLib.h"
#include "NonCopyable.h"
#include "TypeTrait.h"	// For ParamType
#include <string>

namespace MCD {

/*!	\class MapBase
	The base class for an intrusive associative map container.
	The elements in the map are sorted and the sorting criteria is base on the Map::Comparator object,
	see MapComparator for more details.

	To use the map, user have to extend the node first:
	\code
	struct FooNode : public MapBase<int>::Node<FooNode> {
		typedef MapBase<int>::Node<FooNode> Super;
		FooNode(int key, const std::string& val)
			: Super(key), mVal(val)
		{}
		std::string mVal;
	};	// FooNode

	// ...

	Map<FooNode> map;
	map.insert(*(new FooNode(1, "1")));
	\endcode

	To iterate tough the elements of the map:
	\code
	// Iterator from small to large
	for(FooNode* n = &map.findMin(); n != nullptr; n = n->next()) {}
	// Iterator from large to small
	for(FooNode* n = &map.findMax(); n != nullptr; n = n->prev()) {}
	\endcode

	To find an element with a provided key:
	\code
	FooNode* n = map.find(1);
	if(n != nullptr)
		std::cout << n->mVal << std::endl;
	else
		std::cout << "Key not found!" << std::endl;
	\endcode

	To add an element into multiple map (or similar intrusive container such as LinkList):
	\code
	// Node for a bi-directional map
	struct BiDirMapNode {
		BiDirMapNode(int id, const std::string& str)
			: mId(id), mStr(str)
		{}

		struct Integer : public MapBase<int>::NodeBase {
			explicit Integer(int key) : NodeBase(key) {}
			MCD_DECLAR_GET_OUTER_OBJ(BiDirMapNode, mId);
			sal_override void destroyThis() {
				delete getOuterSafe();
			}
		} mId;

		struct String : public MapBase<std::string>::NodeBase {
			explicit String(const std::string& key) : NodeBase(key) {}
			MCD_DECLAR_GET_OUTER_OBJ(BiDirMapNode, mStr);
			sal_override void destroyThis() {
				delete getOuterSafe();
			}
		} mStr;
	};	// BiDirMapNode

	// ...

	Map<BiDirMapNode::Integer> idToStr;
	Map<BiDirMapNode::String> strToId;

	BiDirMapNode* node = new BiDirMapNode(123, "123");
	idToStr.insert(node->mId);
	strToId.insert(node->mStr);

	BiDirMapNode* p1 = idToStr.find(123).getOuterSafe();
	BiDirMapNode* p2 = strToId.find("123").getOuterSafe();
	MCD_ASSERT(p1 == p2);
	\endcode
 */

namespace Impl {

/*!	The core Avl tree implementation for the Map class.
	It is a non template class thus save code size for the Map class.
	\sa http://www.codeproject.com/KB/tips/Containers.aspx
 */
class MCD_CORE_API AvlTree : Noncopyable
{
protected:
	class Node
	{
		friend class AvlTree;

	protected:
		enum Direction { Left = 0, Right = 1 };

		MCD_CORE_API Node();

		MCD_CORE_API virtual ~Node();

	public:
		MCD_CORE_API virtual void destroyThis();

		MCD_CORE_API void removeThis();

	protected:
		bool isInMap() const {
			return mAvlTree != nullptr;
		}

		sal_maybenull AvlTree* getMap() {
			return mAvlTree;
		}

		sal_maybenull Node* next() {
			if(mAvlTree)
				return mAvlTree->walkR(*this);
			return nullptr;
		}

		sal_maybenull Node* prev() {
			if(mAvlTree)
				return mAvlTree->walkL(*this);
			return nullptr;
		}

		bool isBallanceOk() const;

		//! Get weather this node is left or right child of it's parent.
		Direction parentIdx() const;

		//! Set \em child as the child of this node. Note that \em child can be null.
		void setChildSafe(Direction dir, sal_maybenull Node* child);

#ifndef NDEBUG
		MCD_CORE_API size_t assertValid(size_t& total, sal_maybenull const Node* parent, size_t nL, size_t nR) const;
		Node* mRight;	//! For visual studio debug visualization
#endif	// NDEBUG

		void synLeftRight();	//! For visual studio debug visualization

		Node* mChildren[2];	// Left/Right children
		Node* mParent;
		AvlTree* mAvlTree;
		int mBallance;	// Negative=Left, Positive=Right
	};	// Node

	enum Direction { Left = Node::Left, Right = Node::Right };

	AvlTree();

	void insert(Node& node, sal_maybenull Node* parent, int nIdx);

	void remove(Node& node, sal_maybenull Node* onlyChild);

	void remove(Node& node);

	void replaceFixTop(Node& node, Node& next);

	bool rotate(Node& node, int dir);

	void adjustBallance(Node& node, int dir, bool removed);

	//! Get the extreme leaf node.
	template<Direction dir>	static Node* getExtreme(Node& pos);
	static Node* getExtremeL(Node& pos);
	static Node* getExtremeR(Node& pos);

	Node* getExtrRootL() {
		return mRoot ? getExtremeL(*mRoot) : nullptr;
	}
	Node* getExtrRootR() {
		return mRoot ? getExtremeR(*mRoot) : nullptr;
	}

	//! Perform in-order traversal.
	template<Direction dir>
	Node* walk(Node& node);

	//! Reversed in-order traversal (small to large).
	Node* walkL(Node& node);

	//! Forward in-order traversal (large to small).
	Node* walkR(Node& node);

public:
	~AvlTree();

	//!	Remove all elements
	void removeAll();

	//! Destroy all elements
	void destroyAll();

	size_t elementCount() const {
		return mCount;
	}

	bool isEmpty() const;

protected:
	Node* mRoot;
	size_t mCount;
};	// AvlTree

}	// namespace Impl

/*!	The basic comparator used in MCD::Map.
	Requires \em TKeyArg to have the < and == operators defined.
	User can make explicit template specialization on different types of \em TKeyArg,
	see MapCharStrComparator for a specialization for string as key type.

	\sa MCD::Map
 */
template<typename TKeyArg>
struct MapComparator {
	typedef TKeyArg KeyArg;

	explicit MapComparator(KeyArg key)
		: mKey(key)
	{
	}

	/*!	Returns:
		-1	if \em key <  \em mKey
		 0	if \em key == \em mKey
		 1	if \em key >  \em mKey
	 */
	int compare(KeyArg key) {
		return (key < mKey) ? (-1) : (key == mKey) ? 0 : 1;
	}

	// Using \em KeyArg as the type is enough, since the scope of \em MapComparator is temporary.
	KeyArg mKey;
};	// MapComparator

/*!	Comparator for null terminated char string.
	This comparator can remember some useful information during the past comparison(s),
	therefore the 2 string with the same prefix can be skipped.
 */
template<typename TCharType>
struct MapCharStrComparator
{
	typedef TCharType CharType;

	explicit MapCharStrComparator(sal_in_z sal_notnull const CharType* str)
		: mStr(str), mLMatch(0), mRMatch(0)
	{
	}

	int compare(sal_in_z sal_notnull const CharType* rhs)
	{
		struct Dummy {
			static MCD_INLINE2 size_t min(size_t lhs_, size_t rhs_) {
				return lhs_ < rhs_ ? lhs_ : rhs_;
			}
		};	// Dummy

		for(size_t pos = Dummy::min(mLMatch, mRMatch); ;++pos) {
			CharType chThis = mStr[pos];
			CharType chArg = rhs[pos];

			if(chArg < chThis) {	// Already accounted for the case of terminated rhs
				mLMatch = pos;
				return -1;
			}
			if(chArg > chThis) {
				mRMatch = pos;
				return 1;
			}

			if(!chThis)	// Case for termination of mStr
				return 0;
		}
	}

	const CharType* mStr;
	size_t mLMatch;
	size_t mRMatch;
};	// MapCharStrComparator

//! Specialization for comparing STL string.
template<typename TCharType>
struct MapComparator<const std::basic_string<TCharType>&>
	: public MapCharStrComparator<TCharType>
{
	typedef TCharType CharType;
	typedef std::basic_string<CharType> StringType;
	typedef MapCharStrComparator<TCharType> Super;

	explicit MapComparator(const StringType& str)
		: Super(str.c_str())
	{
	}

	int compare(const StringType& rhs) {
		return Super::compare(rhs.c_str());
	}
};	// MapComparator

template<
	class TKey,	//! The key type
	class TKeyArg = typename ParamType<const TKey>::RET,
	class TComparator = MapComparator<TKeyArg>
>
class MapBase : public Impl::AvlTree
{
public:
	//! The key type.
	typedef TKey Key;

	//! The type for passing \em Key as function argument.
	typedef TKeyArg KeyArg;

	//! The comparator object type.
	typedef TComparator Comparator;

	/*!	A single node of the MapBase container.
		\note Each node consume 5 * sizeof(void*) that means 20 bytes on 32-bit machine.
	 */
	class NodeBase : public Impl::AvlTree::Node
	{
	public:
		typedef TKey Key;
		typedef TKeyArg KeyArg;

		NodeBase() {}
		explicit NodeBase(KeyArg key) : mKey(key) {}

		/*!	Destroy the node itself.
			By default it will call "delete this", user can override
			this function for their own memory management.
		 */
		sal_override virtual void destroyThis() {
			Impl::AvlTree::Node::destroyThis();
		}

		//! Remove this node from the map, if it's already in the map.
		void removeThis() {
			Impl::AvlTree::Node::removeThis();
		}

		bool isInMap() const {
			return Impl::AvlTree::Node::isInMap();
		}

		//! Returns null if the node is not in any map.
		MapBase* getMap() {
			return static_cast<MapBase*>(Impl::AvlTree::Node::getMap());
		}
		const MapBase* getMap() const {
			return const_cast<NodeBase*>(this)->getMap();
		}

		//! Get the next node (with key >= this key), return null if this is the last node.
		NodeBase* next() {
			return static_cast<NodeBase*>(Node::next());
		}
		const NodeBase* next() const {
			return const_cast<NodeBase*>(this)->next();
		}

		//! Get the previous node (with key <= this key), return null if this is the first node.
		NodeBase* prev() {
			return static_cast<NodeBase*>(Node::prev());
		}
		const NodeBase* prev() const {
			return const_cast<NodeBase*>(this)->prev();
		}

		KeyArg getKey() const {
			return mKey;
		}

		//! Reassign the key value. The map will be adjusted to keep itself strictly sorted.
		void setKey(KeyArg key) {
			MapBase* map = getMap();
			removeThis();
			mKey = key;
			if(map)
				map->insert(*this);
		}

	private:
		friend class MapBase;
#ifndef NDEBUG
		size_t assertValid(size_t& total, sal_maybenull const NodeBase* parent, int dir) const
		{
			if(!this)
				return 0;

			if(parent) {
				Comparator hint(parent->mKey);
				int cmp = hint.compare(mKey);
				MCD_ASSERT(!cmp || (cmp == dir));
			}

			size_t nL = static_cast<NodeBase*>(mChildren[Left])->assertValid(total, this, -1);
			size_t nR = static_cast<NodeBase*>(mChildren[Right])->assertValid(total, this, 1);
			return Impl::AvlTree::Node::assertValid(total, parent, nL, nR);
		}
#endif	// NDEBUG

		Key mKey;
	};	// NodeBase

	//! A wrapper class over the NodeBase to provide typed interface.
	template<class TNodeType>
	class Node : public NodeBase
	{
	public:
		typedef TNodeType NodeType;

		Node() {}
		explicit Node(KeyArg key) : NodeBase(key) {}

		NodeType* next() {
			return static_cast<NodeType*>(NodeBase::next());
		}
		const NodeType* next() const {
			return const_cast<Node*>(this)->next();
		}

		NodeType* prev() {
			return static_cast<NodeType*>(NodeBase::prev());
		}
		const NodeType* prev() const {
			return const_cast<Node*>(this)->prev();
		}
	};	// Node

	NodeBase* find(KeyArg key) {
		return find<true, 0>(key);
	}
	const NodeBase* find(KeyArg key) const {
		return const_cast<MapBase*>(this)->find(key);
	}

	NodeBase* findMin() {
		return static_cast<NodeBase*>(getExtrRootL());
	}
	const NodeBase* findMin() const {
		return const_cast<MapBase*>(this)->findMin();
	}

	NodeBase* findMax() {
		return static_cast<NodeBase*>(getExtrRootR());
	}
	const NodeBase* findMax() const {
		return const_cast<MapBase*>(this)->findMax();
	}

	NodeBase* findSmaller(KeyArg key) {
		return find<false, -1>(key);
	}
	const NodeBase* findSmaller(KeyArg key) const {
		return const_cast<MapBase*>(this)->findSmaller(key);
	}

	NodeBase* findBigger(KeyArg key) {
		return find<false, 1>(key);
	}
	const NodeBase* findBigger(KeyArg key) const {
		return const_cast<MapBase*>(this)->findBigger(key);
	}

	NodeBase* findExactSmaller(KeyArg key) {
		return find<true, -1>(key);
	}
	const NodeBase* findExactSmaller(KeyArg key) const {
		return const_cast<MapBase*>(this)->findExactSmaller(key);
	}

	NodeBase* findExactBigger(KeyArg key) {
		return find<true, 1>(key);
	}
	const NodeBase* findExactBigger(KeyArg key) const {
		return const_cast<MapBase*>(this)->findExactBigger(key);
	}

	void insert(NodeBase& newNode)
	{
		if(mRoot) {
			Comparator hint(newNode.mKey);
			for(NodeBase* node = static_cast<NodeBase*>(mRoot); ;) {
				int nIdx = (hint.compare(node->mKey) < 0);
				if(!node->mChildren[nIdx]) {
					AvlTree::insert(newNode, node, nIdx);
					break;
				}
				node = static_cast<NodeBase*>(node->mChildren[nIdx]);
			}
		}
		else
			AvlTree::insert(newNode, nullptr, 0);
	}

	//! This function will ensure the insertion will not create more duplicated key.
	bool insertUnique(NodeBase& newNode)
	{
		if(find(newNode.getKey()) != nullptr)
			return false;
		insert(newNode);
		return true;
	}

	/*!	Remove all elements in the map.
		To remove a single element, use MapBase::Node::RemoveFromList
	 */
	void removeAll() {
		Impl::AvlTree::removeAll();
	}

	void assertValid() const
	{
#ifndef NDEBUG
		size_t nTotal = 0;
		static_cast<NodeBase*>(mRoot)->assertValid(nTotal, nullptr, 0);
		MCD_ASSERT(nTotal == elementCount());
#endif	// NDEBUG
	}

private:
	template<bool Exact, int Dir>
	NodeBase* find(KeyArg key)
	{
		Comparator hint(key);
		NodeBase* match = nullptr;

		for(NodeBase* node = static_cast<NodeBase*>(mRoot); node;) {
			int cmp = hint.compare(node->mKey);

			if(Exact && cmp == 0)
				return node;	// Exact match.
			else if(cmp == Dir)
				match = node;

			node = static_cast<NodeBase*>(node->mChildren[cmp < 0]);
		}
		return match;
	}
};	// MapBase

/*!	A wrapper class over the MapBase to provide typed interface.
	\sa MapBase
 */
template<class TNode, class TComparator = MapComparator<typename TNode::KeyArg> >
class Map : public MapBase<typename TNode::Key, typename TNode::KeyArg, TComparator>
{
public:
	typedef TNode Node;
	typedef MapBase<typename TNode::Key, typename TNode::KeyArg, TComparator> Super;
	typedef typename Super::KeyArg KeyArg;
	typedef typename Super::Comparator Comparator;

	Node* find(KeyArg key) {
		return static_cast<Node*>(Super::find(key));
	}
	const Node* find(KeyArg key) const {
		return const_cast<Map*>(this)->find(key);
	}

	Node* findMin() {
		return static_cast<Node*>(Super::findMin());
	}
	const Node* findMin(KeyArg key) const {
		return const_cast<Map*>(this)->findMin(key);
	}

	Node* findMax() {
		return static_cast<Node*>(Super::findMax());
	}
	const Node* findMax(KeyArg key) const {
		return const_cast<Map*>(this)->findMax(key);
	}

	Node* findSmaller(KeyArg key) {
		return static_cast<Node*>(Super::findSmaller(key));
	}
	const Node* findSmaller(KeyArg key) const {
		return const_cast<Map*>(this)->findSmaller(key);
	}

	Node* findBigger(KeyArg key) {
		return static_cast<Node*>(Super::findBigger(key));
	}
	const Node* findBigger(KeyArg key) const {
		return const_cast<Map*>(this)->findBigger(key);
	}

	Node* findExactSmaller(KeyArg key) {
		return static_cast<Node*>(Super::findExactSmaller(key));
	}
	const Node* findExactSmaller(KeyArg key) const {
		return const_cast<Map*>(this)->findExactSmaller(key);
	}

	Node* findExactBigger(KeyArg key) {
		return static_cast<Node*>(Super::findExactBigger(key));
	}
	const Node* findExactBigger(KeyArg key) const {
		return const_cast<Map*>(this)->findExactBigger(key);
	}

	void insert(Node& newNode) {
		Super::insert(newNode);
	}

	bool insertUnique(Node& newNode) {
		return Super::insertUnique(newNode);
	}
};	// Map

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_MAP__
