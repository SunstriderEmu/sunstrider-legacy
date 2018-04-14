
#ifndef TRINITYCORE_DYNAMICOBJECT_H
#define TRINITYCORE_DYNAMICOBJECT_H

#include "Object.h"

class Unit;

enum DynamicObjectType
{
    DYNAMIC_OBJECT_PORTAL           = 0x0,      // unused
    DYNAMIC_OBJECT_AREA_SPELL       = 0x1,
    DYNAMIC_OBJECT_FARSIGHT_FOCUS   = 0x2
};

class TC_GAME_API DynamicObject : public WorldObject, public GridObject<DynamicObject>, public MapObject
{
    public:
        typedef std::set<ObjectGuid> AffectedSet;
        explicit DynamicObject(bool isWorldObject);
		~DynamicObject();

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool CreateDynamicObject(ObjectGuid::LowType guidlow, Unit *caster, uint32 spellId, Position const& pos, float radius, DynamicObjectType type);
        void Update(uint32 p_time) override;
		void Remove();
		uint32 GetSpellId() const { return GetUInt32Value(DYNAMICOBJECT_SPELLID); }
        SpellInfo const* GetSpellInfo() const;
        void SetDuration(int32 newDuration);
        int32 GetDuration() const;
        ObjectGuid GetCasterGUID() const { return GetGuidValue(DYNAMICOBJECT_CASTER); }
        ObjectGuid GetOwnerGUID() const override { return GetCasterGUID(); }
		float GetRadius() const { return GetFloatValue(DYNAMICOBJECT_RADIUS); }
        void Delay(int32 delaytime);
        void SetAura(Aura* aura);
        void RemoveAura();

		void SetCasterViewpoint();
		void RemoveCasterViewpoint();

        void BindToCaster();
        void UnbindFromCaster();

        Unit* GetCaster() const { return _caster; }
        uint32 GetFaction() const override;

    protected:
        Unit* _caster;
        Aura* _aura;
        Aura* _removedAura;
        int32 _duration; // for non-aura dynobjects
		bool _isViewpoint;
};
#endif

