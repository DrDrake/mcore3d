#include "Pch.h"
#include "../../../MCD/Core/System/IntrusivePtr.h"

using namespace MCD;

namespace {

typedef IntrusivePtr<class Object> ObjectPtr;

class Foo
{
public:
	friend void intrusivePtrAddRef(Foo*) {}
	friend void intrusivePtrRelease(Foo*) {}
};

typedef IntrusivePtr<Foo> FooPtr;

// Should compile
void takeTObjectPtr(ObjectPtr& p)
{
	ObjectPtr a = p;
	a = nullptr;
	if(a == nullptr)
		p = a;
}

// Declaration of a IntrusivePtr with incomplete type should work
class Dummy {
	ObjectPtr p;
};

//! A basic Object for testing
class Object
{
protected:
	// Note that the destructor need not to be virtual if the derived
	// classes also have intrusivePtrRelease() function
	virtual ~Object() throw() {
		--mObjectCount;
	}

public:
	Object(const char* id = nullptr)
		: mRefCount(0), mID(id)
	{
		++mObjectCount;
	}

	const char* getId() const {
		return mID;
	}

	size_t getRefCount() const {
		return mRefCount;
	}

	static size_t getObjectCount() {
		return mObjectCount;
	}

	friend void intrusivePtrAddRef(sal_notnull Object* p) {
		++(p->mRefCount);
	}

	friend void intrusivePtrRelease(sal_notnull Object* p) {
		if(--(p->mRefCount) == 0)
			delete p;
	}

protected:
	size_t mRefCount;
	const char* mID;
	static size_t mObjectCount;
};	// Object

size_t Object::mObjectCount = 0;

//! Testing derived class.
class Derived : public Object
{
protected:
	~Derived() throw() {
		--mDerivedCount;
	}

public:
	Derived(const char* id = nullptr)
		: Object(id)
	{
		++mDerivedCount;
	}

	static size_t getObjectCount() {
		return mDerivedCount;
	}

private:
	static size_t mDerivedCount;
};	// Derived

size_t Derived::mDerivedCount = 0;

typedef IntrusivePtr<Derived> DerivedPtr;

}	// namespace

TEST(Basic_IntrusivePtrTest)
{
	CHECK_EQUAL(0u, Object::getObjectCount());
	{
		// Compiler error: cannot create local object.
		//Object error;
		ObjectPtr a(new Object);
		CHECK_EQUAL(1u, a->getRefCount());
		CHECK_EQUAL(1u, (*a).getRefCount());	// Test using operator*
		CHECK_EQUAL(1u, Object::getObjectCount());
		// Destruction of a
	}

	CHECK_EQUAL(0u, Object::getObjectCount());

	{	// Test for equality
		ObjectPtr a(new Object);
		CHECK(a);
		CHECK(!a == false);

		ObjectPtr b(a);
		CHECK(a == b);
		CHECK((a != b) == false);

		ObjectPtr c(new Object);
		CHECK(a != c);
		CHECK((a == c) == false);

		a = nullptr;
		CHECK(!a);
		CHECK(a == false);
	}

	{	// Swap
		ObjectPtr a(new Object);
		ObjectPtr b(new Object);

		Object* rawa = a.get();
		Object* rawb = b.get();

		swap(a, b);
		CHECK_EQUAL(rawb, a.get());
		CHECK_EQUAL(rawa, b.get());
	}
}

TEST(UpDownCast_IntrusivePtrTest)
{
	CHECK_EQUAL(0u, Object::getObjectCount());
	CHECK_EQUAL(0u, Derived::getObjectCount());
	{
		ObjectPtr a(new Derived);
		CHECK_EQUAL(1u, a->getRefCount());
		CHECK_EQUAL(1u, Object::getObjectCount());
		CHECK_EQUAL(1u, Derived::getObjectCount());

		DerivedPtr b(static_pointer_cast<Derived>(a));
		b = (dynamic_pointer_cast<Derived>(a));
		CHECK_EQUAL(2u, a->getRefCount());
		CHECK_EQUAL(1u, Object::getObjectCount());
		CHECK_EQUAL(1u, Derived::getObjectCount());

		// Will not compile since Foo is not related to Object
//		ObjectPtr f(new Foo);

		// Will not compile since it needs a downcast
//		DerivedPtr d(a);

		// Destruction of a
	}

	CHECK_EQUAL(0u, Object::getObjectCount());
	CHECK_EQUAL(0u, Derived::getObjectCount());
}
