#ifndef ___BASE_BASE_H___
#define ___BASE_BASE_H___

#include "Types.h"
#include <assert.h>

namespace base
{
//
// Forwards
//
namespace detail
{
	class PtrHelper;
	class WeakPtrImpl;
}
template<class TYPE> class Ptr;
template<class TYPE> class WeakPtr;


///
/// Base Object class - used for reference counting and automatic object deletion.
/// Virtual inheritance is recommended.
///
class Object
{
public:
	Object(): _referenceCounter(0), _weakPtr(0) {}
	JKBIND_API virtual ~Object();

	inline int getReferenceCounter() const { return _referenceCounter; }

private:
	sint _referenceCounter;
	detail::WeakPtrImpl* _weakPtr;

	//friends
	friend class detail::PtrHelper;
	friend class detail::WeakPtrImpl;
};	// Object

namespace detail //detail namespace - to avoid compiler bugs
{
	class WeakPtrImpl
	{
	public:
		WeakPtrImpl(): _next(0), _prev(0) {}
		WeakPtrImpl(Object* object)
		{
			attach(object);
		}

		~WeakPtrImpl()
		{
			assert(_next == 0);
			assert(_prev == 0);
		}

	protected:
		WeakPtrImpl* _next;
		WeakPtrImpl* _prev;

		JKBIND_API void detach(Object* object);
		JKBIND_API void attach(Object* object);

	//friends
		friend class ::base::Object;
	};	// WeakPtrImpl

	class PtrHelper
	{
	public:
		static void incrementReferenceCount(Object* object)
		{
			object->_referenceCounter++;
		}

		static void decrementReferenceCount(Object* object)
		{
			assert(object->_referenceCounter > 0);
			object->_referenceCounter--;
			if (object->_referenceCounter == 0) delete object;
		}
	};	// PtrHelper
}	// namespace detail

///
/// WeakPtr class - weak pointer for Object
///
template<class TYPE> class WeakPtr : private detail::WeakPtrImpl
{
public:
	typedef TYPE Type;

	WeakPtr(): _object(0) {}

	WeakPtr(Type *object)
		: detail::WeakPtrImpl(static_cast<Object*>(object))
	{
		_object = object;
	}

	template<typename OtherType>
	WeakPtr(const WeakPtr<OtherType>& ptr)
		: detail::WeakPtrImpl(static_cast<Object*>(ptr.pointer()))
	{
		_object = ptr.pointer();
	}

	template<typename OtherType>
	WeakPtr(const Ptr<OtherType>& ptr)
		: detail::WeakPtrImpl(static_cast<Object*>(ptr.pointer()))
	{
		_object = ptr.pointer();
	}

	WeakPtr(const WeakPtr& ptr)
		: detail::WeakPtrImpl(static_cast<Object*>(ptr.pointer()))
	{
		_object = ptr.pointer();
	}

	~WeakPtr() { detach(pointer()); }

	inline WeakPtr& operator=(Type* object)
	{
		reassign(object);
		return *this;
	}

	inline WeakPtr& operator=(const Ptr<Type>& ptr)
	{
		reassign(ptr.pointer());
		return *this;
	}

	inline WeakPtr& operator=(const WeakPtr& ptr)
	{
		reassign(ptr.pointer());
		return *this;
	}

	inline Type* operator->() const
	{
		return pointer();
	}

	inline Type& operator*() const
	{
		return *pointer();
	}

	inline Type* pointer() const
	{
		if(_next) {
			//linked - so object exists
			assert(_prev != 0);
			return _object;
		}
		return 0;
	}

	inline bool isNull() const
	{
		return _next == 0;
	}

	inline bool operator == (const WeakPtr& other) const
	{
		return pointer() == other.pointer();
	}

	inline bool operator == (const Type* other) const
	{
		return pointer() == other;
	}

	inline bool operator != (const WeakPtr& other) const
	{
		return pointer() != other.pointer();
	}

	inline bool operator != (const Type* other) const
	{
		return pointer() != other;
	}

private:
	void reassign(Type* newValue)
	{
		detach(pointer());
		attach(newValue);
		_object = newValue;
	}

	Type* _object;
};	// WeakPtr

///
/// Ptr class - smart pointer with reference counting support
///
template<class TYPE> class Ptr
{
public:
	typedef TYPE Type;

private:
	Type* _object;

public:
	Ptr() : _object(0)
	{}

	Ptr(Type *object) : _object(object)
	{
		if (_object != 0)
		{
			detail::PtrHelper::incrementReferenceCount(_object);
		}
	}

	template<typename OtherType>
		Ptr(const Ptr<OtherType>& ptr) : _object(static_cast<Type*>(ptr.pointer()))
	{
		if (_object != 0)
		{
			detail::PtrHelper::incrementReferenceCount(_object);
		}
	}

	template<typename OtherType>
		Ptr(const WeakPtr<OtherType>& ptr) : _object(static_cast<Type*>(ptr.pointer()))
	{
		if (_object != 0)
		{
			detail::PtrHelper::incrementReferenceCount(_object);
		}
	}

	Ptr(const Ptr& ptr) : _object(ptr._object)
	{
		if (_object != 0)
		{
			detail::PtrHelper::incrementReferenceCount(_object);
		}
	}

	~Ptr()
	{
		if (_object != 0)
		{
			detail::PtrHelper::decrementReferenceCount(_object);
		}
	}

	inline Ptr& operator=(Type* object)
	{
		if(_object != object)
		{
			if (object != 0)
			{
				detail::PtrHelper::incrementReferenceCount(object);
			}
			if (_object != 0)
			{
				detail::PtrHelper::decrementReferenceCount(_object);
			}
			_object = object;
		}

		return *this;
	}

	inline Ptr& operator=(const Ptr& ptr)
	{
		if(_object != ptr._object)
		{
			if (ptr._object != 0)
			{
				detail::PtrHelper::incrementReferenceCount(ptr._object);
			}
			if (_object != 0)
			{
				detail::PtrHelper::decrementReferenceCount(_object);
			}
			_object = ptr._object;
		}
		return *this;
	}

	inline Type* operator->() const
	{
		return _object;
	}

	inline Type& operator*() const
	{
		return *_object;
	}

	inline Type* pointer() const
	{
		return _object;
	}

	inline bool isNull() const
	{
		return _object == 0;
	}

	inline bool operator == (const Ptr& other) const
	{
		return _object == other._object;
	}

	inline bool operator == (const Type* other) const
	{
		return pointer() == other;
	}

	inline bool operator != (const Ptr& other) const
	{
		return _object != other._object;
	}

	inline bool operator != (const Type* other) const
	{
		return pointer() != other;
	}
};	// Ptr

}	// namespace base

using base::Ptr;
using base::WeakPtr;

#endif  //___BASE_BASE_H___