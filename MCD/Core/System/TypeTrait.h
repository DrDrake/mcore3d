#ifndef __MCD_CORE_SYSTEM_TYPETRAIT__
#define __MCD_CORE_SYSTEM_TYPETRAIT__

#include "Platform.h"

namespace MCD {

template<typename T> struct ParamType {
	typedef T& RET;
};

template<typename T> struct ParamType<T&> {
	typedef T& RET;
};

template<typename T> struct ParamType<const T&> {
	typedef const T& RET;
};

template<typename T> struct ParamType<T*> {
	typedef T* RET;
};

template<typename T> struct ParamType<const T*> {
	typedef const T* RET;
};

template<> struct ParamType<int> {
	typedef int RET;
};

template<> struct ParamType<const int> {
	typedef const int RET;
};

template<> struct ParamType<size_t> {
	typedef size_t RET;
};

template<> struct ParamType<const size_t> {
	typedef const size_t RET;
};

template<> struct ParamType<float> {
	typedef float RET;
};

template<> struct ParamType<const float> {
	typedef const float RET;
};

template<> struct ParamType<double> {
	typedef double RET;
};

template<> struct ParamType<const double> {
	typedef const double RET;
};

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_TYPETRAIT__
