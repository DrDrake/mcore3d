#ifndef __MCD_CORE_SYSTEM_LINKLIST__
#define __MCD_CORE_SYSTEM_LINKLIST__

#include "../ShareLib.h"
#include "NonCopyable.h"

namespace MCD {

/*!	The base class for an intrusive double linked list.
	To use the link list, user have to extend the node first:
	\code
	struct FooNode : public LinkListBase::Node<FooNode> {
		FooNode(int id) : mId(id) {}
		int mId;
	};	// FooNode

	// ...

	LinkList<FooNode> list;
	list.pushBack(*(new FooNode(123)));
	\endcode

	The syntax for iterating the list is similar to STL:
	\code
	for(Node* n = &list.front(); n != list.end(); n = n->next()) {}
	\endcode
	Note that \em next will not return null for the tail, you must use \em End
	to check for end of list.

	You can reverse iterate the list by:
	\code
	for(Node* n = &list.back(); n != list.rend(); n = n->prev()) {}
	\endcode

	To add an element into multiple list (or similar intrusive container such as Map):
	\code
	struct ClientInfo {
		struct Client : public LinkListBase::NodeBase {
			MCD_DECLAR_GET_OUTER_OBJ(ClientInfo, mClient);
			void destroyThis() throw() {
				delete getOuterSafe();
			}
		} mClient;

		struct Server : public LinkListBase::NodeBase {
			MCD_DECLAR_GET_OUTER_OBJ(ClientInfo, mServer);
			void destroyThis() throw() {
				delete getOuterSafe();
			}
		} mServer;

		std::string mName;
	};	// ClientInfo

	// ...

	LinkList<ClientInfo::Client> clientList;
	LinkList<ClientInfo::Server> serverList;

	std::auto_ptr<ClientInfo> info(new ClientInfo);
	info->mName = "Test";
	clientList.pushBack(info->mClient);
	serverList.pushBack(info->mServer);

	ClientInfo* p = clientList.front().getOuterSafe();
	\endcode

	\note The link list is not copyable.
	\note All nodes will be deleted when the list destroy.

	\note Implementation details: It's a very simple double linked list
		with a dummy head and dummy tail node so that the code for insertion and
		removal are much simpler because no special care at both ends. But a little
		disadvantage is that you cannot determine the end of the list by comparing
		the prev/next pointer of a node with a null value. Therefore we have to
		compare with \em End and \em REnd when iterating the list.
 */
class MCD_CORE_API LinkListBase : private Noncopyable
{
public:
	/*!	A double link list node.
		\note Each node consume 3 * sizeof(void*) that means 12 bytes on 32-bit machine.
	 */
	class MCD_CORE_API NodeBase
	{
		friend class LinkListBase;

	public:
		NodeBase();

		//! The destructor will remove this node from the list.
		virtual ~NodeBase();

		/*!	Destroy the node itself.
			By default it will call "delete this", user can override
			this function for their own memory management.
		 */
		virtual void destroyThis() throw();

		//! Remove this node from the list, if it's already in the list.
		void removeThis() throw();

		bool isInList() const {
			return mList != nullptr;
		}

		//! Returns null if the node is not in any list.
		sal_maybenull LinkListBase* getList() {
			return mList;
		}
		sal_maybenull const LinkListBase* getList() const {
			return mList;
		}

		/*! Get the next node.
			\note Will not return null even \em this is already the last node, because of the dummy head and tail in LinkListBase
		 */
		sal_notnull NodeBase* next() {
			return mNext;
		}
		sal_notnull const NodeBase* next() const {
			return mNext;
		}

		/*!	Get the previous node.
			\note Will not return null even \em this is already the last node, because of the dummy head and tail in LinkListBase
		 */
		sal_notnull NodeBase* prev() {
			return mPrev;
		}
		sal_notnull const NodeBase* prev() const {
			return mPrev;
		}

	protected:
		LinkListBase* mList;
		NodeBase* mPrev;
		NodeBase* mNext;
	};	// NodeBase

	//! A wrapper class over the NodeBase to provide typed interface.
	template<class TNodeType>
	class Node : public NodeBase
	{
	public:
		typedef TNodeType NodeType;

		//! Get the next node.
		sal_notnull NodeType* next() {
			return static_cast<NodeType*>(NodeBase::next());
		}
		sal_notnull const NodeType* next() const {
			return static_cast<const NodeType*>(NodeBase::next());
		}

		//! Get the previous node.
		sal_notnull NodeType* prev() {
			return static_cast<NodeType*>(NodeBase::prev());
		}
		sal_notnull const NodeType* prev() const {
			return static_cast<const NodeType*>(NodeBase::prev());
		}
	};	// Node

public:
	LinkListBase();
	~LinkListBase();

	//! Get the first node, assertion if the list is empty.
	NodeBase& front() {
		MCD_ASSERT(!isEmpty());
		return *mHead->mNext;
	}
	const NodeBase& front() const {
		MCD_ASSERT(!isEmpty());
		return *mHead->mNext;
	}

	//! Get the last node, assertion if the list is empty.
	NodeBase& back() {
		MCD_ASSERT(!isEmpty());
		return *mTail->mPrev;
	}
	const NodeBase& back() const {
		MCD_ASSERT(!isEmpty());
		return *mTail->mPrev;
	}

	//! Get the node beyond the last node, for use when forward iterating the list.
	sal_notnull NodeBase* end() {
		return mTail;
	}
	sal_notnull const NodeBase* end() const {
		return mTail;
	}

	//! Get the node before the first node, for use when reverse iterating the list.
	sal_notnull NodeBase* rend() {
		return mHead;
	}
	sal_notnull const NodeBase* rend() const {
		return mHead;
	}

	//! Please make sure \em newNode is not already in any list.
	void pushFront(NodeBase& newNode);

	//! Please make sure \em newNode is not already in any list.
	void pushBack(NodeBase& newNode);

	/*!	Insert \em newNode before the node \em beforeThis.
		Assertion fail if \em newNode is not already in any list and \em beforeThis
		must already in this list.
	 */
	void insertBefore(NodeBase& newNode, const NodeBase& beforeThis);

	/*!	Insert \em newNode after the node \em afterThis.
		Assertion fail if \em newNode is not already in any list and \em afterThis
		must already in this list.
	 */
	void insertAfter(NodeBase& newNode, const NodeBase& afterThis);

	/*!	Remove all elements in the list.
		To remove a single element, use NodeBase::RemoveThis
	 */
	void removeAll();

	/*!	Destroy all elements in the list.
		To destroy a single element, use NodeBase::DestroyThis
	 */
	void destroyAll();

	size_t elementCount() const {
		return mCount;
	}

	bool isEmpty() const {
		return mCount == 0;
	}

protected:
	NodeBase* mHead;	//!< The head of the list (a dummy node)
	NodeBase* mTail;	//!< The tail of the list (a dummy node)
	size_t mCount;
};	// LinkListBase

/*!	A wrapper class over the LinkListBase to provide typed interface.
	\sa LinkListBase
 */
template<class NodeType>
class LinkList : public LinkListBase
{
public:
	typedef NodeType Node;

	LinkList() : LinkListBase() {}

	Node& front() {
		return static_cast<Node&>(LinkListBase::front());
	}
	const Node& front() const {
		return static_cast<const Node&>(LinkListBase::front());
	}

	Node& back() {
		return static_cast<Node&>(LinkListBase::back());
	}
	const Node& back() const {
		return static_cast<const Node&>(LinkListBase::back());
	}

	void pushFront(Node& newNode) {
		LinkListBase::pushFront(newNode);
	}

	void pushBack(Node& newNode) {
		LinkListBase::pushBack(newNode);
	}

	void insertBefore(Node& newNode, const Node& beforeThis) {
		LinkListBase::insertBefore(newNode, beforeThis);
	}

	void insertAfter(Node& newNode, const Node& afterThis) {
		LinkListBase::insertAfter(newNode, afterThis);
	}
};	// LinkList

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_LINKLIST__
