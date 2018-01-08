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
class AuraScript;

// forward decl
class Aura;
struct ChannelTargetData;

typedef void(Aura::*pAuraEffectHandler)(bool Apply, uint8 mode);
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
        void HandleNULL(bool, uint8)
        {
            // NOT IMPLEMENTED
        }
        void HandleUnused(bool, uint8)
        {
            // NOT USED BY ANY SPELL OR USELESS
        }
        void HandleNoImmediateEffect(bool, uint8)
        {
            // aura not have immediate effect at add/remove and handled by ID in other code place
        }
        void HandleBindSight(bool Apply, uint8 mode);
        void HandleModPossess(bool Apply, uint8 mode);
        void HandlePeriodicDamage(bool Apply, uint8 mode);
        void HandleAuraDummy(bool Apply, uint8 mode);
        void HandleAuraPeriodicDummy(bool apply, uint8 mode);
        void HandleModConfuse(bool Apply, uint8 mode);
        void HandleModCharm(bool Apply, uint8 mode);
        void HandleModFear(bool Apply, uint8 mode);
        void HandlePeriodicHeal(bool Apply, uint8 mode);
        void HandleModAttackSpeed(bool Apply, uint8 mode);
        void HandleModMeleeRangedSpeedPct(bool apply, uint8 mode);
        void HandleModCombatSpeedPct(bool apply, uint8 mode);
        void HandleModThreat(bool Apply, uint8 mode);
        void HandleModTaunt(bool Apply, uint8 mode);
        void HandleFeignDeath(bool Apply, uint8 mode);
        void HandleAuraModDisarm(bool Apply, uint8 mode);
        void HandleAuraModStalked(bool Apply, uint8 mode);
        void HandleAuraWaterWalk(bool Apply, uint8 mode);
        void HandleAuraFeatherFall(bool Apply, uint8 mode);
        void HandleAuraHover(bool Apply, uint8 mode);
        void HandleAuraModStun(bool Apply, uint8 mode);
        void HandleModDamageDone(bool Apply, uint8 mode);
        void HandleAuraUntrackable(bool Apply, uint8 mode);
        void HandleAuraEmpathy(bool Apply, uint8 mode);
        void HandleModOffhandDamagePercent(bool apply, uint8 mode);
        void HandleAuraModRangedAttackPower(bool Apply, uint8 mode);
        void HandleAuraModIncreaseEnergyPercent(bool Apply, uint8 mode);
        void HandleAuraModIncreaseHealthPercent(bool Apply, uint8 mode);
        void HandleAuraModRegenInterrupt(bool Apply, uint8 mode);
        void HandleHaste(bool Apply, uint8 mode);
        void HandlePeriodicTriggerSpell(bool Apply, uint8 mode);
        void HandlePeriodicEnergize(bool Apply, uint8 mode);
        void HandleAuraModResistanceExclusive(bool Apply, uint8 mode);
        void HandleModStealth(bool Apply, uint8 mode);
		void HandleModStealthDetect(bool Apply, uint8 mode);
        void HandleModInvisibility(bool Apply, uint8 mode);
        void HandleModInvisibilityDetect(bool Apply, uint8 mode);
        void HandleAuraModTotalHealthPercentRegen(bool Apply, uint8 mode);
        void HandleAuraModTotalPowerPercentRegen(bool Apply, uint8 mode);
        void HandleAuraModResistance(bool Apply, uint8 mode);
        void HandleAuraModRoot(bool Apply, uint8 mode);
        void HandleAuraModSilence(bool Apply, uint8 mode);
        void HandleAuraModStat(bool Apply, uint8 mode);
        void HandleAuraModIncreaseSpeed(bool Apply, uint8 mode);
        void HandleAuraModIncreaseMountedSpeed(bool Apply, uint8 mode);
        void HandleAuraModIncreaseFlightSpeed(bool Apply, uint8 mode);
        void HandleAuraModDecreaseSpeed(bool Apply, uint8 mode);
        void HandleAuraModUseNormalSpeed(bool Apply, uint8 mode);
        void HandleAuraModIncreaseHealth(bool Apply, uint8 mode);
        void HandleAuraModIncreaseEnergy(bool Apply, uint8 mode);
        void HandleAuraModShapeshift(bool Apply, uint8 mode);
        void HandleAuraModEffectImmunity(bool Apply, uint8 mode);
        void HandleAuraModStateImmunity(bool Apply, uint8 mode);
        void HandleAuraModSchoolImmunity(bool Apply, uint8 mode);
        void HandleAuraModDmgImmunity(bool Apply, uint8 mode);
        void HandleAuraModDispelImmunity(bool Apply, uint8 mode);
        void HandleAuraProcTriggerSpell(bool Apply, uint8 mode);
        void HandleAuraTrackCreatures(bool Apply, uint8 mode);
        void HandleAuraTrackResources(bool Apply, uint8 mode);
        void HandleAuraModParryPercent(bool Apply, uint8 mode);
        void HandleAuraModDodgePercent(bool Apply, uint8 mode);
        void HandleAuraModBlockPercent(bool Apply, uint8 mode);
        void HandleAuraModWeaponCritPercent(bool Apply, uint8 mode);
        void HandlePeriodicLeech(bool Apply, uint8 mode);
        void HandleModHitChance(bool Apply, uint8 mode);
        void HandleModSpellHitChance(bool Apply, uint8 mode);
        void HandleAuraModScale(bool Apply, uint8 mode);
        void HandlePeriodicManaLeech(bool Apply, uint8 mode);
        void HandleModCastingSpeed(bool Apply, uint8 mode);
        void HandleAuraMounted(bool Apply, uint8 mode);
        void HandleWaterBreathing(bool Apply, uint8 mode);
        void HandleModBaseResistance(bool Apply, uint8 mode);
        void HandleModRegen(bool Apply, uint8 mode);
        void HandleModPowerRegen(bool Apply, uint8 mode);
        void HandleModPowerRegenPCT(bool Apply, uint8 mode);
        void HandleChannelDeathItem(bool Apply, uint8 mode);
        void HandlePeriodicDamagePCT(bool Apply, uint8 mode);
        void HandleAuraModAttackPower(bool Apply, uint8 mode);
        void HandleAuraTransform(bool Apply, uint8 mode);
        void HandleModSpellCritChance(bool Apply, uint8 mode);
        void HandleAuraModIncreaseSwimSpeed(bool Apply, uint8 mode);
        void HandleModPowerCostPCT(bool Apply, uint8 mode);
        void HandleModPowerCost(bool Apply, uint8 mode);
        void HandleModPossessPet(bool Apply, uint8 mode);
        void HandleModMechanicImmunity(bool Apply, uint8 mode);
        void HandleAuraModSkill(bool Apply, uint8 mode);
        void HandleModDamagePercentDone(bool Apply, uint8 mode);
        void HandleModPercentStat(bool Apply, uint8 mode);
        void HandleModResistancePercent(bool Apply, uint8 mode);
        void HandleAuraModBaseResistancePCT(bool Apply, uint8 mode);
        void HandleAuraTrackStealthed(bool Apply, uint8 mode);
        void HandleModStealthLevel(bool Apply, uint8 mode);
        void HandleForceReaction(bool Apply, uint8 mode);
        void HandleAuraModRangedHaste(bool Apply, uint8 mode);
        void HandleRangedAmmoHaste(bool Apply, uint8 mode);
        void HandleModHealingDone(bool Apply, uint8 mode);
        void HandleModTotalPercentStat(bool Apply, uint8 mode);
        void HandleAuraModTotalThreat(bool Apply, uint8 mode);
        void HandleModUnattackable(bool Apply, uint8 mode);
        void HandleAuraModPacify(bool Apply, uint8 mode);
        void HandleAuraGhost(bool Apply, uint8 mode);
        void HandleAuraAllowFlight(bool Apply, uint8 mode);
        void HandleModRating(bool apply, uint8 mode);
        void HandleModTargetResistance(bool apply, uint8 mode);
        void HandleAuraModAttackPowerPercent(bool apply, uint8 mode);
        void HandleAuraModRangedAttackPowerPercent(bool apply, uint8 mode);
        void HandleAuraModRangedAttackPowerOfStatPercent(bool apply, uint8 mode);
        void HandleSpiritOfRedemption(bool apply, uint8 mode);
        void HandleModManaRegen(bool apply, uint8 mode);
        void HandleComprehendLanguage(bool apply, uint8 mode);
        void HandleShieldBlockValue(bool apply, uint8 mode);
        void HandleShieldBlockValuePercent(bool apply, uint8 mode);
        void HandleModSpellCritChanceShool(bool apply, uint8 mode);
        void HandleAuraRetainComboPoints(bool apply, uint8 mode);
        void HandleModSpellDamagePercentFromStat(bool apply, uint8 mode);
        void HandleModSpellHealingPercentFromStat(bool apply, uint8 mode);
        void HandleModSpellDamagePercentFromAttackPower(bool apply, uint8 mode);
        void HandleModSpellHealingPercentFromAttackPower(bool apply, uint8 mode);
        void HandleAuraModPacifyAndSilence(bool Apply, uint8 mode);
        void HandleAuraModIncreaseMaxHealth(bool apply, uint8 mode);
        void HandleAuraModExpertise(bool apply, uint8 mode);
        void HandleForceMoveForward(bool apply, uint8 mode);
        void HandleAuraModResistenceOfStatPercent(bool apply, uint8 mode);
        void HandleAuraPowerBurn(bool apply, uint8 mode);
        void HandleSchoolAbsorb(bool apply, uint8 mode);
        void HandlePreventFleeing(bool apply, uint8 mode);
        void HandleManaShield(bool apply, uint8 mode);
        void HandleArenaPreparation(bool apply, uint8 mode);
        void HandleAttackerPowerBonus(bool apply, uint8 mode);
        void HandleAOECharm(bool apply, uint8 mode);
        void HandlePeriodicTriggerSpellWithValue(bool apply, uint8 mode);
        void HandleModDetaunt(bool apply, uint8 mode);
        void HandleModStateImmunityMask(bool apply, uint8 mode);
        void HandleAuraCloneCaster(bool apply, uint8 mode);

        //custom on sunstrider, should propably be replaced with custom dummy spells
        void HandleAuraApplyExtraFlag(bool apply, uint8 mode);
        void HandleAuraImmunityId(bool apply, uint8 mode);

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

        SpellInfo const* GetSpellInfo() const { return m_spellInfo; }
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
        bool IsDeathPersistent() const { return m_isDeathPersist; }
        bool IsRemovedOnShapeLost() const { return m_isRemovedOnShapeLost; }
        bool IsRemoved() const { return m_isRemoved; }
        bool IsInUse() const { return m_in_use;}
        void CleanupTriggeredSpells();

        bool CanBeSaved() const;

        virtual void Update(uint32 diff);
        void HandleEffect(bool apply, uint8 mode = AURA_EFFECT_HANDLE_DEFAULT);

        void CalculateSpellMod();
        void ApplySpellMod(Unit* target, bool apply);

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

        // AuraScript
        void LoadScripts();
        bool CallScriptEffectApplyHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
        bool CallScriptEffectRemoveHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
        void CallScriptAfterEffectApplyHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
        void CallScriptAfterEffectRemoveHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);

        template <class Script>
        Script* GetScript(std::string const& scriptName) const
        {
            return dynamic_cast<Script*>(GetScriptByName(scriptName));
        }

        std::vector<AuraScript*> m_loadedScripts;
    private:
        AuraScript * GetScriptByName(std::string const& scriptName) const;

    protected:
        Aura(SpellInfo const* spellproto, uint8 eff, int32 *currentBasePoints, Unit *target, Unit *caster = nullptr, Item* castItem = nullptr);

        Modifier m_modifier;
        SpellModifier *m_spellmod;
        uint8 m_effIndex;
        SpellInfo const *m_spellInfo;
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
        bool m_in_use:1;                                    // true while in Aura::HandleEffect call
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

