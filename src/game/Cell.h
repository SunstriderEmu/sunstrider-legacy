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

#ifndef TRINITY_CELL_H
#define TRINITY_CELL_H

#include "GameSystem/TypeContainerVisitor.h"
#include "GridDefines.h"
#include <cmath>

class Map;
class WorldObject;

enum District
{
    UPPER_DISTRICT = 1,
    LOWER_DISTRICT = 1 << 1,
    LEFT_DISTRICT = 1 << 2,
    RIGHT_DISTRICT = 1 << 3,
    CENTER_DISTRICT = 1 << 4,
    UPPER_LEFT_DISTRICT = (UPPER_DISTRICT | LEFT_DISTRICT),
    UPPER_RIGHT_DISTRICT = (UPPER_DISTRICT | RIGHT_DISTRICT),
    LOWER_LEFT_DISTRICT = (LOWER_DISTRICT | LEFT_DISTRICT),
    LOWER_RIGHT_DISTRICT = (LOWER_DISTRICT | RIGHT_DISTRICT),
    ALL_DISTRICT = (UPPER_DISTRICT | LOWER_DISTRICT | LEFT_DISTRICT | RIGHT_DISTRICT | CENTER_DISTRICT)
};

struct CellArea
{
    CellArea() {}
    CellArea(CellCoord low, CellCoord high) : low_bound(low), high_bound(high) {}

    bool operator!() const { return low_bound == high_bound; }

    void ResizeBorders(CellCoord& begin_cell, CellCoord& end_cell) const
    {
        begin_cell = low_bound;
        end_cell = high_bound;
    }

    CellCoord low_bound;
    CellCoord high_bound;
};

struct Cell
{
    Cell() { data.All = 0; }
    Cell(const Cell &cell) { data.All = cell.data.All; }
    explicit Cell(CellCoord const& p);
    explicit Cell(float x, float y);

    void Compute(uint32 &x, uint32 &y) const
    {
        x = data.Part.grid_x*MAX_NUMBER_OF_CELLS + data.Part.cell_x;
        y = data.Part.grid_y*MAX_NUMBER_OF_CELLS + data.Part.cell_y;
    }

    inline bool DiffCell(const Cell &cell) const
    {
        return( data.Part.cell_x != cell.data.Part.cell_x ||
            data.Part.cell_y != cell.data.Part.cell_y );
    }

    inline bool DiffGrid(const Cell &cell) const
    {
        return( data.Part.grid_x != cell.data.Part.grid_x ||
            data.Part.grid_y != cell.data.Part.grid_y );
    }

    uint32 CellX() const { return data.Part.cell_x; }
    uint32 CellY() const { return data.Part.cell_y; }
    uint32 GridX() const { return data.Part.grid_x; }
    uint32 GridY() const { return data.Part.grid_y; }
    bool NoCreate() const { return data.Part.nocreate; }
    void SetNoCreate() { data.Part.nocreate = 1; }

    CellCoord cellPair() const
    {
        return CellCoord(
            data.Part.grid_x*MAX_NUMBER_OF_CELLS+data.Part.cell_x,
            data.Part.grid_y*MAX_NUMBER_OF_CELLS+data.Part.cell_y);
    }

    Cell& operator=(const Cell &cell)
    {
        this->data.All = cell.data.All;
        return *this;
    }

    bool operator==(const Cell &cell) const { return (data.All == cell.data.All); }
    bool operator!=(const Cell &cell) const { return !operator==(cell); }
    union
    {
        struct
        {
            unsigned grid_x : 6;
            unsigned grid_y : 6;
            unsigned cell_x : 6;
            unsigned cell_y : 6;
            unsigned nocreate : 1;
            unsigned reserved : 7;
        } Part;
        uint32 All;
    } data;

    template<class T, class CONTAINER> void Visit(const CellCoord&, TypeContainerVisitor<T, CONTAINER> &visitor, Map &, const WorldObject&, float) const;
    template<class T, class CONTAINER> void Visit(const CellCoord &, TypeContainerVisitor<T, CONTAINER> &visitor, Map &, float radius, float x_off, float y_off) const;

    static CellArea CalculateCellArea(const WorldObject &obj, float radius);
    static CellArea CalculateCellArea(float x, float y, float radius);

private:
    template<class T, class CONTAINER> void VisitCircle(TypeContainerVisitor<T, CONTAINER> &, Map &, const CellCoord&, const CellCoord&) const;
};

#endif

