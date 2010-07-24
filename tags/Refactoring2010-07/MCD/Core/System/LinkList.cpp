#include "Pch.h"
#include "LinkList.h"

namespace MCD {

LinkListBase::NodeBase::NodeBase()
	: mList(nullptr), mPrev(nullptr), mNext(nullptr)
{
}

LinkListBase::NodeBase::~NodeBase()
{
	removeThis();
}

void LinkListBase::NodeBase::destroyThis() throw()
{
	delete this;
}

void LinkListBase::NodeBase::removeThis() throw()
{
	if(!mList)
		return;

	--(mList->mCount);

	mPrev->mNext = mNext;
	mNext->mPrev = mPrev;

	mList = nullptr;
	mNext = mPrev = nullptr;
}

LinkListBase::LinkListBase()
	: mHead(new NodeBase), mTail(new NodeBase), mCount(0)
{
	mHead->mNext = mTail;
	mTail->mPrev = mHead;
}

LinkListBase::~LinkListBase()
{
	destroyAll();
	delete mHead;
	delete mTail;
}

void LinkListBase::insertBefore(NodeBase& newNode, const NodeBase& beforeThis)
{
	MCD_ASSERT(!newNode.isInList());
	MCD_ASSERT("Parameter 'beforeThis' should be in this list" &&
		beforeThis.mNext == nullptr || beforeThis.getList() == this
	);

	beforeThis.mPrev->mNext = &newNode;
	newNode.mPrev = beforeThis.mPrev;

	const_cast<NodeBase&>(beforeThis).mPrev = &newNode;
	newNode.mNext = const_cast<NodeBase*>(&beforeThis);

	newNode.mList = this;
	++mCount;
}

void LinkListBase::pushFront(NodeBase& newNode)
{
	insertAfter(newNode, *mHead);
}

void LinkListBase::pushBack(NodeBase& newNode)
{
	insertBefore(newNode, *mTail);
}

void LinkListBase::insertAfter(NodeBase& newNode, const NodeBase& afterThis)
{
	insertBefore(newNode, *afterThis.mNext);
}

void LinkListBase::removeAll()
{
	NodeBase* n;
	// Excluding the dummy node mHead and mTail
	while(mTail != (n = mHead->mNext)) {
		n->removeThis();
	}
}

void LinkListBase::destroyAll()
{
	NodeBase* n;
	// Excluding the dummy node mHead and mTail
	while(mTail != (n = mHead->mNext)) {
		n->destroyThis();
	}
}

}	// namespace MCD
