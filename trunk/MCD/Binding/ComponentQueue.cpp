#include "Pch.h"
#include "ComponentQueue.h"
#include "../Core/Entity/Component.h"
#include "../Core/System/Macros.h"
#include "../Core/System/Map.h"

namespace MCD {

// A node for implementing bi-directional map
// See SystemTest -> BiDirectional_MapTest for more information
class ComponentQueue::QueueNode
{
public:
	QueueNode(float time, Component& component)
		: mTime(time), mComponent(&component)
	{
	}

	struct Time : public MapBase<float>::Node<Time> {
		explicit Time(float key) : MapBase<float>::Node<Time>(key) {}
		MCD_DECLAR_GET_OUTER_OBJ(QueueNode, mTime);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	} mTime;

	struct Component_ : public MapBase<ComponentPtr>::NodeBase {
		explicit Component_(const ComponentPtr& key) : NodeBase(key) {}
		MCD_DECLAR_GET_OUTER_OBJ(QueueNode, mComponent);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	} mComponent;
};	// QueueNode

class ComponentQueue::Impl
{
public:
	void setItem(float wakeUpTime, MCD::Component& component)
	{
		QueueNode* node = componentToTime.find(&component)->getOuterSafe();

		if(node)
			node->mTime.setKey(wakeUpTime);
		else {
			// TODO: Make use of the find() operation preformed above to make the
			// insert() operation much faster
			node = new QueueNode(wakeUpTime, component);
			timeToComponent.insert(node->mTime);
			componentToTime.insert(node->mComponent);
		}
	}

	MCD::Component* getItem(float currentTime, const QueueNode*& begin)
	{
		const QueueNode* node = nullptr;

		// A starting node is supplied, use it
		if(begin)
			node = begin;
		// No starting node is supplied, start from the very beginning
		else
			node = timeToComponent.findMin()->getOuterSafe();

		if(node) {
			// Returns the node's next position
			begin = node->mTime.next()->getOuterSafe();

			const QueueNode* next = node;
			while(node && node->mTime.getKey() <= currentTime)
			{
				begin = node->mTime.next()->getOuterSafe();

				if(Component* c = next->mComponent.getKey().get())
					return node == next ? c : nullptr;	// node == next means the first iteration of the loop

				// Do garbage collection and remove all null node
				delete next;
				node = next = begin;
			}
		}

		return nullptr;
	}

	Map<QueueNode::Time> timeToComponent;
	Map<QueueNode::Component_> componentToTime;
};	// Impl

ComponentQueue::ComponentQueue()
{
	mImpl = new Impl();
}

ComponentQueue::~ComponentQueue()
{
	delete mImpl;
}

void ComponentQueue::setItem(float wakeUpTime, Component& component)
{
	MCD_ASSUME(mImpl);
	mImpl->setItem(wakeUpTime, component);
}

Component* ComponentQueue::getItem(float currentTime, const ComponentQueue::QueueNode*& begin)
{
	MCD_ASSUME(mImpl);
	return mImpl->getItem(currentTime, begin);
}

void registerComponentQueueBinding(script::VMCore* v);

}	// namespace MCD

#include "Entity.h"

namespace script {

using namespace MCD;

//! A structure for squirrel to return both Component and QueueNode at the same time
struct ComponentQueueResult
{
	typedef ComponentQueue::QueueNode QueueNode;
	ComponentQueueResult() : component(nullptr), queueNode(nullptr) {}
	const Component* getComponent() const { return component; }
	const QueueNode* getQueueNode() const { return queueNode; }
	const Component* component;
	const QueueNode* queueNode;
};	// ComponentQueueResult

SCRIPT_CLASS_DECLAR(ComponentQueueResult);
SCRIPT_CLASS_DECLAR(ComponentQueue::QueueNode);

// NOTE: This function is not thread safe, cannot' be invoked by more than one thread at a time.
static ComponentQueueResult* componentQueueGetItem(ComponentQueue& self, float currentTime, const ComponentQueue::QueueNode* begin)
{
	// NOTE: This function can be thread safe if this variable use TLS
	static ComponentQueueResult result;
	result.queueNode = begin;
	result.component = self.getItem(currentTime, result.queueNode);

	// NOTE: We are returning the address of a static variable!
	// Since the usage pattern of ComponentQueueResult is just very temporary, therefore
	// instead of dynamically allocating new ComponentQueueResult, we reuse the static
	// variable. So, make sure ComponentQueue.getItem() will only invoked in only one
	// thread, and the returning script variable will not be stored.
	return &result;
}
SCRIPT_CLASS_REGISTER_NAME(ComponentQueue::QueueNode, "__ComponentQueueNode__")
;}

SCRIPT_CLASS_REGISTER_NAME(ComponentQueueResult, "ComponentQueueResult")
	.enableGetset()
	.constructor()
	.method<objNoCare>(xSTRING("_getcomponent"), &ComponentQueueResult::getComponent)
	.method<objNoCare>(xSTRING("_getqueueNode"), &ComponentQueueResult::getQueueNode)
;}

SCRIPT_CLASS_REGISTER_NAME(ComponentQueue, "ComponentQueue")
	.constructor()
	.method(xSTRING("setItem"), &ComponentQueue::setItem)
	.wrappedMethod<objNoCare>(xSTRING("getItem"), &componentQueueGetItem)	// NOTE: Use <objNoCare>, more info in the comments of componentQueueGetItem.
;}

}	// namespace script

void MCD::registerComponentQueueBinding(script::VMCore* v)
{
	script::ClassTraits<ComponentQueue>::bind(v);
	script::ClassTraits<ComponentQueue::QueueNode>::bind(v);
	script::ClassTraits<script::ComponentQueueResult>::bind(v);
}
