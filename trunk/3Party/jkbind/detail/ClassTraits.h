#ifndef ___SCRIPT_CLASS_TRAIS_CONTROL__
#define ___SCRIPT_CLASS_TRAIS_CONTROL__

namespace script {

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

		static inline ClassID classID()
		{
			return &_dummyField;
		}

	private:
/*		static void _dummy()
		{	//just dummy - to have unique class id
			Class* ptr = 0;
			delete ptr;
		}*/

		static unsigned long _dummyField;
	};

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