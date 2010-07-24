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
	: mImpl(*new Impl)
{
}

ComponentQueue::~ComponentQueue()
{
	delete &mImpl;
}

void ComponentQueue::setItem(float wakeUpTime, Component& component)
{
	mImpl.setItem(wakeUpTime, component);
}

Component* ComponentQueue::getItem(float currentTime, const ComponentQueue::QueueNode*& begin)
{
	return mImpl.getItem(currentTime, begin);
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

static int componentQueueUpdate(HSQUIRRELVM vm)
{
	script::detail::StackHandler sa(vm);
	ComponentQueue& q = get(types::TypeSelect<ComponentQueue&>(), vm, 1);
	float time = sa.getFloat(2);

	const ComponentQueue::QueueNode* queueNode = nullptr;
	do {
		Component* c = q.getItem(time, queueNode);
		Entity* e;
		if(c && (e = c->entity()) != nullptr && e->enabled) {
			objNoCare::pushResult(vm, c);
			sq_pushstring(vm, "wakeup", -1);
			sq_get(vm, -2);	// Get the "wakeup" function from the component
			objNoCare::pushResult(vm, c);
			sq_call(vm, 1, SQFalse, SQTrue);
			sq_pop(vm, 2);	// Pops the component and the function
		}
	} while(queueNode);

	return 1;
}

SCRIPT_CLASS_REGISTER_NAME(ComponentQueue, "ComponentQueue")
	.constructor()
	.method("setItem", &ComponentQueue::setItem)
	.rawMethod("update", &componentQueueUpdate)
;}

}	// namespace script

void MCD::registerComponentQueueBinding(script::VMCore* v)
{
	script::ClassTraits<ComponentQueue>::bind(v);
}
