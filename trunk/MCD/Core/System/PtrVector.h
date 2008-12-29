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

	template<typename T_, class I>
	class iterator_template : public std::iterator<std::random_access_iterator_tag, T_>
	{
		friend class ptr_vector;
		typedef typename std::iterator<std::random_access_iterator_tag, T_> super_;

	public:
		typedef typename super_::pointer pointer;
		typedef typename super_::reference reference;
		typedef typename super_::value_type value_type;
		typedef typename super_::difference_type difference_type;
		typedef typename super_::iterator_category iterator_category;

		iterator_template(const I& i) : it(i) {}
		template<typename T2, class I2>	// Template constructor allowing conversion from iterator to const_iterator
		iterator_template(const iterator_template<T2, I2>& i) : it(i.it) {}
		reference operator*() const { return **it; }
		pointer operator->() const { return *it; }
		iterator_template& operator++() { ++it; return *this; }
		iterator_template& operator++(int) { it++; return *this; }
		iterator_template& operator--() { --it; return *this; }
		iterator_template& operator--(int) { it++; return *this; }
		iterator_template& operator+=(difference_type offset) { it += offset; return *this; }
		iterator_template& operator-=(difference_type offset) { it -= offset; return *this; }
		iterator_template operator+(difference_type offset) const { return it + offset; }
		iterator_template operator-(difference_type offset) const { return it - offset; }
		difference_type operator-(const iterator_template& rhs) const { return it - rhs.it; }
		bool operator==(const iterator_template& rhs) const { return it == rhs.it; }
		bool operator!=(const iterator_template& rhs) const { return it != rhs.it; }
		bool operator< (const iterator_template& rhs) const { return it <  rhs.it; }
		bool operator> (const iterator_template& rhs) const { return it >  rhs.it; }
		bool operator<=(const iterator_template& rhs) const { return it <= rhs.it; }
		bool operator>=(const iterator_template& rhs) const { return it >= rhs.it; }

	protected:
		I it;	//!< The actual iterator
	};	// iterator_template

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

	typedef iterator_template<T, typename super_type::iterator> iterator;
	typedef iterator_template<const T, typename super_type::const_iterator> const_iterator;
	typedef iterator_template<T, typename super_type::reverse_iterator> reverse_iterator;
	typedef iterator_template<const T, typename super_type::const_reverse_iterator> const_reverse_iterator;

	~ptr_vector() { clear(); }

// Accessors
	iterator begin() { return super_type::begin(); }
	const_iterator begin() const { return super_type::begin(); }

	iterator end() { return super_type::end(); }
	const_iterator end() const { return super_type::end(); }

	reverse_iterator rbegin() { return super_type::rbegin(); }
	const_reverse_iterator rbegin() const { return super_type::rbegin(); }

	reverse_iterator rend() { return super_type::rend(); }
	const_reverse_iterator rend() const { return super_type::rend(); }

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
		return super_type::insert(where.it, ptr);
	}

	iterator erase(iterator where, bool deleteObject=true)
	{
		if(DeleteObject && deleteObject)
			delete &(*where);
		return super_type::erase(where.it);
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
		for(i=super_type::begin(); i!=super_type::end(); ++i)
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

	void clear(bool deleteObject=true)
	{
		if(DeleteObject && deleteObject)
			for(typename super_type::iterator i=super_type::begin(); i!=super_type::end(); ++i)
				delete (*i);
		super_type::clear();
	}

private:
	//! Re-size is not supported
	void resize(size_type newSize);

	void assertUnique(pointer ptr)
	{
#ifndef NDEBUG
		if(DeleteObject) for(typename super_type::iterator i=super_type::begin(); i!=super_type::end(); ++i)
			assert(*i != ptr);
#endif
	}
};	// ptr_vector

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_PTRVECTOR__
