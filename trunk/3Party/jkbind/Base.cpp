#include "Base.h"

namespace base {

void detail::WeakPtrImpl::detach(Object* object)
{
	if (object) {
		//target exists
		assert(_next);
		assert(_prev);
#ifdef _MSC_VER
		__assume(_next);
		__assume(_prev);
#endif
		if(_next == this) {
			//we are the only weakref
			assert(_next == _prev);
			object->_weakPtr = 0;
		} else {
			//we are in the list - unlinking
			if(object->_weakPtr == this) {
				object->_weakPtr = _next;
			}

			_next->_prev = _prev;
			_prev->_next = _next;
		}
	} else {
		//no target - clearing self
		assert(_next == 0);
		assert(_prev == 0);
	}

	_next = 0;
	_prev = 0;
}

void detail::WeakPtrImpl::attach(Object* object)
{
	if (object) {
		//target exists
		if(object->_weakPtr) {
			//some other weaks exists, linking self
			_next = object->_weakPtr->_next;
			_prev = object->_weakPtr;
			object->_weakPtr->_next->_prev = this;
			object->_weakPtr->_next = this;
		} else {
			//first weakref
			object->_weakPtr = this;
			_next = this;
			_prev = this;
		}
	} else {
		//no target - clearing self
		_next = 0;
		_prev = 0;
	}
}

Object::~Object()
{
	assert(_referenceCounter == 0);

	if(_weakPtr) {
		detail::WeakPtrImpl* slider = _weakPtr;
		do {
			detail::WeakPtrImpl* next = slider->_next;
			slider->_next = 0;
			slider->_prev = 0;
			slider = next;
		} while(slider != _weakPtr);
	}
}

}	//namespace base
