#ifndef TRINITY_SPELLAURAS_H
#define TRINITY_SPELLAURAS_H

#include "SpellAuraDefines.h"

struct DamageManaShield
{
    uint32 m_spellId;
    uint32 m_modType;
    int32 m_schoolType;
    uint32 m_totalAbsorb;
    uint32 m_currAbsorb;
};

struct Modifier
{
    AuraType m_auraname;
    int32 m_amount;
    uint32 m_miscvalue;
    uint32 periodictime;
};

class Unit;
class SpellInfo;
struct SpellModifier;
struct ProcTriggerSpell;

// forward decl
class Aura;
struct ChannelTargetData;

typedef void(Aura::*pAuraHandler)(bool Apply, bool Real);
// Real == true at aura add/remove
// Real == false at aura mod unapply/reapply; when adding/removing dependent aura/item/stat mods
//
// Code in aura handler can be guarded by if(Real) check if it should execution only at real add/remove of aura
//
// MAIN RULE: Code MUST NOT be guarded by if(Real) check if it modifies any stats
//      (percent auras, stats mods, etc)
// Second rule: Code must be guarded by if(Real) check if it modifies object state (start/stop attack, send packets to client, etc)
//
// Other case choice: each code line moved under if(Real) check is Trinity speedup,
//      each setting object update field code line moved under if(Real) check is significant Trinity speedup, and less server->client data sends
//      each packet sending code moved under if(Real) check is _large_ Trinity speedup, and lot less server->client data sends

class TC_GAME_API Aura
{
    friend Aura* CreateAura(SpellInfo const* spellproto, uint32 eff, int32 *currentBasePoints, Unit *target, Unit *caster, Item* castItem);

    public:
        //aura handlers
        void HandleNULL(bool, bool)
        {
            // NOT IMPLEMENTED
        }
        void HandleUnused(bool, bool)
        {
            // NOT USED BY ANY SPELL OR USELESS
        }
        void HandleNoImmediateEffect(bool, bool)
        {
            // aura not have immediate effect at add/remove and handled by ID in other code place
        }
        void HandleBindSight(bool Apply, bool Real);
        void HandleModPossess(bool Apply, bool Real);
        void HandlePeriodicDamage(bool Apply, bool Real);
        void HandleAuraDummy(bool Apply, bool Real);
        void HandleAuraPeriodicDummy(bool apply, bool Real);
        void HandleModConfuse(bool Apply, bool Real);
        void HandleModCharm(bool Apply, bool Real);
        void HandleModFear(bool Apply, bool Real);
        void HandlePeriodicHeal(bool Apply, bool Real);
        void HandleModAttackSpeed(bool Apply, bool Real);
        void HandleModMeleeRangedSpeedPct(bool apply, bool Real);
        void HandleModCombatSpeedPct(bool apply, bool Real);
        void HandleModThreat(bool Apply, bool Real);
        void HandleModTaunt(bool Apply, bool Real);
        void HandleFeignDeath(bool Apply, bool Real);
        void HandleAuraModDisarm(bool Apply, bool Real);
        void HandleAuraModStalked(bool Apply, bool Real);
        void HandleAuraWaterWalk(bool Apply, bool Real);
        void HandleAuraFeatherFall(bool Apply, bool Real);
        void HandleAuraHover(bool Apply, bool Real);
        void HandleAddModifier(bool Apply, bool Real);
        void HandleAuraModStun(bool Apply, bool Real);
        void HandleModDamageDone(bool Apply, bool Real);
        void HandleAuraUntrackable(bool Apply, bool Real);
        void HandleAuraEmpathy(bool Apply, bool Real);
        void HandleModOffhandDamagePercent(bool apply, bool Real);
        void HandleAuraModRangedAttackPower(bool Apply, bool Real);
        void HandleAuraModIncreaseEnergyPercent(bool Apply, bool Real);
        void HandleAuraModIncreaseHealthPercent(bool Apply, bool Real);
        void HandleAuraModRegenInterrupt(bool Apply, bool Real);
        void HandleHaste(bool Apply, bool Real);
        void HandlePeriodicTriggerSpell(bool Apply, bool Real);
        void HandlePeriodicEnergize(bool Apply, bool Real);
        void HandleAuraModResistanceExclusive(bool Apply, bool Real);
        void HandleModStealth(bool Apply, bool Real);
		void HandleModStealthDetect(bool Apply, bool Real);
        void HandleModInvisibility(bool Apply, bool Real);
        void HandleModInvisibilityDetect(bool Apply, bool Real);
        void HandleAuraModTotalHealthPercentRegen(bool Apply, bool Real);
        void HandleAuraModTotalPowerPercentRegen(bool Apply, bool Real);
        void HandleAuraModResistance(bool Apply, bool Real);
        void HandleAuraModRoot(bool Apply, bool Real);
        void HandleAuraModSilence(bool Apply, bool Real);
        void HandleAuraModStat(bool Apply, bool Real);
        void HandleAuraModIncreaseSpeed(bool Apply, bool Real);
        void HandleAuraModIncreaseMountedSpeed(bool Apply, bool Real);
        void HandleAuraModIncreaseFlightSpeed(bool Apply, bool Real);
        void HandleAuraModDecreaseSpeed(bool Apply, bool Real);
        void HandleAuraModUseNormalSpeed(bool Apply, bool Real);
        void HandleAuraModIncreaseHealth(bool Apply, bool Real);
        void HandleAuraModIncreaseEnergy(bool Apply, bool Real);
        void HandleAuraModShapeshift(bool Apply, bool Real);
        void HandleAuraModEffectImmunity(bool Apply, bool Real);
        void HandleAuraModStateImmunity(bool Apply, bool Real);
        void HandleAuraModSchoolImmunity(bool Apply, bool Real);
        void HandleAuraModDmgImmunity(bool Apply, bool Real);
        void HandleAuraModDispelImmunity(bool Apply, bool Real);
        void HandleAuraProcTriggerSpell(bool Apply, bool Real);
        void HandleAuraTrackCreatures(bool Apply, bool Real);
        void HandleAuraTrackResources(bool Apply, bool Real);
        void HandleAuraModParryPercent(bool Apply, bool Real);
        void HandleAuraModDodgePercent(bool Apply, bool Real);
        void HandleAuraModBlockPercent(bool Apply, bool Real);
        void HandleAuraModWeaponCritPercent(bool Apply, bool Real);
        void HandlePeriodicLeech(bool Apply, bool Real);
        void HandleModHitChance(bool Apply, bool Real);
        void HandleModSpellHitChance(bool Apply, bool Real);
        void HandleAuraModScale(bool Apply, bool Real);
        void HandlePeriodicManaLeech(bool Apply, bool Real);
        void HandleModCastingSpeed(bool Apply, bool Real);
        void HandleAuraMounted(bool Apply, bool Real);
        void HandleWaterBreathing(bool Apply, bool Real);
        void HandleModBaseResistance(bool Apply, bool Real);
        void HandleModRegen(bool Apply, bool Real);
        void HandleModPowerRegen(bool Apply, bool Real);
        void HandleModPowerRegenPCT(bool Apply, bool Real);
        void HandleChannelDeathItem(bool Apply, bool Real);
        void HandlePeriodicDamagePCT(bool Apply, bool Real);
        void HandleAuraModAttackPower(bool Apply, bool Real);
        void HandleAuraTransform(bool Apply, bool Real);
        void HandleModSpellCritChance(bool Apply, bool Real);
        void HandleAuraModIncreaseSwimSpeed(bool Apply, bool Real);
        void HandleModPowerCostPCT(bool Apply, bool Real);
        void HandleModPowerCost(bool Apply, bool Real);
        void HandleModPossessPet(bool Apply, bool Real);
        void HandleModMechanicImmunity(bool Apply, bool Real);
        void HandleAuraModSkill(bool Apply, bool Real);
        void HandleModDamagePercentDone(bool Apply, bool Real);
        void HandleModPercentStat(bool Apply, bool Real);
        void HandleModResistancePercent(bool Apply, bool Real);
        void HandleAuraModBaseResistancePCT(bool Apply, bool Real);
        void HandleAuraTrackStealthed(bool Apply, bool Real);
        void HandleModStealthLevel(bool Apply, bool Real);
        void HandleForceReaction(bool Apply, bool Real);
        void HandleAuraModRangedHaste(bool Apply, bool Real);
        void HandleRangedAmmoHaste(bool Apply, bool Real);
        void HandleModHealingDone(bool Apply, bool Real);
        void HandleModTotalPercentStat(bool Apply, bool Real);
        void HandleAuraModTotalThreat(bool Apply, bool Real);
        void HandleModUnattackable(bool Apply, bool Real);
        void HandleAuraModPacify(bool Apply, bool Real);
        void HandleAuraGhost(bool Apply, bool Real);
        void HandleAuraAllowFlight(bool Apply, bool Real);
        void HandleModRating(bool apply, bool Real);
        void HandleModTargetResistance(bool apply, bool Real);
        void HandleAuraModAttackPowerPercent(bool apply, bool Real);
        void HandleAuraModRangedAttackPowerPercent(bool apply, bool Real);
        void HandleAuraModRangedAttackPowerOfStatPercent(bool apply, bool Real);
        void HandleSpiritOfRedemption(bool apply, bool Real);
        void HandleModManaRegen(bool apply, bool Real);
        void HandleComprehendLanguage(bool apply, bool Real);
        void HandleShieldBlockValue(bool apply, bool Real);
        void HandleShieldBlockValuePercent(bool apply, bool Real);
        void HandleModSpellCritChanceShool(bool apply, bool Real);
        void HandleAuraRetainComboPoints(bool apply, bool Real);
        void HandleModSpellDamagePercentFromStat(bool apply, bool Real);
        void HandleModSpellHealingPercentFromStat(bool apply, bool Real);
        void HandleModSpellDamagePercentFromAttackPower(bool apply, bool Real);
        void HandleModSpellHealingPercentFromAttackPower(bool apply, bool Real);
        void HandleAuraModPacifyAndSilence(bool Apply, bool Real);
        void HandleAuraModIncreaseMaxHealth(bool apply, bool Real);
        void HandleAuraModExpertise(bool apply, bool Real);
        void HandleForceMoveForward(bool apply, bool Real);
        void HandleAuraModResistenceOfStatPercent(bool apply, bool Real);
        void HandleAuraPowerBurn(bool apply, bool Real);
        void HandleSchoolAbsorb(bool apply, bool Real);
        void HandlePreventFleeing(bool apply, bool Real);
        void HandleManaShield(bool apply, bool Real);
        void HandleArenaPreparation(bool apply, bool Real);
        void HandleAttackerPowerBonus(bool apply, bool Real);
        void HandleAOECharm(bool apply, bool Real);
        void HandlePeriodicTriggerSpellWithValue(bool apply, bool Real);
        void HandleAuraIgnored(bool apply, bool Real);
        void HandleModStateImmunityMask(bool apply, bool Real);
        void HandleAuraCloneCaster(bool apply, bool Real);

        //custom on WM
        void HandleAuraApplyExtraFlag(bool apply, bool Real);
        void HandleAuraImmunityId(bool apply, bool Real);

        virtual ~Aura();

        //Compat TC
        inline Aura* GetBase() { return this; }

        void SetModifier(AuraType t, int32 a, uint32 pt, uint32 miscValue);
        AuraType GetAuraType() const { return m_modifier.m_auraname; }
        Modifier const* GetModifier() const { return &m_modifier;}
        int32 GetAmount() const { return m_modifier.m_amount; }
        int32 GetModifierValuePerStack() const { return m_modifier.m_amount;}
        int32 GetModifierValue() const { return m_modifier.m_amount * m_stackAmount;}
        int32 GetMiscValue() const;
        int32 GetMiscBValue() const;
        void SetModifierValuePerStack(int32 newAmount);
        void SetModifierValue(int32 newAmount) { m_modifier.m_amount = newAmount; }

        SpellInfo const* GetSpellInfo() const { return m_spellProto; }
        uint32 GetId() const;
        ObjectGuid GetCastItemGUID() const { return m_castItemGuid; }
		uint8 GetEffIndex() const{ return m_effIndex; }
        uint8 GetEffectMask() const { return 1 << m_effIndex; }
        int32 GetBasePoints() const { return m_currentBasePoints; }
        void SetBasePoints(uint32 basePoints) { m_currentBasePoints = basePoints; }

        int32 GetMaxDuration() const { return m_maxduration; }
        void SetMaxDuration(int32 duration) { m_maxduration = duration; }
        int32 GetDuration() const { return m_duration; }
        void SetDuration(int32 duration)
        {
            m_duration = duration;
            if (duration<0)
                m_permanent=true;
            else
                m_permanent=false;
        }
        time_t GetApplyTime() { return m_applyTime; }

        bool IsActive() { return m_active; }
        bool IsExpired() const { return !GetDuration() && !(IsPermanent() || IsPassive()); }
        void UpdateAuraDuration();
        void SendAuraDurationForCaster(Player* caster);
        void UpdateSlotCounterAndDuration();
        uint32 GetTickNumber() const { return m_tickNumber; }

        ObjectGuid const& GetCasterGUID() const { return m_caster_guid; }
        Unit* GetCaster() const;
        //compat TC
        inline WorldObject* GetOwner() const { return GetCaster(); }
        //compat TC
        Unit* GetUnitOwner() const { return GetCaster(); }

        Unit* GetTarget() const { return m_target; }
        void SetTarget(Unit* target) { m_target = target; }
        void SetLoadedState(ObjectGuid caster_guid, int32 damage, int32 maxduration, int32 duration, int32 charges)
        {
            m_caster_guid = caster_guid;
            m_modifier.m_amount = damage;
            m_maxduration = maxduration;
            m_duration = duration;
            m_procCharges = charges;
        }

        uint8 GetAuraSlot() const { return m_auraSlot; }
        void SetAuraSlot(uint8 slot) { m_auraSlot = slot; }
        void UpdateAuraCharges()
        {
            uint8 slot = GetAuraSlot();

            // only aura in slot with charges and without stack limitation
            if (slot < MAX_AURAS && m_procCharges >= 1 && GetSpellInfo()->StackAmount==0)
                SetAuraApplication(slot, m_procCharges - 1);
        }

        bool IsPositive() const { return m_positive; }
        void SetNegative() { m_positive = false; }
        void SetPositive() { m_positive = true; }
        void SetDeathPersistent(bool set) { m_isDeathPersist = set; }

        bool IsPermanent() const { return m_permanent; }
        bool IsArea() const { return m_isAreaAura; }
        bool IsPeriodic() const { return m_isPeriodic; }
        bool IsTrigger() const { return m_IsTrigger; }
        bool IsPassive() const { return m_isPassive; }
        bool IsPersistent() const { return m_isPersistent; }
		bool IsCooldownStartedOnEvent() const;
        bool IsDeathPersistent() const { return m_isDeathPersist; }
        bool IsRemovedOnShapeLost() const { return m_isRemovedOnShapeLost; }
        bool IsRemoved() const { return m_isRemoved; }
        bool IsInUse() const { return m_in_use;}
        void CleanupTriggeredSpells();

        bool CanBeSaved() const;

        virtual void Update(uint32 diff);
        void ApplyModifier(bool apply, bool Real = false);

        void _AddAura(bool sameSlot = true);
        void _RemoveAura();

        void TriggerSpell();

        bool IsUpdated() { return m_updated; }
        void SetUpdated(bool val) { m_updated = val; }
        void SetRemoveMode(AuraRemoveMode mode) { m_removeMode = mode; }
		AuraRemoveMode GetRemoveMode() const { return m_removeMode; }

        int32 m_procCharges;
        void SetCharges(int32 charges) { m_procCharges = charges; }
        int32 GetCharges() const { return m_procCharges; }
        uint8 CalcMaxCharges(Unit* caster) const;
        uint8 CalcMaxCharges() const { return CalcMaxCharges(GetCaster()); }
        bool ModCharges(int32 num, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        bool DropCharge(AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT) { return ModCharges(-1, removeMode); }
        bool IsUsingCharges() const { return m_procCharges > 0;  }

        Unit* GetTriggerTarget() const;

        // add/remove SPELL_AURA_MOD_SHAPESHIFT (36) linked auras
        void HandleShapeshiftBoosts(bool apply);

        // Allow Apply Aura Handler to modify and access m_AuraDRGroup
        void setDiminishGroup(DiminishingGroup group) { m_AuraDRGroup = group; }
        DiminishingGroup getDiminishGroup() const { return m_AuraDRGroup; }

        void SendTickImmune(Unit* target, Unit* caster) const;
        void PeriodicTick();
        void PeriodicDummyTick();

        int32 GetStackAmount() const {return m_stackAmount;}
        void SetStackAmount(int32 amount) {m_stackAmount=amount;}

        // Single cast aura helpers
        void UnregisterSingleCastAura();
        bool IsSingleTarget() const {return m_isSingleTargetAura;}
        void SetIsSingleTarget(bool val) { m_isSingleTargetAura = val;}
        bool DoesAuraApplyAuraName(uint32 name);
        int32 GetPeriodicTimer() { return m_periodicTimer; }
        void SetPeriodicTimer(int32 newTimer) { m_periodicTimer = newTimer; }
    protected:
        Aura(SpellInfo const* spellproto, uint8 eff, int32 *currentBasePoints, Unit *target, Unit *caster = nullptr, Item* castItem = nullptr);

        Modifier m_modifier;
        SpellModifier *m_spellmod;
        uint8 m_effIndex;
        SpellInfo const *m_spellProto;
        int32 m_currentBasePoints;                          // cache SpellInfo::EffectBasePoints and use for set custom base points
        ObjectGuid m_caster_guid;
        Unit* m_target;
        int32 m_maxduration;
        int32 m_duration;
        uint32 m_tickNumber;
        int32 m_timeCla;
        ObjectGuid m_castItemGuid;                              // it is NOT safe to keep a pointer to the item because it may get deleted
        time_t m_applyTime;
        bool m_active;

        AuraRemoveMode m_removeMode;

        uint8 m_auraSlot;

        bool m_positive:1;
        bool m_permanent:1;
        bool m_isPeriodic:1;
        bool m_IsTrigger:1;
        bool m_isAreaAura:1;
        bool m_isPassive:1;
        bool m_isPersistent:1;
        bool m_isDeathPersist:1;
        bool m_isRemovedOnShapeLost:1;
        bool m_isRemoved:1;
        bool m_updated:1;
        bool m_in_use:1;                                    // true while in Aura::ApplyModifier call
        bool m_isSingleTargetAura:1;                        // true if it's a single target spell and registered at caster - can change at spell steal for example

        //channel information for channel triggering
        ChannelTargetData* m_channelData;

        int32 m_periodicTimer;
        int32 m_amplitude;
        uint32 m_PeriodicEventId;
        DiminishingGroup m_AuraDRGroup;

        int32 m_stackAmount;
    private:
        void SetAura(uint32 slot, bool remove) { m_target->SetUInt32Value(UNIT_FIELD_AURA + slot, remove ? 0 : GetId()); }
        void SetAuraFlag(uint32 slot, bool add);
        void SetAuraLevel(uint32 slot, uint32 level);
        void SetAuraApplication(uint32 slot, int8 count);
};

class TC_GAME_API AreaAura : public Aura
{
    public:
        AreaAura(SpellInfo const* spellproto, uint32 eff, int32 *currentBasePoints, Unit *target, Unit *caster = nullptr, Item* castItem = nullptr);
        ~AreaAura() override;
        void Update(uint32 diff) override;
        bool CheckTarget(Unit *target);
    private:
        float m_radius;
        AreaAuraType m_areaAuraType;
};

/* PersistentAreaAura are maintained by a list of multiple dynamic objects from the same caster.
The aura is removed if we can't find any sources dynobjects in range.
*/
class TC_GAME_API PersistentAreaAura : public Aura
{
    public:
        PersistentAreaAura(SpellInfo const* spellproto, uint32 eff, int32 *currentBasePoints, Unit *target, Unit *caster = nullptr, Item* castItem = nullptr);
        ~PersistentAreaAura() override;
        void Update(uint32 diff) override;
        void AddSource(DynamicObject* dynObj);
    public:
        std::list<ObjectGuid> sourceDynObjects; //list of dynamic objects the aura originate from
};

Aura* CreateAura(SpellInfo const* spellproto, uint32 eff, int32 *currentBasePoints, Unit *target, Unit *caster = nullptr, Item* castItem = nullptr);
#endif

