#ifndef __MCD_CORE_BINDING_CONSTRUCTORS__
#define __MCD_CORE_BINDING_CONSTRUCTORS__

namespace MCD {
namespace Binding {

template<class T>
T* constructionFunction() {
	return new T();
}

template<class T, class A1>
T* constructionFunction(A1 a1) {
	return new T(a1);
}

template<class T, class A1, class A2>
T* constructionFunction(A1 a1, A2 a2) {
	return new T(a1, a2);
}

template<class T, class A1, class A2, class A3>
T* constructionFunction(A1 a1, A2 a2, A3 a3) {
	return new T(a1, a2, a3);
}

template<class T, class A1, class A2, class A3, class A4>
T* constructionFunction(A1 a1, A2 a2, A3 a3, A4 a4) {
	return new T(a1, a2, a3, a4);
}

template<class T, T* F(const T*)>
T* cloneFunction(T* srcObj) {
	return F(srcObj);
}

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_CORE_BINDING_CONSTRUCTORS__
