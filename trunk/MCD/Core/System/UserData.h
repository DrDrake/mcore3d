#ifndef __MCD_CORE_SYSTEM_USERDATA__
#define __MCD_CORE_SYSTEM_USERDATA__

#include "Platform.h"

namespace MCD {

/*	A class that store user data in a type-safe manner.
	It will take the ownership of the object.

	Example:
	\code
	UserData data;
	// It is suggested to specify the type explicitly: setPtr<int>
	data.setPtr<int>(new int(123));

	// The getPtr will perform type checking, and return null if the
	// type is not matched.
	CHECK(data.getPtr<float>() == nullptr);

	int* p = data.getPtr<int>();
	if(p)
		CHECK(*p == 123);
	\endcode
 */
class UserData
{
public:
	UserData()
	{
		new(holder()) Holder<Dummy>(nullptr);
		setHolder(holder());
	}

	~UserData()
	{
		holder()->~IHolder();
	}

	template<typename T>
	void setPtr(sal_maybenull T* p)
	{
		holder()->~IHolder();
		new(holder()) Holder<T>(p);
	}

	template<typename T>
	sal_maybenull T* getPtr() {
		Holder<T>* h = dynamic_cast<Holder<T>*>(holder());
		if(!h)
			return nullptr;
		return h->ptr.get();
	}

protected:
	MCD_ABSTRACT_CLASS struct IHolder {
		virtual ~IHolder() {}
	};
	template<typename T>
	struct Holder : IHolder {
		Holder(T* p) : ptr(p) {}
		std::auto_ptr<T> ptr;
	};

	IHolder* holder() {
		return reinterpret_cast<IHolder*>(&mBuffer);
	}

#ifndef NDEBUG
	void setHolder(IHolder* h) { mHolder = h; }
	// For debugging purpose so that more information can be displayed in debugger watch window.
	IHolder* mHolder;
#else
	void setHolder(IHolder* h) {}
#endif

	struct Dummy {};
	// Since the size of ANY Holder<> structure are the same, we can use
	// a pre-allocated buffer to store it, saving us a dynamic allocation
	// for each instance of UserData.
	char mBuffer[sizeof(Holder<Dummy>)];
};	// UserData

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_USERDATA__
