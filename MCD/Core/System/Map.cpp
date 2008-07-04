#include "Pch.h"
#include "Map.h"
#include <memory>	// For std::swap (VC only needs <utilty> but gcc need <memory>)

namespace SGE {

namespace Impl {

// Note that only mAvlTree need to be initialized, other variables will be
// set correctly once the node is inserted into the map
AvlTree::Node::Node()
	: mAvlTree(nullptr)
{
}

AvlTree::Node::~Node()
{
	removeThis();
}

void AvlTree::Node::destroyThis() throw()
{
	delete this;
}

void AvlTree::Node::removeThis() throw()
{
	if(!mAvlTree)
		return;

	mAvlTree->remove(*this);
	mAvlTree = nullptr;
}

bool AvlTree::Node::isBallanceOk() const
{
	return (mBallance >= -1) && (mBallance <= 1);
}

AvlTree::Node::Direction AvlTree::Node::parentIdx() const
{
	SGE_ASSERT((this == mParent->mChildren[Left]) || (this == mParent->mChildren[Right]));
	return this == mParent->mChildren[Left] ? Left : Right;
}

void AvlTree::Node::setChildSafe(Direction dir, Node* child)
{
	if((mChildren[dir] = child) != nullptr)
		child->mParent = this;
}

#ifndef NDEBUG
SGE_INLINE2 size_t max(size_t lhs, size_t rhs)
{
	return lhs > rhs ? lhs : rhs;
}

size_t AvlTree::Node::assertValid(size_t& total, const Node* parent, size_t nL, size_t nR) const
{
	SGE_ASSERT(mParent == parent);
	SGE_ASSERT(isBallanceOk());
	SGE_ASSERT(nL + mBallance == nR);

	++total;
	return max(nL, nR) + 1;
}

#endif	// NDEBUG

AvlTree::AvlTree()
	: mRoot(nullptr), mCount(0)
{
}

AvlTree::~AvlTree() throw()
{
	destroyAll();
}

bool AvlTree::isEmpty() const
{
	SGE_ASSERT(!mRoot == (mCount == 0));
	return !mRoot;
}

void AvlTree::insert(Node& node, Node* parent, int nIdx)
{
	node.mChildren[Left] = nullptr;
	node.mChildren[Right] = nullptr;
	node.mParent = parent;
	node.mAvlTree = this;
	node.mBallance = 0;

	if(parent) {
		SGE_ASSERT(!parent->mChildren[nIdx]);
		parent->mChildren[nIdx] = &node;

		adjustBallance(*parent, nIdx ? 1 : -1, false);

	} else {
		SGE_ASSERT(!mRoot);
		mRoot = &node;
	}

	++mCount;
}

void AvlTree::remove(Node& node, Node* onlyChild)
{
	SGE_ASSERT(!node.mChildren[Left] || !node.mChildren[Right]);

	if(onlyChild)
		onlyChild->mParent = node.mParent;

	Node* parent = node.mParent;
	if(parent) {
		Node::Direction idx = node.parentIdx();
		parent->mChildren[idx] = onlyChild;

		adjustBallance(*parent, idx ? -1 : 1, true);

	} else
		mRoot = onlyChild;

	--mCount;
}

void AvlTree::remove(Node& node)
{
	if(node.mChildren[Left])
		if(node.mChildren[Right]) {
			// Find the successor of this node.
			Node* pSucc = getExtreme<Left>(*node.mChildren[Right]);

			remove(*pSucc, pSucc->mChildren[Right]);

			pSucc->setChildSafe(Node::Left, node.mChildren[Left]);
			pSucc->setChildSafe(Node::Right, node.mChildren[Right]);
			replaceFixTop(node, *pSucc);

			pSucc->mBallance = node.mBallance;

		} else
			remove(node, node.mChildren[Left]);
	else
		remove(node, node.mChildren[Right]);
}

void AvlTree::removeAll()
{
	while(!isEmpty())
		mRoot->removeThis();
}

void AvlTree::destroyAll() throw()
{
	while(!isEmpty())
		mRoot->destroyThis();
}

void AvlTree::replaceFixTop(Node& node, Node& next)
{
	if((next.mParent = node.mParent) != nullptr)
		node.mParent->mChildren[node.parentIdx()] = &next;
	else {
		SGE_ASSERT(&node == mRoot);
		mRoot = &next;
	}
}

bool AvlTree::rotate(Node& node, int dir)
{
	SGE_ASSERT((-1 == dir) || (1 == dir));
	int nIdx = (Right == dir);

	SGE_ASSERT(node.mBallance);
	SGE_ASSERT(node.mBallance * dir < 0);

	Node* pNext = node.mChildren[!nIdx];
	SGE_ASSUME(pNext != nullptr);
	SGE_ASSERT(pNext->isBallanceOk());

	if(dir == pNext->mBallance) {
		SGE_VERIFY(!rotate(*pNext, -dir));
		pNext = pNext = node.mChildren[!nIdx];
		SGE_ASSERT(pNext && pNext->isBallanceOk());
		SGE_ASSERT(dir != pNext->mBallance);
	}

	bool bDepthDecrease = pNext->mBallance && !node.isBallanceOk();

	node.mBallance += dir;
	if(pNext->mBallance) {
		if(!node.mBallance)
			pNext->mBallance += dir;
		node.mBallance += dir;
	}
	pNext->mBallance += dir;

	node.setChildSafe(Node::Direction(!nIdx), pNext->mChildren[nIdx]);

	pNext->mChildren[nIdx] = &node;

	replaceFixTop(node, *pNext);

	node.mParent = pNext;

	return bDepthDecrease;
}

void AvlTree::adjustBallance(Node& node_, int dir, bool removed)
{
	SGE_ASSUME((1 == dir) || (-1 == dir));
	Node* node = &node_;

	while(true) {
		SGE_ASSERT(node->isBallanceOk());

		Node* parent = node->mParent;
		node->mBallance += dir;

		int nDirNext = -1;	// O no
		if(parent)
			nDirNext = ((Left != Direction(node->parentIdx())) ^ removed) ? 1 : -1;

		bool match = false;
		switch(node->mBallance) {
		case -1:
		case 1:
			match = false;
			break;
		case 0:
			match = true;
			break;
		case -2:
			match = rotate(*node, 1);
			break;
		case 2:
			match = rotate(*node, -1);
			break;
		default:
			SGE_ASSERT(false);
			NoReturn();
		}

		if(!parent || (match ^ removed))
			break;

		node = parent;
		dir = nDirNext;
	}
}

AvlTree::Node* AvlTree::walkL(Node& node)
{
	return walk<Left>(node);
}

AvlTree::Node* AvlTree::walkR(Node& node)
{
	return walk<Right>(node);
}

template<AvlTree::Direction dir>
AvlTree::Node* AvlTree::walk(Node& node)
{
	Node* ret = node.mChildren[dir];
	if(ret)
		return getExtreme<dir == Left ? Right : Left>(*ret);

	ret = &node;
	while(true) {
		Node* parent = ret->mParent;
		if(!parent)
			return nullptr;

		if(Direction(ret->parentIdx()) != dir)
			return parent;

		ret = parent;
	}
}

AvlTree::Node* AvlTree::getExtremeL(Node& node)
{
	return getExtreme<Left>(node);
}

AvlTree::Node* AvlTree::getExtremeR(Node& node)
{
	return getExtreme<Right>(node);
}

// In VS2005, the body of GetExtr must below GetExtrL and GetExtrR, otherwise
// the function template will not be exported. What a strange behavior!
template<AvlTree::Direction dir>
AvlTree::Node* AvlTree::getExtreme(Node& pos_)
{
	Node* pos = &pos_;
	while(true) {
		Node* pVal = pos->mChildren[dir];
		if(!pVal)
			return pos;
		pos = pVal;
	}
}

}	// namespace Impl

}	// namespace SGE
