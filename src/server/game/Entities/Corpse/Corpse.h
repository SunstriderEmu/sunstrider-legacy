
#ifndef TRINITYCORE_CORPSE_H
#define TRINITYCORE_CORPSE_H

#include "Object.h"
#include "GridDefines.h"
#include "LootMgr.h"

enum CorpseType
{
    CORPSE_BONES             = 0,
    CORPSE_RESURRECTABLE_PVE = 1,
    CORPSE_RESURRECTABLE_PVP = 2
};
#define MAX_CORPSE_TYPE        3

// Value equal client resurrection dialog show radius.
#define CORPSE_RECLAIM_RADIUS 39

enum CorpseFlags
{
    CORPSE_FLAG_NONE        = 0x00,
    CORPSE_FLAG_BONES       = 0x01,
    CORPSE_FLAG_UNK1        = 0x02,
    CORPSE_FLAG_UNK2        = 0x04,
    CORPSE_FLAG_HIDE_HELM   = 0x08,
    CORPSE_FLAG_HIDE_CLOAK  = 0x10,
    CORPSE_FLAG_LOOTABLE    = 0x20
};

class TC_GAME_API Corpse : public WorldObject, public GridObject<Corpse>
{
    public:
        explicit Corpse( CorpseType type = CORPSE_BONES );
        ~Corpse( ) override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool Create(ObjectGuid::LowType guidlow );
        bool Create(ObjectGuid::LowType guidlow, Player *owner);

		void SaveToDB();
		bool LoadCorpseFromDB(ObjectGuid::LowType guid, Field* fields);

		void DeleteFromDB(SQLTransaction& trans);
		static void DeleteFromDB(ObjectGuid const& ownerGuid, SQLTransaction& trans);

        ObjectGuid GetOwnerGUID() const override;
        uint32 GetFaction() const override;

        time_t const& GetGhostTime() const { return m_time; }
        void ResetGhostTime();
        CorpseType GetType() const { return m_type; }

		CellCoord const& GetCellCoord() const { return _cellCoord; }
		void SetCellCoord(CellCoord const& cellCoord) { _cellCoord = cellCoord; }

        Loot loot;                                          // remove insignia ONLY at BG
        Player* lootRecipient;

		bool IsExpired(time_t t) const;

    private:
        CorpseType m_type;
        time_t m_time;
		CellCoord _cellCoord;
        bool _noDatabaseSave = false;
};
#endif

