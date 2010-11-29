#ifndef __MCD_CORE_BINDING_CLASSTRAITS__
#define __MCD_CORE_BINDING_CLASSTRAITS__

typedef struct SQVM* HSQUIRRELVM;

namespace MCD {
namespace Binding {

class VMCore;
typedef void* ClassID;

/// Common class traits
template<typename T> struct ClassTraits;

namespace Detail {

template<typename T> struct ClassTraits
{
	typedef T Class;
	static ClassID classID() { return &_dummyField; }

private:
	static size_t _dummyField;
};	// ClassTraits

template<typename T> size_t ClassTraits<T>::_dummyField = 0;

}	// namespace Detail

#define SCRIPT_CLASS_DECLAR(Class)				\
template<> struct ClassTraits<Class> :			\
	public ::MCD::Binding::Detail::ClassTraits<Class> {	\
	static void bind(VMCore* vm);				\
};

#define SCRIPT_CLASS_DECLAR_EXPORT(Class, API)	\
template<> struct API ClassTraits<Class> :		\
	public ::MCD::Binding::Detail::ClassTraits<Class> {	\
	static void bind(VMCore* vm);				\
};

#define SCRIPT_CLASS_REGISTER(Class)			\
void ClassTraits<Class>::bind(VMCore* vm) {		\
	RootDeclarator root(vm);					\
	root

#define SCRIPT_CLASS_REGISTER_NAME(Class)		\
void ClassTraits<Class>::bind(VMCore* vm) {		\
	RootDeclarator root(vm);					\
	root.declareClass<Class>(#Class)

}	// namespace Binding
}   // namespace MCD

#endif	// __MCD_CORE_BINDING_CLASSTRAITS__
