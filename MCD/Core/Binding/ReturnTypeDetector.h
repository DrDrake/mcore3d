#ifndef __MCD_CORE_BINDING_RETURTYPEDETECTOR__
#define __MCD_CORE_BINDING_RETURTYPEDETECTOR__

namespace MCD {
namespace Binding {

// Remove * and & for type T
template<typename T> struct RmPtr { typedef T RET; };
template<typename T> struct RmPtr<T*> { typedef T RET; };
template<typename T> struct RmPtr<T&> { typedef T RET; };

struct MemberField {};
struct MemberFunc {};
struct StaticFunc {};
struct RawSqFunc {};

// Template traits to detect the various compile-time info of a function
template<typename Func>
struct FuncTraits {
};

// Raw squirrel function
template<>
struct FuncTraits<int (*)(HSQUIRRELVM)> {
	typedef int RET;
	typedef HSQUIRRELVM FirstParam;
	typedef RawSqFunc FuncType;
	enum { ParamCount = 0 };
};

// Static function
template<typename RT>
struct FuncTraits<RT (*)()> {
	typedef typename RmPtr<RT>::RET RET;
	typedef void FirstParam;
	typedef StaticFunc FuncType;
	enum { ParamCount = 0 };
};

template<typename RT, typename P1>
struct FuncTraits<RT (*)(P1)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef StaticFunc FuncType;
	enum { ParamCount = 1 };
};

template<typename RT, typename P1,typename P2>
struct FuncTraits<RT (*)(P1,P2)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef StaticFunc FuncType;
	enum { ParamCount = 2 };
};

template<typename RT, typename P1,typename P2,typename P3>
struct FuncTraits<RT (*)(P1,P2,P3)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef StaticFunc FuncType;
	enum { ParamCount = 3 };
};

template<typename RT, typename P1,typename P2,typename P3,typename P4>
struct FuncTraits<RT (*)(P1,P2,P3,P4)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef StaticFunc FuncType;
	enum { ParamCount = 4 };
};

template<typename RT, typename P1,typename P2,typename P3,typename P4,typename P5>
struct FuncTraits<RT (*)(P1,P2,P3,P4,P5)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef StaticFunc FuncType;
	enum { ParamCount = 5 };
};

template<typename RT, typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
struct FuncTraits<RT (*)(P1,P2,P3,P4,P5,P6)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef StaticFunc FuncType;
	enum { ParamCount = 6 };
};

template<typename RT, typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
struct FuncTraits<RT (*)(P1,P2,P3,P4,P5,P6,P7)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef StaticFunc FuncType;
	enum { ParamCount = 7 };
};

// Memember field
template<class Callee, typename T>
struct FuncTraits<T (Callee::*)> {
	typedef T RET;
	typedef void FirstParam;
	typedef MemberField FuncType;
	enum { ParamCount = 0 };
};

// Member function
template<class Callee, typename RT>
struct FuncTraits<RT (Callee::*)()> {
	typedef typename RmPtr<RT>::RET RET;
	typedef void FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 0 };
};

template<class Callee, typename RT, typename P1>
struct FuncTraits<RT (Callee::*)(P1)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 1 };
};

template<class Callee, typename RT, typename P1,typename P2>
struct FuncTraits<RT (Callee::*)(P1,P2)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 2 };
};

template<class Callee, typename RT, typename P1,typename P2,typename P3>
struct FuncTraits<RT (Callee::*)(P1,P2,P3)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 3 };
};

template<class Callee, typename RT, typename P1,typename P2,typename P3,typename P4>
struct FuncTraits<RT (Callee::*)(P1,P2,P3,P4)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 4 };
};

template<class Callee, typename RT, typename P1,typename P2,typename P3,typename P4,typename P5>
struct FuncTraits<RT (Callee::*)(P1,P2,P3,P4,P5)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 5 };
};

template<class Callee, typename RT, typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
struct FuncTraits<RT (Callee::*)(P1,P2,P3,P4,P5,P6)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 6 };
};

template<class Callee, typename RT, typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
struct FuncTraits<RT (Callee::*)(P1,P2,P3,P4,P5,P6,P7)> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 7 };
};

// Const member function
template<class Callee, typename RT>
struct FuncTraits<RT (Callee::*)() const> {
	typedef typename RmPtr<RT>::RET RET;
	typedef void FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 0 };
};

template<class Callee, typename RT, typename P1>
struct FuncTraits<RT (Callee::*)(P1) const> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 1 };
};

template<class Callee, typename RT, typename P1,typename P2>
struct FuncTraits<RT (Callee::*)(P1,P2) const> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 2 };
};

template<class Callee, typename RT, typename P1,typename P2,typename P3>
struct FuncTraits<RT (Callee::*)(P1,P2,P3) const> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 3 };
};

template<class Callee, typename RT, typename P1,typename P2,typename P3,typename P4>
struct FuncTraits<RT (Callee::*)(P1,P2,P3,P4) const> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 4 };
};

template<class Callee, typename RT, typename P1,typename P2,typename P3,typename P4,typename P5>
struct FuncTraits<RT (Callee::*)(P1,P2,P3,P4,P5) const> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 5 };
};

template<class Callee, typename RT, typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
struct FuncTraits<RT (Callee::*)(P1,P2,P3,P4,P5,P6) const> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 6 };
};

template<class Callee, typename RT, typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
struct FuncTraits<RT (Callee::*)(P1,P2,P3,P4,P5,P6,P7) const> {
	typedef typename RmPtr<RT>::RET RET;
	typedef P1 FirstParam;
	typedef MemberFunc FuncType;
	enum { ParamCount = 7 };
};

}   //namespace Binding
}   //namespace MCD

#endif	// __MCD_CORE_BINDING_RETURTYPEDETECTOR__
