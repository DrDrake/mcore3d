#ifndef ___SCRIPT_COMMON_POINTERS___
#define ___SCRIPT_COMMON_POINTERS___

///\file
///
/// classes, that used to convert different types to and from its' pointer
///

namespace script {
namespace ptr {

template<typename T>
class pointer
{
public:
	typedef T HostType;
	/*!	Invoked when a function returns an object but not a pointer/reference,
		and a new instance should be new on heap.
	 */
	static inline HostType* to(const T& src) {
		return new T(src);
	}
};

template<typename T>
class pointer<T*>
{
public:
	typedef T HostType;
	static inline HostType* to(T* src) { return src; }
};

template<typename T>
class pointer<T&>
{
public:
	typedef T HostType;
	static inline HostType* to(T& src) { return &src; }
};

template<typename T>
class pointer<const T*>
{
public:
	typedef T HostType;
	static inline HostType* to(T& src) { return const_cast<T*>(src); }
};

template<typename T>
class pointer<const T&>
{
public:
	typedef T HostType;
	static inline HostType* to(const T& src) { return const_cast<T*>(&src); }
};

}	//namespace ptr
}	//namespace script

#endif//___SCRIPT_COMMON_POINTERS___
