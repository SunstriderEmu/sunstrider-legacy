
#ifndef TRINITY_TYPECONTAINER_H
#define TRINITY_TYPECONTAINER_H

/*
* Here, you'll find a series of containers that allow you to hold multiple
* types of object at the same time.
*/

#include <map>
#include <unordered_map>
#include <vector>
#include "Define.h"
#include "Dynamic/TypeList.h"
#include "GridRefManager.h"

/*
* @class ContainerMapList is a mulit-type container for map elements
* By itself its meaningless but collaborate along with TypeContainers,
* it become the most powerfully container in the whole system.
*/
template<class OBJECT>
struct ContainerMapList
{
	//std::map<OBJECT_HANDLE, OBJECT *> _element;
	GridRefManager<OBJECT> _element;
};

template<>
struct ContainerMapList<TypeNull>                /* nothing is in type null */
{
};

template<class H, class T>
struct ContainerMapList<TypeList<H, T> >
{
	ContainerMapList<H> _elements;
	ContainerMapList<T> _TailElements;
};

template<class OBJECT, class KEY_TYPE>
struct ContainerUnorderedMap
{
	std::unordered_map<KEY_TYPE, OBJECT*> _element;
};

template<class KEY_TYPE>
struct ContainerUnorderedMap<TypeNull, KEY_TYPE>
{
};

template<class H, class T, class KEY_TYPE>
struct ContainerUnorderedMap<TypeList<H, T>, KEY_TYPE>
{
	ContainerUnorderedMap<H, KEY_TYPE> _elements;
	ContainerUnorderedMap<T, KEY_TYPE> _TailElements;
};

#include "TypeContainerFunctions.h"

/*
* @class TypeMapContainer contains a fixed number of types and is
* determined at compile time.  This is probably the most complicated
* class and do its simplest thing, that is, holds objects
* of different types.
*/

template<class OBJECT_TYPES>
class TypeMapContainer
{
public:
	template<class SPECIFIC_TYPE> size_t Count() const { return Trinity::Count(i_elements, (SPECIFIC_TYPE*)NULL); }

	/// inserts a specific object into the container
	template<class SPECIFIC_TYPE>
	bool insert(SPECIFIC_TYPE *obj)
	{
		SPECIFIC_TYPE* t = Trinity::Insert(i_elements, obj);
		return (t != NULL);
	}

	///  Removes the object from the container, and returns the removed object
	//template<class SPECIFIC_TYPE>
	//bool remove(SPECIFIC_TYPE* obj)
	//{
	//    SPECIFIC_TYPE* t = Trinity::Remove(i_elements, obj);
	//    return (t != NULL);
	//}

	ContainerMapList<OBJECT_TYPES> & GetElements(void) { return i_elements; }
	const ContainerMapList<OBJECT_TYPES> & GetElements(void) const { return i_elements; }

private:
	ContainerMapList<OBJECT_TYPES> i_elements;
};

template<class OBJECT_TYPES, class KEY_TYPE>
class TypeUnorderedMapContainer
{
public:
	template<class SPECIFIC_TYPE>
	bool Insert(KEY_TYPE const& handle, SPECIFIC_TYPE* obj)
	{
		return Trinity::Insert(_elements, handle, obj);
	}

	template<class SPECIFIC_TYPE>
	bool Remove(KEY_TYPE const& handle)
	{
		return Trinity::Remove(_elements, handle, (SPECIFIC_TYPE*)NULL);
	}

	template<class SPECIFIC_TYPE>
	SPECIFIC_TYPE* Find(KEY_TYPE const& handle)
	{
		return Trinity::Find(_elements, handle, (SPECIFIC_TYPE*)NULL);
	}

	ContainerUnorderedMap<OBJECT_TYPES, KEY_TYPE>& GetElements() { return _elements; }
	ContainerUnorderedMap<OBJECT_TYPES, KEY_TYPE> const& GetElements() const { return _elements; }

private:
	ContainerUnorderedMap<OBJECT_TYPES, KEY_TYPE> _elements;
};

#endif
