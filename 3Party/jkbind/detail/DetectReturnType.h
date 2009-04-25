#ifndef ___SCRIPT_DETAIL_DETECTRETURNTYPE___
#define ___SCRIPT_DETAIL_DETECTRETURNTYPE___

namespace script {
namespace detail {

// Remove * and & for type T
template<typename T> struct RmPtr { typedef T RET; };
template<typename T> struct RmPtr<T*> { typedef T RET; };
template<typename T> struct RmPtr<T&> { typedef T RET; };

// Detect the return type of the function type Func
template<typename Func>
struct ReturnTypeDetector {
};

// Static function
template<typename RT>
struct ReturnTypeDetector<RT (*)()> {
	typedef typename RmPtr<RT>::RET RET;
};

template<typename RT, typename P1>
struct ReturnTypeDetector<RT (*)(P1)> {
	typedef typename RmPtr<RT>::RET RET;
};

template<typename RT, typename P1,typename P2>
struct ReturnTypeDetector<RT (*)(P1,P2)> {
	typedef typename RmPtr<RT>::RET RET;
};

template<typename RT, typename P1,typename P2,typename P3>
struct ReturnTypeDetector<RT (*)(P1,P2,P3)> {
	typedef typename RmPtr<RT>::RET RET;
};

// Member function
template<class Callee, typename RT>
struct ReturnTypeDetector<RT (Callee::*)()> {
	typedef typename RmPtr<RT>::RET RET;
};

template<class Callee, typename RT, typename P1>
struct ReturnTypeDetector<RT (Callee::*)(P1)> {
	typedef typename RmPtr<RT>::RET RET;
};

template<class Callee, typename RT, typename P1,typename P2>
struct ReturnTypeDetector<RT (Callee::*)(P1,P2)> {
	typedef typename RmPtr<RT>::RET RET;
};

template<class Callee, typename RT, typename P1,typename P2,typename P3>
struct ReturnTypeDetector<RT (Callee::*)(P1,P2,P3)> {
	typedef typename RmPtr<RT>::RET RET;
};

// Const member function
template<class Callee, typename RT>
struct ReturnTypeDetector<RT (Callee::*)() const> {
	typedef typename RmPtr<RT>::RET RET;
};

template<class Callee, typename RT, typename P1>
struct ReturnTypeDetector<RT (Callee::*)(P1) const> {
	typedef typename RmPtr<RT>::RET RET;
};

template<class Callee, typename RT, typename P1,typename P2>
struct ReturnTypeDetector<RT (Callee::*)(P1,P2) const> {
	typedef typename RmPtr<RT>::RET RET;
};

template<class Callee, typename RT, typename P1,typename P2,typename P3>
struct ReturnTypeDetector<RT (Callee::*)(P1,P2,P3) const> {
	typedef typename RmPtr<RT>::RET RET;
};

}   //namespace detail
}   //namespace script

#endif//___SCRIPT_DETAIL_DETECTRETURNTYPE___
