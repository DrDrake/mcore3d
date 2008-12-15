#ifndef __MCD_CORE_SYSTEM_PTRVECTOR__
#define __MCD_CORE_SYSTEM_PTRVECTOR__

#include "Platform.h"
#include <vector>

namespace MCD {

/*!	A std::vector like class that is specialized to store object pointer.
	The interface of this class is slightly different to std::vector,
	it stores pointer to objects but the way you retieve the object is by
	reference rather than pointer.

	Example:
	\code
	class Foo {
	public:
		void myFunction();
	};
	typedef ptr_vector<Foo> FooList;
	FooList fooList;
	fooList.push_back(new Foo);
	fooList.push_back(new Foo);
	fooList[0].myFunction();
	// The Foo objects get deleted when FooList is out of scope
	\endcode
 */
template<typename T, class A = std::allocator<T*>, bool DeleteObject=true>
class ptr_vector : protected std::vector<T*, A>
{
	typedef std::vector<T*, A> super_type;

public:
	// Adjust the types from pointer to value
	typedef T value_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef const pointer const_pointer;
	typedef const value_type& const_reference;

	// Types that no adjustment is needed
	typedef typename super_type::size_type size_type;
	typedef typename super_type::allocator_type allocator_type;
	typedef typename super_type::difference_type difference_type;

	class iterator : public super_type::iterator
	{
		typedef typename super_type::iterator super_;

	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef value_type& reference;

		iterator(const super_& s) : super_(s) {}
		reference operator*() const { return *super_::operator*(); }
		pointer operator->() const { return (&**this); }
	};	// iterator

	class const_iterator : public super_type::const_iterator
	{
		typedef typename super_type::const_iterator super_;

	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef value_type& reference;

		const_iterator(const super_& s) : super_(s) {}
		reference operator*() const { return *super_::operator*(); }
		pointer operator->() const { return (&**this); }
	};	// const_iterator

	~ptr_vector()
	{
		if(DeleteObject) for(typename super_type::iterator i=this->begin(); i!=this->end(); ++i)
			delete (*i);
	}

// Accessors
	iterator begin() { return super_type::begin(); }
	const_iterator begin() const { return super_type::begin(); }

	iterator end() { return super_type::end(); }
	const_iterator end() const { return super_type::end(); }

	reference at(size_type pos) { return *super_type::at(pos); }
	const_reference at(size_type pos) const { return *super_type::at(pos); }

	reference operator[](size_type pos) { return *super_type::operator[](pos); }
	const_reference operator[](size_type pos) const { return *super_type::operator[](pos); }

	reference front() { return *super_type::front(); }
	const_reference front() const { return *super_type::front(); }

	reference back() { return *super_type::back(); }
	const_reference back() const { return *super_type::back(); }

	size_type capacity() const { return super_type::capacity(); }

	bool empty() const { return super_type::empty(); }

	size_type size() const { return super_type::size(); }

// Modifiers
	void reserve(size_type count) { super_type::reserve(count); }

	iterator insert(iterator where, pointer ptr)
	{
		assertUnique(ptr);
		return super_type::insert(where, ptr);
	}

	iterator erase(iterator where)
	{
		if(DeleteObject)
			delete &(*where);
		return super_type::erase(where);
	}

	//! Make sure ptr is not already in the vector.
	void push_back(pointer ptr)
	{
		assertUnique(ptr);
		super_type::push_back(ptr);
	}

	//!	If ptr is already in the vector, it will move to the back without adding another entry.
	void re_push_back(pointer ptr)
	{
		typename super_type::iterator i;
		for(i=this->begin(); i!=this->end(); ++i)
			if(*i == ptr) break;
		if(i != super_type::end())
			super_type::erase(i);
		super_type::push_back(ptr);
	}

	//! If DeleteObject=false, return the pointer after pop out, null otherwise.
	pointer pop_back()
	{
		pointer p = super_type::back();
		if(DeleteObject)
			delete super_type::back();
		super_type::pop_back();
		return DeleteObject ? NULL : p;
	}

private:
	//! Re-size is not supported
	void resize(size_type newSize);

	void assertUnique(pointer ptr)
	{
		if(DeleteObject) for(typename super_type::iterator i=this->begin(); i!=this->end(); ++i)
			assert(*i != ptr);
	}
};	// ptr_vector

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_PTRVECTOR__
