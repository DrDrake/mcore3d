#include "Pch.h"
#include "../../../MCD/Core/System/LinkList.h"
#include "../../../MCD/Core/System/Macros.h"
#include <memory>   // For auto_ptr
#include <stdexcept>
#include <vector>

using namespace MCD;

#ifdef MCD_VC
#	pragma warning(disable : 6211)
#endif

namespace {

struct FooNode : public LinkListBase::Node<FooNode> {
	FooNode(int id) : mId(id) {}
	int mId;
};	// FooNode

}	// namespace

TEST(Basic_LinkListTest)
{
	{	// Operations for the node itself
		std::auto_ptr<FooNode> n(new FooNode(123));
		CHECK(!n->isInList());
		CHECK(!n->next());
		CHECK(!n->prev());
		n->removeThis();	// Should do nothing since not in list
		n->destroyThis();
		n.release();
	};

	{	LinkList<FooNode> list;
		CHECK(list.isEmpty());

		// Iterate though empty list
		size_t i = 0;
		for(FooNode* n = list.begin(); n != list.end(); n = n->next()) {
			++i;
		}
		CHECK_EQUAL(0u, i);

		static const size_t cCount = 10;
		for(size_t id=0; id<cCount; ++id) {
			list.pushFront(*(new FooNode(cCount-1-id)));
			list.pushBack(*(new FooNode(id+cCount)));
		}

		CHECK_EQUAL(cCount * 2, list.elementCount());
		CHECK(!list.isEmpty());

		// Iterate though the list
		i = 0;
		for(FooNode* n = list.begin(); n != list.end(); n = n->next()) {
			CHECK_EQUAL(int(i), n->mId);
			++i;
		}

		// Reverse iteration
		for(FooNode* n = list.rbegin(); n != list.rend(); n = n->prev()) {
			--i;
			CHECK_EQUAL(int(i), n->mId);
		}

		{	// Remove a specific node
			FooNode& n = list.front();
			n.removeThis();
			CHECK_EQUAL(cCount * 2 - 1, list.elementCount());
			CHECK_EQUAL(1, list.front().mId);
			delete &n;
		}

		{	// Destroy a specific node
			FooNode& n = list.front();
			n.destroyThis();
			CHECK_EQUAL(cCount * 2 - 2, list.elementCount());
			CHECK_EQUAL(2, list.front().mId);
		}

		// list get out of scope and all element are destroyed
	}
}

TEST(Insert_LinkListTest)
{
	LinkList<FooNode> list;

	FooNode* f1 = new FooNode(1);
	FooNode* f2 = new FooNode(2);
	FooNode* f3 = new FooNode(3);

	list.pushBack(*f3);
	list.insertBefore(*f1, *f3);
//	list.insertBefore(*f2, *f3);
	list.insertAfter(*f2, *f1);

	size_t i = 1;
	for(FooNode* n = &list.front(); n != list.end(); n = n->next()) {
		CHECK_EQUAL(int(i), n->mId);
		++i;
	}
}

TEST(RemoveAll_LinkListTest)
{
	LinkList<FooNode> list;
	std::vector<FooNode*> vec;
	static const size_t cCount = 10;
	for(size_t i=0; i<cCount; ++i) {
		std::auto_ptr<FooNode> ptr(new FooNode(i));
		list.pushBack(*ptr);
		CHECK_EQUAL(&list, ptr->getList());
		vec.push_back(ptr.get());
		ptr.release();
	}

	list.removeAll();
	CHECK(list.isEmpty());

	for(size_t i=0; i<cCount; ++i) {
		delete vec[i];
	}
}

TEST(DestroyAll_LinkListTest)
{
	LinkList<FooNode> list;
	list.destroyAll();

	static const size_t cCount = 10;
	for(size_t i=0; i<cCount; ++i) {
		std::auto_ptr<FooNode> ptr(new FooNode(i));
		list.pushBack(*ptr);
		ptr.release();
	}

	CHECK_EQUAL(cCount, list.elementCount());
	list.destroyAll();
	CHECK(list.isEmpty());
}

namespace {

struct ClientInfo
{
	struct Client : public LinkListBase::NodeBase {
		MCD_DECLAR_GET_OUTER_OBJ(ClientInfo, mClient);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	} mClient;

	struct Server : public LinkListBase::NodeBase {
		MCD_DECLAR_GET_OUTER_OBJ(ClientInfo, mServer);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	} mServer;

	std::string mName;
};	// ClientInfo

}	// namespace

TEST(Multiple_LinkListTest)
{
	LinkList<ClientInfo::Client> clientList;
	LinkList<ClientInfo::Server> serverList;

	std::auto_ptr<ClientInfo> info(new ClientInfo);
	info->mName = "Test";
	clientList.pushBack(info->mClient);
	serverList.pushBack(info->mServer);

	CHECK_EQUAL(1u, clientList.elementCount());
	CHECK_EQUAL(1u, serverList.elementCount());

	ClientInfo* p = clientList.front().getOuterSafe();
	if(p) {
		CHECK_EQUAL("Test", p->mName);
	}

	CHECK_EQUAL(p, serverList.front().getOuterSafe());

	// Force to destroy the client info
	info.reset();

	// The lists should be empty
	CHECK(clientList.isEmpty());
	CHECK(serverList.isEmpty());
}
