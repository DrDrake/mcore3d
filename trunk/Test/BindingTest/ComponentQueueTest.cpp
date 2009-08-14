#include "Pch.h"
#include "../../MCD/Binding/ComponentQueue.h"
#include "../../MCD/Core/Entity/Component.h"

using namespace MCD;

namespace {

class MockComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(MockComponent);
	}
};	// MockComponent

}	// namespace

TEST(ComponentQueueTest)
{
	{	// Create empty
		ComponentQueue queue;
	}

	{	// Insert an item and destroy (without popping)
		ComponentQueue queue;
		std::auto_ptr<Component> c(new MockComponent());
		queue.setItem(0, *c);
	}

	{	// Insert some items and pop
		ComponentQueue queue;
		std::auto_ptr<Component> c1(new MockComponent());
		std::auto_ptr<Component> c2(new MockComponent());
		queue.setItem(1, *c1);
		queue.setItem(2, *c2);

		const ComponentQueue::QueueNode* node = nullptr;
		CHECK(!queue.getItem(0, node));
		CHECK(node);	// Event the queue returns null, but it keeps return the next queue node

		node = nullptr;
		CHECK_EQUAL(c1.get(), queue.getItem(1, node));
		CHECK(!queue.getItem(1, node));
		CHECK(!node);

		node = nullptr;
		CHECK_EQUAL(c1.get(), queue.getItem(2, node));
		CHECK_EQUAL(c2.get(), queue.getItem(2, node));
		CHECK(!node);
	}

	{	// Reset the wake up time of an existing component
		ComponentQueue queue;
		std::auto_ptr<Component> c(new MockComponent());
		queue.setItem(1, *c);
		queue.setItem(2, *c);

		const ComponentQueue::QueueNode* node = nullptr;
		CHECK(!queue.getItem(1, node));
		CHECK(!node);

		node = nullptr;
		CHECK_EQUAL(c.get(), queue.getItem(2, node));
		CHECK(!node);
	}

	{	// Destroy an already inserted component
		ComponentQueue queue;
		std::auto_ptr<Component> c(new MockComponent());
		queue.setItem(1, *c);
		c.reset();

		const ComponentQueue::QueueNode* node = nullptr;
		CHECK(!queue.getItem(2, node));
		CHECK(!node);
	}
}
