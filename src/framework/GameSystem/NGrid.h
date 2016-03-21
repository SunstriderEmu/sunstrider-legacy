/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef TRINITY_NGRID_H
#define TRINITY_NGRID_H

/** NGrid is nothing more than a wrapper of the Grid with an NxN cells
 */

#include "GameSystem/Grid.h"
#include "GameSystem/GridReference.h"
#include "Timer.h"

template
<
unsigned int N,
class ACTIVE_OBJECT,
class WORLD_OBJECT_TYPES,
class GRID_OBJECT_TYPES,
class ThreadModel = Trinity::SingleThreaded<ACTIVE_OBJECT>
>
class NGrid
{
    public:

        typedef Grid<ACTIVE_OBJECT, WORLD_OBJECT_TYPES, GRID_OBJECT_TYPES, ThreadModel> GridType;
        NGrid(uint32 id, int32 x, int32 y) :
            i_gridId(id), i_x(x), i_y(y), i_GridObjectDataLoaded(false)
            {
            }

        const GridType& operator()(unsigned short x, unsigned short y) const
        {
            ASSERT(x < N);
            ASSERT(y < N);
            return i_cells[x][y];
        }

        GridType& operator()(unsigned short x, unsigned short y)
        {
            ASSERT(x < N);
            ASSERT(y < N);
            return i_cells[x][y];
        }

        inline const uint32& GetGridId(void) const { return i_gridId; }
        inline void SetGridId(const uint32 id) const { i_gridId = id; }
        inline int32 getX() const { return i_x; }
        inline int32 getY() const { return i_y; }

        void link(GridRefManager<NGrid<N, ACTIVE_OBJECT, WORLD_OBJECT_TYPES, GRID_OBJECT_TYPES, ThreadModel> >* pTo)
        {
            i_Reference.link(pTo, this);
        }
        bool isGridObjectDataLoaded() const { return i_GridObjectDataLoaded; }
        void setGridObjectDataLoaded(bool pLoaded) { i_GridObjectDataLoaded = pLoaded; }

        template<class SPECIFIC_OBJECT> void AddWorldObject(const uint32 x, const uint32 y, SPECIFIC_OBJECT *obj)
        {
            getGridType(x, y).AddWorldObject(obj);
        }

        template<class SPECIFIC_OBJECT> void RemoveWorldObject(const uint32 x, const uint32 y, SPECIFIC_OBJECT *obj)
        {
            getGridType(x, y).RemoveWorldObject(obj);
        }

        template<class T, class TT> void Visit(TypeContainerVisitor<T, TypeMapContainer<TT> > &visitor)
        {
            for(unsigned int x=0; x < N; ++x)
                for(unsigned int y=0; y < N; ++y)
                    getGridType(x, y).Visit(visitor);
        }

        template<class T, class TT> void Visit(const uint32 &x, const uint32 &y, TypeContainerVisitor<T, TypeMapContainer<TT> > &visitor)
        {
            getGridType(x, y).Visit(visitor);
        }

        //This gets the player count in grid
        //I disable this to avoid confusion (active object usually means something else)
        /*
        uint32 GetActiveObjectCountInGrid() const
        {
            uint32 count = 0;
            for (uint32 x = 0; x < N; ++x)
                for (uint32 y = 0; y < N; ++y)
                    count += i_cells[x][y].ActiveObjectsInGrid();
            return count;
        }
        */

        template<class SPECIFIC_OBJECT> bool AddGridObject(const uint32 x, const uint32 y, SPECIFIC_OBJECT *obj)
        {
            return getGridType(x, y).AddGridObject(obj);
        }

        template<class SPECIFIC_OBJECT> bool RemoveGridObject(const uint32 x, const uint32 y, SPECIFIC_OBJECT *obj)
        {
            return getGridType(x, y).RemoveGridObject(obj);
        }

        template<class T>
        uint32 GetWorldObjectCountInNGrid() const
        {
            uint32 count = 0;
            for (uint32 x = 0; x < N; ++x)
                for (uint32 y = 0; y < N; ++y)
                    count += i_cells[x][y].template GetWorldObjectCountInGrid<T>();
            return count;
        }

    private:

        GridType& getGridType(const uint32& x, const uint32& y)
        {
            ASSERT(x < N);
            ASSERT(y < N);
            return i_cells[x][y];
        }

        uint32 i_gridId;
        GridReference<NGrid<N, ACTIVE_OBJECT, WORLD_OBJECT_TYPES, GRID_OBJECT_TYPES, ThreadModel> > i_Reference;
        int32 i_x;
        int32 i_y;
        GridType i_cells[N][N];
        bool i_GridObjectDataLoaded;
};
#endif

