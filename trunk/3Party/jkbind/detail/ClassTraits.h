#ifndef ___SCRIPT_CLASS_TRAIS_CONTROL__
#define ___SCRIPT_CLASS_TRAIS_CONTROL__

typedef struct SQVM* HSQUIRRELVM;

namespace script {

class VMCore;
typedef void* ClassID;

///
/// Common class traits
///
template<typename T>
class ClassTraits;

namespace detail
{

template<typename T>
class ClassTraits
{
public:
	typedef T Class;

	static inline ClassID classID() {
		return &_dummyField;
	}

private:
	static unsigned long _dummyField;
};	// ClassTraits

template<typename T>
unsigned long ClassTraits<T>::_dummyField = 0;

}   //namespace detail


#define SCRIPT_CLASS_DECLAR(Class)				\
template<>										\
class ClassTraits<Class> :						\
	public script::detail::ClassTraits<Class> {	\
public:											\
	static void bind(VMCore* vm);				\
};

#define SCRIPT_CLASS_DECLAR_EXPORT(Class, API)	\
template<>										\
class API ClassTraits<Class> :					\
	public script::detail::ClassTraits<Class> {	\
public:											\
	static void bind(VMCore* vm);				\
};

#define SCRIPT_CLASS_REGISTER(Class)			\
void ClassTraits<Class>::bind(VMCore* vm) {		\
	RootDeclarator root(vm);					\
	root

#define SCRIPT_CLASS_REGISTER_NAME(Class, name)	\
void ClassTraits<Class>::bind(VMCore* vm) {		\
	RootDeclarator root(vm);					\
	root.declareClass<Class>(xSTRING(name))

}   //namespace script

#endif//___SCRIPT_CLASS_TRAIS_CONTROL__
