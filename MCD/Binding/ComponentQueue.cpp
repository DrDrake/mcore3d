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
		explicit Time(float key) : Node<Time>(key) {}
		MCD_DECLAR_GET_OUTER_OBJ(QueueNode, mTime);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	} mTime;

	struct Component : public MapBase<ComponentPtr>::NodeBase {
		explicit Component(const ComponentPtr& key) : NodeBase(key) {}
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

			if(node->mTime.getKey() <= currentTime) {
				if(Component* c = node->mComponent.getKey().get())
					return c;
				delete node;	// Do garbage collection
			}
		}

		return nullptr;
	}

	Map<QueueNode::Time> timeToComponent;
	Map<QueueNode::Component> componentToTime;
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
	const QueueNode* getQueueNode() const {
		// TODO: Remove this hack
		return queueNode ? queueNode : (QueueNode*)1;
	}
	const Component* component;
	const QueueNode* queueNode;
};	// ComponentQueueResult

SCRIPT_CLASS_DECLAR(ComponentQueueResult);
SCRIPT_CLASS_DECLAR(ComponentQueue::QueueNode);

static ComponentQueueResult* componentQueueGetItem(ComponentQueue& self, float currentTime, const ComponentQueue::QueueNode* begin)
{
	ComponentQueueResult result;
	// TODO: Remove this hack
	if(int(begin) == 1)
		begin = nullptr;
	result.queueNode = begin;
	result.component = self.getItem(currentTime, result.queueNode);
	return new ComponentQueueResult(result);
}
SCRIPT_CLASS_REGISTER_NAME(ComponentQueue::QueueNode, "__ComponentQueueNode__")
;}

SCRIPT_CLASS_REGISTER_NAME(ComponentQueueResult, "ComponentQueueResult")
	.enableGetset(L"ComponentQueueResult")
	.constructor()
	.method<objNoCare>(L"_getcomponent", &ComponentQueueResult::getComponent)
	.method<objNoCare>(L"_getqueueNode", &ComponentQueueResult::getQueueNode)
;}

SCRIPT_CLASS_REGISTER_NAME(ComponentQueue, "ComponentQueue")
	.constructor()
	.method(L"setItem", &ComponentQueue::setItem)
	.wrappedMethod(L"getItem", &componentQueueGetItem)
;}

}	// namespace script

void MCD::registerComponentQueueBinding(script::VMCore* v)
{
	script::ClassTraits<ComponentQueue>::bind(v);
	script::ClassTraits<ComponentQueue::QueueNode>::bind(v);
	script::ClassTraits<script::ComponentQueueResult>::bind(v);
}
