
#ifndef TRINITYCORE_DYNAMICOBJECT_H
#define TRINITYCORE_DYNAMICOBJECT_H

#include "Object.h"

class Unit;

class TC_GAME_API DynamicObject : public WorldObject, public GridObject<DynamicObject>
{
    public:
        typedef std::set<Unit*> AffectedSet;
        explicit DynamicObject(bool isWorldObject);

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool Create(uint32 guidlow, Unit *caster, uint32 spellId, uint32 effIndex, float x, float y, float z, int32 duration, float radius);
        void Update(uint32 p_time) override;
        void Delete();
        uint32 GetSpellId() const { return m_spellId; }
        uint32 GetEffIndex() const { return m_effIndex; }
        uint32 GetDuration() const { return m_aliveDuration; }
        uint64 GetCasterGUID() const { return m_casterGuid; }
        Unit* GetCaster() const;
        float GetRadius() const { return m_radius; }
        bool IsAffecting(Unit *unit) const { return m_affected.find(unit) != m_affected.end(); }
        void AddAffected(Unit *unit);
        void RemoveAffected(Unit *unit) { m_affected.erase(unit); }
        void Delay(int32 delaytime);
        bool IsVisibleForInState(Player const* u, bool inVisibleList) const override;

    protected:
        uint64 m_casterGuid;
        uint32 m_spellId;
        uint32 m_effIndex;
        int32 m_aliveDuration;
        uint32 m_updateTimer;
        time_t m_nextThinkTime;
        float m_radius;
        AffectedSet m_affected;
    private:
        GridReference<DynamicObject> m_gridRef;
};
#endif

