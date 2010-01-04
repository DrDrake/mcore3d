#ifndef ___SCRIPT_DETAIL_FIELDS___
#define ___SCRIPT_DETAIL_FIELDS___

namespace script {
namespace detail {

template<typename ResultPolicy, typename Callee, typename RawField>
inline int PushField(Callee* callee, RawField (Callee::*fieldPtr), HSQUIRRELVM v)
{
	if(!callee)
		return sq_throwerror(v, xSTRING("This pointer is null"));

	typedef types::GetterSetter<RawField> GetterSetter;
	return ResultPolicy::template pushResult(v, GetterSetter::get(callee->*fieldPtr));
}

template<typename Callee, typename Field, typename ReturnPolicy>
SQInteger fieldGetterFunction(HSQUIRRELVM v)
{
	// getting "this"
	StackHandler sa(v);
	Callee* instance(0);
	instance = (Callee*)sa.getInstanceUp(1, 0);
	int paramCount = sa.getParamCount();

	Field field = getFieldPointer<Field>(v, paramCount);

	return PushField<ReturnPolicy, Callee>(instance, field, v);
}

template<typename Callee, typename RawField>
inline int AssignField(Callee* callee, RawField (Callee::*fieldPtr), HSQUIRRELVM v, int index)
{
	if(!callee)
		return sq_throwerror(v, xSTRING("This pointer is null"));

	typedef typename types::GetterSetter<RawField>::setterType fieldType;
#if jkDEBUG_SCRIPT
	if (!match(types::TypeSelect<fieldType>(), v,index))
		return sq_throwerror(v, xSTRING("Incorrect field setter argument"));
#endif//jkDEBUG_SCRIPT

	callee->*fieldPtr = get(types::TypeSelect<fieldType>(), v, index);
	return 0;
}

template<typename Callee, typename Field >
SQInteger fieldSetterFunction(HSQUIRRELVM v)
{
	//getting "this"
	StackHandler sa(v);
	Callee* instance(0);
	instance = (Callee*)sa.getInstanceUp(1, 0);
	int paramCount = sa.getParamCount();
	Field field = getFieldPointer<Field>(v, paramCount);
	return AssignField<Callee>(instance, field, v, 2);
}

}	//namespace detail
}	//namespace script

#endif//___SCRIPT_DETAIL_FIELDS___
