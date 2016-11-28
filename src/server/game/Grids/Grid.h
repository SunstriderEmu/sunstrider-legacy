
#ifndef TRINITY_GRID_H
#define TRINITY_GRID_H

/*
  @class Grid
  Grid is a logical segment of the game world represented inside TrinIty.
  Grid is bind at compile time to a particular type of object which
  we call it the object of interested.  There are many types of loader,
  specially, dynamic loader, static loader, or on-demand loader.  There's
  a subtle difference between dynamic loader and on-demand loader but
  this is implementation specific to the loader class.  From the
  Grid's perspective, the loader meets its API requirement is suffice.
*/

#include "Define.h"
#include "ThreadingModel.h"
#include "TypeContainer.h"
#include "TypeContainerVisitor.h"

// forward declaration
template<class A, class T, class O> class GridLoader;

template
<
class ACTIVE_OBJECT,
class WORLD_OBJECT_TYPES,
class GRID_OBJECT_TYPES,
class ThreadModel = Trinity::SingleThreaded<ACTIVE_OBJECT>
>
class TC_GAME_API Grid
{
    // allows the GridLoader to access its internals
    template<class A, class T, class O> friend class GridLoader;
    public:

        /** destructor to clean up its resources. This includes unloading the
        grid if it has not been unload.
        */
        ~Grid() = default;

        /** an object of interested enters the grid
         */
        template<class SPECIFIC_OBJECT> void AddWorldObject(SPECIFIC_OBJECT *obj)
        {
            if(!i_objects.template insert<SPECIFIC_OBJECT>(obj))
                ABORT();
        }

        /** an object of interested exits the grid
         */
        template<class SPECIFIC_OBJECT> void RemoveWorldObject(SPECIFIC_OBJECT *obj)
        {
            if(!i_objects.template remove<SPECIFIC_OBJECT>(obj))
                ABORT();
        }

        /** Refreshes/update the grid. This required for remote grids.
         */
        void RefreshGrid(void) { /* TBI */}

        /** Locks a grid.  Any object enters must wait until the grid is unlock.
         */
        void LockGrid(void) { /* TBI */ }

        /** Unlocks the grid.
         */
        void UnlockGrid(void) { /* TBI */ }

        /** Grid visitor for grid objects
         */
        template<class T> void Visit(TypeContainerVisitor<T, TypeMapContainer<GRID_OBJECT_TYPES> > &visitor)
        {
            visitor.Visit(i_container);
        }

        /** Grid visitor for world objects
         */
        template<class T> void Visit(TypeContainerVisitor<T, TypeMapContainer<WORLD_OBJECT_TYPES> > &visitor)
        {
            visitor.Visit(i_objects);
        }

        /** Returns the number of object within the grid.
         */
        //unsigned int ActiveObjectsInGrid(void) const { return i_objects.template Count<ACTIVE_OBJECT>(); }
        template<class T>
        uint32 GetWorldObjectCountInGrid() const
        {
            return i_objects.template Count<T>();
        }

        /** Inserts a container type object into the grid.
         */
        template<class SPECIFIC_OBJECT> void AddGridObject(SPECIFIC_OBJECT *obj)
        {
            if(!i_container.template insert<SPECIFIC_OBJECT>(obj))
                ABORT();
        }

        /** Removes a containter type object from the grid
         */
        template<class SPECIFIC_OBJECT> void RemoveGridObject(SPECIFIC_OBJECT *obj)
        {
            if(!i_container.template remove<SPECIFIC_OBJECT>(obj))
                ABORT();
        }

    private:

        typedef typename ThreadModel::Lock Guard;
        typedef typename ThreadModel::VolatileType VolatileType;

        TypeMapContainer<GRID_OBJECT_TYPES> i_container;
        TypeMapContainer<WORLD_OBJECT_TYPES> i_objects;
        //typedef std::set<void*> ActiveGridObjects;
        //ActiveGridObjects m_activeGridObjects;
};
#endif

