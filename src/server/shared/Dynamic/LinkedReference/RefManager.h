#ifndef _REFMANAGER_H
#define _REFMANAGER_H
//=====================================================

#include "Dynamic/LinkedList.h"
#include "Dynamic/LinkedReference/Reference.h"

template <class TO, class FROM>
class RefManager : public LinkedListHead
{
public:
	typedef LinkedListHead::Iterator<Reference<TO, FROM>> iterator;
	RefManager() { }

	Reference<TO, FROM>* getFirst() { return static_cast<Reference<TO, FROM>*>(LinkedListHead::getFirst()); }

	Reference<TO, FROM> const* getFirst() const { return static_cast<Reference<TO, FROM> const*>(LinkedListHead::getFirst()); }

	iterator begin() { return iterator(getFirst()); }
	iterator end() { return iterator(nullptr); }

	virtual ~RefManager()
	{
		clearReferences();
	}

	void clearReferences()
	{
		while (Reference<TO, FROM>* ref = getFirst())
			ref->invalidate();
	}
};

//=====================================================
#endif
