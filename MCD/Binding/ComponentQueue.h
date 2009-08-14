#ifndef __MCD_BINDING_COMPONENTQUEUE__
#define __MCD_BINDING_COMPONENTQUEUE__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"

namespace MCD {

class Component;

//! A data structure for component update scheduling.
class MCD_BINDING_API ComponentQueue : Noncopyable
{
public:
	ComponentQueue();

	~ComponentQueue();

	/*!	Insert a component to the queue, and can be pop out again using popItem()
		when the current time reach the wakeUpTime.

		The ComponentQueue only take weak reference to the component, so a schedued
		component take may be dropped because the component itself is deleted.
	 */
	void setItem(float wakeUpTime, Component& component);

	/*!	Internal data structure that implements the queue, is also use as an
		iteration context, see getItem().
	 */
	class QueueNode;

	/*!	The component with wakeUpTime >= currentTime will be returned.
		\param begin Start the search right from this parameter, if it's null, then
			the search will be performed at the very begining. At the same time, it
			will return the next node with wakeUpTime >= that of the current one.

		With the \em begin variable, you can iterate all components with wakeUpTime
		smaller than \em currentTime:
		\code
		float currentTime = 123.0f;
		const QueueNode* queueNode = nullptr;	// Act as an iteration context
		while(true) {
			Component* c = componentQueue.getItem(currentTime, queueNode);
			// queueNode == null means end of queue, c == null means no more component ought to be run.
			if(c && queueNode)
				// Do something with the component ...
			else
				break;
		}
		\endcode
	 */
	sal_maybenull Component* getItem(float currentTime, sal_inout_opt const QueueNode*& begin);

protected:
	class Impl;
	Impl* mImpl;
};	// ComponentQueue

}	// namespace MCD

#include "../../3Party/jkbind/Declarator.h"

namespace script {

SCRIPT_CLASS_DECLAR_EXPORT(MCD::ComponentQueue, MCD_BINDING_API);

}	// namespace script

#endif	// __MCD_BINDING_COMPONENTQUEUE__
