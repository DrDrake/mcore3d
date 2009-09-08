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

TEST(DeadObjectRemoval_ComponentQueueTest)
{
	ComponentQueue queue;
	const size_t cCount = 10;
	std::auto_ptr<MockComponent> c[cCount];

	for(size_t i=0; i<cCount; ++i) {
		c[i].reset(new MockComponent());
		queue.setItem(1, *c[i]);
	}

	{	// A normal loop should invoke all components
		const ComponentQueue::QueueNode* node = nullptr;
		size_t count = 0;
		while(queue.getItem(2, node)) {
			++count;
			if(!node) break;
		}
		CHECK_EQUAL(cCount, count);
	}

	{	// Delete some components and the remaining should still get invoked.
		c[1].reset();
		c[3].reset();
		c[4].reset();
		c[8].reset();

		const ComponentQueue::QueueNode* node = nullptr;
		size_t count = 0;
		MockComponent* p = nullptr;
		while((p = static_cast<MockComponent*>(queue.getItem(2, node))) != nullptr) {
			++count;
			if(!node) break;
		}
		CHECK_EQUAL(6u, count);
	}

	{	// Delete some components and only the remaining component with correct timing will be invoked.

		for(size_t i=0; i<cCount; ++i) {
			c[i].reset(new MockComponent());
			queue.setItem(float(i), *c[i]);
		}

		c[3].reset();
		const ComponentQueue::QueueNode* node = nullptr;
		size_t count = 0;
		MockComponent* p = nullptr;
		while((p = static_cast<MockComponent*>(queue.getItem(2, node))) != nullptr) {
			++count;
			if(!node) break;
		}
		CHECK_EQUAL(3u, count);

		count = 0;
		node = nullptr;
		while((p = static_cast<MockComponent*>(queue.getItem(3, node))) != nullptr) {
			++count;
			if(!node) break;
		}
		CHECK_EQUAL(3u, count);

		count = 0;
		node = nullptr;
		while((p = static_cast<MockComponent*>(queue.getItem(4, node))) != nullptr) {
			++count;
			if(!node) break;
		}
		CHECK_EQUAL(4u, count);
	}
}
