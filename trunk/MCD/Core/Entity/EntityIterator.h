#ifndef __MCD_CORE_ENTITY_ENTITYITERATOR__
#define __MCD_CORE_ENTITY_ENTITYITERATOR__

#include "../ShareLib.h"
#include "../System/Platform.h"

namespace MCD {

class Entity;

/*!	An iterator that preforms a pre-order traversal on the Entity tree.
	Example:
	\code
	Entity root;
	for(EntityPreorderIterator itr(&root); !itr.ended(); itr.next()) {
		// Do something ...
	}
	\endcode
 */
class MCD_CORE_API EntityPreorderIterator
{
public:
	explicit EntityPreorderIterator(sal_maybenull Entity* start);

	explicit EntityPreorderIterator(sal_maybenull Entity* start, sal_maybenull Entity* current);

	// NOTE: Assumming the iterator is valid and so the returned pointer will not be null.
	sal_notnull Entity* operator->() {
		return mCurrent;
	}

	//! Return the current element.
	sal_notnull Entity* current() {
		return mCurrent;
	}

	//! Returns true if there are NO more items in the collection.
	bool ended() const {
		return mCurrent == nullptr;
	}

	//! Returns the next element in the collection, and advances to the next.
	sal_maybenull Entity* next();

	/*!	Returns and advance to the next enabled Entity.
		If an Entity is disabled, all it's children are also skipped.

		\code
		if(node && node->enabled) {	// Take care if the first node is disabled
			for(EntityPreorderIterator itr(node); !itr.ended(); itr.nextEnabled()) {
				// Do something with itr
				// ...
			}
		}
		\endcode
	 */
	sal_maybenull Entity* nextEnabled();

	/*!	Returns and advances to the next element in the collection, without visiting current node's children.
		Usefull when encountering disabled Entity, for example:

		\code
		for(EntityPreorderIterator itr(node); !itr.ended(); ) {
			if(!itr->enabled) {
				itr.skipChildren();
				continue;
			}
			// Do something with itr
			// ...
			itr.next();
		}
		\endcode
	 */
	sal_maybenull Entity* skipChildren();

	//!	Returns how many preorder iterations are the 2 Entity aparted from.
	static sal_checkreturn int offsetFrom(const Entity& from, const Entity& to);

	//!	Returns an Entity identified by an offset from an Entity.
	static sal_maybenull Entity* entityByOffset(const Entity& entity, int offset);

protected:
	Entity* mCurrent;
	//! The position where this iterator is constructed, so it knows where to stop.
	const Entity* mStart;
};	// EntityPreorderIterator

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_ENTITYITERATOR__
