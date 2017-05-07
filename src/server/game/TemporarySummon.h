#ifndef TRINITYCORE_TEMPSUMMON_H
#define TRINITYCORE_TEMPSUMMON_H

#include "Creature.h"
#include "ObjectAccessor.h"

class TC_GAME_API TemporarySummon : public Creature
{
    public:
        explicit TemporarySummon(uint64 summoner = 0);
        ~TemporarySummon() override{};
        void Update(uint32 time) override;
        void Summon(TempSummonType type, uint32 lifetime, Map* map);
        void UnSummon();
        void SaveToDB(uint32 mapid, uint8 spawnMask) override { }
        TempSummonType GetSummonType() { return m_type; }
        Unit* GetSummoner() const override { return m_summoner ? ObjectAccessor::GetUnit(*this, m_summoner) : nullptr; }
        bool DespawnOnDeath();
    private:
        TempSummonType m_type;
        uint32 m_timer;
        uint32 m_lifetime;
        uint64 m_summoner;
};
#endif

