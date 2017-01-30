
#ifndef TRINITY_OBJECTGRIDLOADER_H
#define TRINITY_OBJECTGRIDLOADER_H

#include "Define.h"
#include "GridLoader.h"
#include "GridDefines.h"
#include "Cell.h"

class ObjectWorldLoader;

class ObjectGridLoader
{
    friend class ObjectWorldLoader;

    public:
        ObjectGridLoader(NGridType &grid, Map* map, const Cell &cell)
            : i_cell(cell), i_grid(grid), i_map(map), i_gameObjects(0), i_creatures(0), i_corpses (0)
            {}

        void Load(GridType &grid);
        void Visit(GameObjectMapType &m);
        void Visit(CreatureMapType &m);
        void Visit(CorpseMapType &) {}

        void Visit(DynamicObjectMapType&) { }

        void LoadN(void);

    private:
        Cell i_cell;
        NGridType &i_grid;
        Map* i_map;
        uint32 i_gameObjects;
        uint32 i_creatures;
        uint32 i_corpses;
};

class ObjectGridUnloader
{
    public:
        ObjectGridUnloader(NGridType &grid) : i_grid(grid) {}

        void MoveToRespawnN();
        void UnloadN()
        {
            for(uint32 x=0; x < MAX_NUMBER_OF_CELLS; ++x)
            {
                for(uint32 y=0; y < MAX_NUMBER_OF_CELLS; ++y)
                {
                    GridLoader<Player, AllWorldObjectTypes, AllGridObjectTypes> loader;
                    loader.Unload(i_grid(x, y), *this);
                }
            }
        }

        void Unload(GridType &grid);
        template<class T> void Visit(GridRefManager<T> &m);
    private:
        NGridType &i_grid;
};

class ObjectGridStoper
{
    public:
        ObjectGridStoper(NGridType &grid) : i_grid(grid) {}

        void StopN()
        {
            for(uint32 x=0; x < MAX_NUMBER_OF_CELLS; ++x)
            {
                for(uint32 y=0; y < MAX_NUMBER_OF_CELLS; ++y)
                {
                    GridLoader<Player, AllWorldObjectTypes, AllGridObjectTypes> loader;
                    loader.Stop(i_grid(x, y), *this);
                }
            }
        }

        void Stop(GridType &grid);
        void Visit(CreatureMapType &m);

        template<class NONACTIVE> void Visit(GridRefManager<NONACTIVE> &) {}
    private:
        NGridType &i_grid;
};

class ObjectGridCleaner
{
    public:
        ObjectGridCleaner(NGridType &grid) : i_grid(grid) {}

        void CleanN()
        {
            for(uint32 x=0; x < MAX_NUMBER_OF_CELLS; ++x)
            {
                for(uint32 y=0; y < MAX_NUMBER_OF_CELLS; ++y)
                {
                    GridLoader<Player, AllWorldObjectTypes, AllGridObjectTypes> loader;
                    loader.Stop(i_grid(x, y), *this);
                }
            }
        }

        void Stop(GridType &grid);
        void Visit(CreatureMapType &m);
        template<class T> void Visit(GridRefManager<T> &);
    private:
        NGridType &i_grid;
};

typedef GridLoader<Player, AllWorldObjectTypes, AllGridObjectTypes> GridLoaderType;
#endif

