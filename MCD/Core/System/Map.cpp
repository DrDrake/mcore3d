#include "Pch.h"
#include "Map.h"
#include <memory>	// For std::swap (VC only needs <utilty> but gcc need <memory>)

#ifdef MCD_VC
#	include <malloc.h>	// _malloca
#endif

namespace MCD {

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

void AvlTree::Node::destroyThis()
{
	delete this;
}

void AvlTree::Node::removeThis()
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
	MCD_ASSERT((this == mParent->mChildren[Left]) || (this == mParent->mChildren[Right]));
	return this == mParent->mChildren[Left] ? Left : Right;
}

void AvlTree::Node::setChildSafe(Direction dir, Node* child)
{
	if((mChildren[dir] = child) != nullptr)
		child->mParent = this;

	synLeftRight();
}

#ifndef NDEBUG
MCD_INLINE2 size_t max(size_t lhs, size_t rhs)
{
	return lhs > rhs ? lhs : rhs;
}

size_t AvlTree::Node::assertValid(size_t& total, const Node* parent, size_t nL, size_t nR) const
{
	MCD_ASSERT(mParent == parent);
	MCD_ASSERT(isBallanceOk());
	MCD_ASSERT(nL + mBallance == nR);

	++total;
	return max(nL, nR) + 1;
}

void AvlTree::Node::synLeftRight()
{
	mRight = mChildren[Right];
}

#else

MCD_INLINE2 void AvlTree::Node::synLeftRight()
{
}

#endif	// NDEBUG

AvlTree::AvlTree()
	: mRoot(nullptr), mCount(0)
{
}

AvlTree::~AvlTree()
{
	destroyAll();
}

bool AvlTree::isEmpty() const
{
	MCD_ASSERT(!mRoot == (mCount == 0));
	return !mRoot;
}

void AvlTree::insert(Node& node, Node* parent, int nIdx)
{
	node.mChildren[Left] = nullptr;
	node.mChildren[Right] = nullptr;
	node.synLeftRight();

	node.mParent = parent;
	node.mAvlTree = this;
	node.mBallance = 0;

	if(parent) {
		MCD_ASSERT(!parent->mChildren[nIdx]);
		parent->mChildren[nIdx] = &node;
		parent->synLeftRight();

		adjustBallance(*parent, nIdx ? 1 : -1, false);
	} else {
		MCD_ASSERT(!mRoot);
		mRoot = &node;
	}

	++mCount;
}

void AvlTree::remove(Node& node, Node* onlyChild)
{
	MCD_ASSERT(!node.mChildren[Left] || !node.mChildren[Right]);

	if(onlyChild)
		onlyChild->mParent = node.mParent;

	Node* parent = node.mParent;
	if(parent) {
		Node::Direction idx = node.parentIdx();
		parent->mChildren[idx] = onlyChild;
		parent->synLeftRight();

		adjustBallance(*parent, idx ? -1 : 1, true);
	} else
		mRoot = onlyChild;

	MCD_ASSERT(mCount > 0);
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
	// NOTE: The simplest way, but too slow because
	// all the tree balancing act is just a waste.
//	while(!isEmpty())
//		mRoot->removeThis();

	// A fast removeAll approach
	for(Node* n = getExtrRootL(); n != nullptr; ) {
		Node* bk = n;
		n = n->next();
		bk->mAvlTree = nullptr;
	}
	mRoot = nullptr;
	mCount = 0;
}

void AvlTree::destroyAll()
{
	// NOTE: The simplest way, but too slow because
	// all the tree balancing act is just a waste.
//	while(!isEmpty())
//		mRoot->destroyThis();

	// NOTE: A faster destroy all approach, but use more temporary memory
	Node** tmp = (Node**)MCD_STACKALLOCA(mCount * sizeof(Node*));
	size_t i = 0;
	for(Node* n = getExtrRootL(); n != nullptr; n = n->next(), ++i)
		tmp[i] = n;

	for(; mCount--; ) {
		tmp[mCount]->mAvlTree = nullptr;
		tmp[mCount]->destroyThis();
	}

	mCount = 0;
	mRoot = nullptr;

	MCD_STACKFREE(tmp);
}

void AvlTree::replaceFixTop(Node& node, Node& next)
{
	if((next.mParent = node.mParent) != nullptr) {
		node.mParent->mChildren[node.parentIdx()] = &next;
		node.mParent->synLeftRight();
	}
	else {
		MCD_ASSERT(&node == mRoot);
		mRoot = &next;
	}
}

bool AvlTree::rotate(Node& node, int dir)
{
	MCD_ASSERT((-1 == dir) || (1 == dir));
	int nIdx = (Right == dir);

	MCD_ASSERT(node.mBallance);
	MCD_ASSERT(node.mBallance * dir < 0);

	Node* pNext = node.mChildren[!nIdx];
	MCD_ASSUME(pNext != nullptr);
	MCD_ASSERT(pNext->isBallanceOk());

	if(dir == pNext->mBallance) {
		MCD_VERIFY(!rotate(*pNext, -dir));
		pNext = pNext = node.mChildren[!nIdx];
		MCD_ASSERT(pNext && pNext->isBallanceOk());
		MCD_ASSERT(dir != pNext->mBallance);
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
	pNext->synLeftRight();

	replaceFixTop(node, *pNext);

	node.mParent = pNext;

	return bDepthDecrease;
}

void AvlTree::adjustBallance(Node& node_, int dir, bool removed)
{
	MCD_ASSUME((1 == dir) || (-1 == dir));
	Node* node = &node_;

	while(true) {
		MCD_ASSERT(node->isBallanceOk());

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
			MCD_ASSERT(false);
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

}	// namespace MCD
