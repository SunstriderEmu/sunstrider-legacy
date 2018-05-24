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

class Unit;
class SpellInfo;
struct SpellModifier;
struct ProcTriggerSpell;
class AuraScript;
class ChargeDropEvent;
class Aura;
class DynObjAura;
struct ChannelTargetData;

// update aura target map every 500 ms instead of every update - reduce amount of grid searcher calls
#define UPDATE_TARGET_MAP_INTERVAL 500

//Aura applied to an specific unit
class TC_GAME_API AuraApplication
{
    friend class Unit;
    friend AuraApplication * Unit::_CreateAuraApplication(Aura* aura, uint8 effMask);
private:
    Unit * const _target;
    Aura* const _base;
    AuraRemoveMode _removeMode : 8;                // Store info for know remove aura reason
    uint8 _slot;                                   // Aura slot on unit
    uint8 _flags;                                  // Aura info flag. /!\ Does not contains the same value in TC
    bool _positive; //replaces AFLAG_POSITIVE logic from TC
    bool _selfCast; //replaces AFLAG_CASTER logic from TC
    uint8 _effectMask; //replaces AFLAG_EFF_INDEX_* logic from TC
    uint8 _effectsToApply;                         // Used only at spell hit to determine which effect should be applied
    bool _needClientUpdate : 1;
    bool _durationChanged : 1; //sun: added to avoid sending uncessary data at each change

    explicit AuraApplication(Unit* target, Unit* caster, Aura* base, uint8 effMask);
    void _Remove();

    void _InitFlags(Unit* caster, uint8 effMask);
    //sun: moved slot selection logic in here since we sometimes have to reuse later on BC
    void _UpdateSlot();
    void _HandleEffect(uint8 effIndex, bool apply);

public:
    Unit * GetTarget() const { return _target; }
    Aura* GetBase() const { return _base; }

    uint8 GetSlot() const { return _slot; }
    uint8 GetFlags() const { return _flags; }
    //TC uint8 GetEffectMask() const { return _flags & (AFLAG_EFF_INDEX_0 | AFLAG_EFF_INDEX_1 | AFLAG_EFF_INDEX_2); }
    uint8 GetEffectMask() const { return _effectMask; }
    //TC bool HasEffect(uint8 effect) const { ASSERT(effect < MAX_SPELL_EFFECTS); return (_flags & (1 << effect)) != 0; }
    bool HasEffect(uint8 effect) const { ASSERT(effect < MAX_SPELL_EFFECTS); return (_effectMask & (1 << effect)) != 0; }
    bool IsPositive() const { return _positive; }
    bool IsSelfcast() const { return _selfCast; }

    uint8 GetEffectsToApply() const { return _effectsToApply; }
    void UpdateApplyEffectMask(uint8 newEffMask);

    void SetRemoveMode(AuraRemoveMode mode) { _removeMode = mode; }
    AuraRemoveMode GetRemoveMode() const { return _removeMode; }

    //Set for duration change, charges change, stack change
    void SetNeedClientUpdate() { _needClientUpdate = true; }
    void SetDurationChanged() { _durationChanged = true; }
    bool IsNeedClientUpdate() const { return _needClientUpdate; }
    void BuildUpdatePacket(ByteBuffer& data, bool remove) const;
    void ClientUpdate(bool remove = false);

private:
    //BC specific functions
    //update aura list at client (field UNIT_FIELD_AURA)
    void SetAura(bool remove);
    //update aura flag (UNIT_FIELD_AURAFLAGS field)
    void SetAuraFlag(bool remove);
    //update aura level (UNIT_FIELD_AURALEVELS field)
    void SetAuraLevel(bool remove, uint32 level);
    //update charges/stacks (UNIT_FIELD_AURAAPPLICATIONS field)
    void UpdateAuraCharges(bool remove);
    void SetAuraApplication(bool remove, int8 count); 

    //Send duration for caster and his group
    void UpdateAuraDuration();
public:
    void SendAuraDurationForCaster(Player* caster);
};

// Caches some information about caster (because it may no longer exist)
struct CasterInfo
{
    float CritChance = 0.f;
    float BonusDonePct = 0.f;
    uint8 Level = 0;
    bool  ApplyResilience = false;
};

class TC_GAME_API Aura
{
    friend class Unit;

public:
    typedef std::unordered_map<ObjectGuid, AuraApplication*> ApplicationMap;

    static uint8 BuildEffectMaskForOwner(SpellInfo const* spellProto, uint8 availableEffectMask, WorldObject* owner);
    static Aura* TryRefreshStackOrCreate(AuraCreateInfo& createInfo);
    static Aura* TryCreate(AuraCreateInfo& createInfo);
    static Aura* Create(AuraCreateInfo& createInfo);
    explicit Aura(AuraCreateInfo const& createInfo);
    void _InitEffects(uint8 effMask, Unit* caster, int32 const* baseAmount);
    void SaveCasterInfo(Unit* caster);
    virtual ~Aura();

    SpellInfo const* GetSpellInfo() const { return m_spellInfo; }
    uint32 GetId() const;
    ObjectGuid GetCastItemGUID() const { return m_castItemGuid; }

    int32 GetMaxDuration() const { return m_maxDuration; }
    void SetMaxDuration(int32 duration) { m_maxDuration = duration; }
    int32 CalcMaxDuration() const { return CalcMaxDuration(GetCaster()); }
    int32 CalcMaxDuration(Unit* caster) const { return Aura::CalcMaxDuration(GetSpellInfo(), caster); }
    static int32 CalcMaxDuration(SpellInfo const* spellInfo, WorldObject* caster);
    int32 GetDuration() const { return m_duration; }
    void SetDuration(int32 duration, bool withMods = false);
    void RefreshDuration(bool withMods = false);
    void RefreshTimers(bool resetPeriodicTimer);
    time_t GetApplyTime() const { return m_applyTime; }

    bool IsActive() { return m_active; }
    bool IsExpired() const { return !GetDuration() && !m_dropEvent; }

    ObjectGuid const& GetCasterGUID() const { return m_casterGuid; }
    Unit* GetCaster() const;
    WorldObject* GetOwner() const { return m_owner; }
    Unit* GetUnitOwner() const { ASSERT(GetType() == UNIT_AURA_TYPE); return m_owner->ToUnit(); }
    DynamicObject* GetDynobjOwner() const { ASSERT(GetType() == DYNOBJ_AURA_TYPE); return m_owner->ToDynObject(); }

    void SetLoadedState(int32 maxduration, int32 duration, int32 charges, uint8 stackamount, uint8 recalculateMask, float critChance, bool applyResilience, int32* amount);

    AuraObjectType GetType() const;

    virtual void _ApplyForTarget(Unit* target, Unit* caster, AuraApplication * auraApp);
    virtual void _UnapplyForTarget(Unit* target, Unit* caster, AuraApplication * auraApp);
    void _Remove(AuraRemoveMode removeMode);
    virtual void Remove(AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT) = 0;

    virtual void FillTargetMap(std::unordered_map<Unit*, uint8>& targets, Unit* caster) = 0;
    void UpdateTargetMap(Unit* caster, bool apply = true);

    void _RegisterForTargets() { Unit* caster = GetCaster(); UpdateTargetMap(caster, false); }
    void ApplyForTargets() { Unit* caster = GetCaster(); UpdateTargetMap(caster, true); }
    void _ApplyEffectForTargets(uint8 effIndex);

    void UpdateOwner(uint32 diff, WorldObject* owner);
    void Update(uint32 diff, Unit* caster);

    //compat TC
    float GetCritChance() const { return 0.0f; }
    //compat TC
    bool  CanApplyResilience() const { return false; }

    bool HasMoreThanOneEffectForType(AuraType auraType) const;
    bool IsPermanent() const { return GetMaxDuration() == -1; }
    bool IsArea() const { return m_isAreaAura; }
    bool IsPeriodic() const { return m_isPeriodic; }
    bool IsPassive() const;
    bool IsPersistent() const { return m_isPersistent; }
    bool IsDeathPersistent() const;
    bool IsRemoved() const { return m_isRemoved; }

    bool IsRemovedOnShapeLost(Unit* target) const
    {
        return GetCasterGUID() == target->GetGUID()
            && m_spellInfo->Stances
            && !m_spellInfo->HasAttribute(SPELL_ATTR2_NOT_NEED_SHAPESHIFT)
            && !m_spellInfo->HasAttribute(SPELL_ATTR0_NOT_SHAPESHIFT);
    }

    bool CanBeSaved() const;
    bool CanBeSentToClient() const;
    // Single cast aura helpers
    bool IsSingleTarget() const { return m_isSingleTarget; }
    bool IsSingleTargetWith(Aura const* aura) const;
    void SetIsSingleTarget(bool val) { m_isSingleTarget = val; }
    void UnregisterSingleTarget();
    int32 CalcDispelChance(Unit const* auraTarget, bool offensive) const;

    int32 m_procCharges;
    void SetCharges(uint8 charges);
    int32 GetCharges() const { return m_procCharges; }
    uint8 CalcMaxCharges(Unit* caster) const;
    uint8 CalcMaxCharges() const { return CalcMaxCharges(GetCaster()); }
    bool ModCharges(int32 num, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
    bool DropCharge(AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT) { return ModCharges(-1, removeMode); }
    void ModChargesDelayed(int32 num, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
    void DropChargeDelayed(uint32 delay, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);

    // Allow Apply Aura Handler to modify and access m_AuraDRGroup
    void setDiminishGroup(DiminishingGroup group) { m_AuraDRGroup = group; }
    DiminishingGroup getDiminishGroup() const { return m_AuraDRGroup; }
        
    int32 GetStackAmount() const {return m_stackAmount;}
    void SetStackAmount(int32 amount);
    bool ModStackAmount(int32 num, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT, bool resetPeriodicTimer = true);

    uint8 GetCasterLevel() const { return _casterInfo.Level; }
    float GetDonePct() const { return _casterInfo.BonusDonePct; }
    void  SetDonePct(float val) { _casterInfo.BonusDonePct = val; }

    // Single cast aura helpers
    bool DoesAuraApplyAuraName(uint32 name);

    bool HasEffect(uint8 effIndex) const { return GetEffect(effIndex) != nullptr; }
    bool HasEffectType(AuraType type) const;
    AuraEffect* GetEffect(uint8 effIndex) const { ASSERT(effIndex < MAX_SPELL_EFFECTS); return m_effects[effIndex]; }
    uint8 GetEffectMask() const { uint8 effMask = 0; for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i) if (m_effects[i]) effMask |= 1 << i; return effMask; }
    void RecalculateAmountOfEffects();
    void HandleAllEffects(AuraApplication * aurApp, uint8 mode, bool apply);

    // Helpers for targets
    ApplicationMap const& GetApplicationMap() { return m_applications; }
    void GetApplicationVector(std::vector<AuraApplication*>& applicationList) const;
    AuraApplication const* GetApplicationOfTarget(ObjectGuid guid) const { ApplicationMap::const_iterator itr = m_applications.find(guid); if (itr != m_applications.end()) return itr->second; return nullptr; }
    AuraApplication* GetApplicationOfTarget(ObjectGuid guid) { ApplicationMap::iterator itr = m_applications.find(guid); if (itr != m_applications.end()) return itr->second; return nullptr; }
    bool IsAppliedOnTarget(ObjectGuid guid) const { return m_applications.find(guid) != m_applications.end(); }

    void SetNeedClientUpdateForTargets() const;
    void HandleAuraSpecificMods(AuraApplication const* aurApp, Unit* caster, bool apply, bool onReapply);
    bool CanBeAppliedOn(Unit* target);
    bool CheckAreaTarget(Unit* target);
    bool CanStackWith(Aura const* existingAura) const;


    bool IsProcOnCooldown(std::chrono::steady_clock::time_point now) const;
    void AddProcCooldown(std::chrono::steady_clock::time_point cooldownEnd);
    void PrepareProcToTrigger(AuraApplication* aurApp, ProcEventInfo& eventInfo, std::chrono::steady_clock::time_point now);
    uint8 GetProcEffectMask(AuraApplication* aurApp, ProcEventInfo& eventInfo, std::chrono::steady_clock::time_point now) const;
    float CalcProcChance(SpellProcEntry const& procEntry, ProcEventInfo& eventInfo) const;
    void TriggerProcOnEvent(uint8 procEffectMask, AuraApplication* aurApp, ProcEventInfo& eventInfo);
    void SetUsingCharges(bool val) { m_isUsingCharges = val; }
    bool IsUsingCharges() const { return m_isUsingCharges; }

    void HeartbeatResistance(uint32 diff, Unit* caster);

    // AuraScript
    void LoadScripts();
    bool CallScriptCheckAreaTargetHandlers(Unit* target);
    void CallScriptDispel(DispelInfo* dispelInfo);
    void CallScriptAfterDispel(DispelInfo* dispelInfo);
    bool CallScriptEffectApplyHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
    bool CallScriptEffectRemoveHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
    void CallScriptAfterEffectApplyHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
    void CallScriptAfterEffectRemoveHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
    bool CallScriptEffectPeriodicHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp);
    void CallScriptEffectUpdatePeriodicHandlers(AuraEffect* aurEff);
    void CallScriptEffectCalcAmountHandlers(AuraEffect const* aurEff, int32 & amount, bool & canBeRecalculated);
    void CallScriptEffectCalcPeriodicHandlers(AuraEffect const* aurEff, bool & isPeriodic, int32 & amplitude);
    void CallScriptEffectCalcSpellModHandlers(AuraEffect const* aurEff, SpellModifier* & spellMod);
    void CallScriptEffectAbsorbHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount, bool & defaultPrevented);
    void CallScriptEffectAfterAbsorbHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount);
    void CallScriptEffectManaShieldHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount, bool & defaultPrevented);
    void CallScriptEffectAfterManaShieldHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount);
    void CallScriptEffectSplitHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & splitAmount);

    // Spell Proc Hooks
    bool CallScriptCheckProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo);
    bool CallScriptCheckEffectProcHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, ProcEventInfo& eventInfo);
    bool CallScriptPrepareProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo);
    bool CallScriptProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo);
    void CallScriptAfterProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo);
    bool CallScriptEffectProcHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, ProcEventInfo& eventInfo);
    void CallScriptAfterEffectProcHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, ProcEventInfo& eventInfo);

    UnitAura* ToUnitAura() { if (GetType() == UNIT_AURA_TYPE) return reinterpret_cast<UnitAura*>(this); else return nullptr; }
    UnitAura const* ToUnitAura() const { if (GetType() == UNIT_AURA_TYPE) return reinterpret_cast<UnitAura const*>(this); else return nullptr; }

    DynObjAura* ToDynObjAura() { if (GetType() == DYNOBJ_AURA_TYPE) return reinterpret_cast<DynObjAura*>(this); else return nullptr; }
    DynObjAura const* ToDynObjAura() const { if (GetType() == DYNOBJ_AURA_TYPE) return reinterpret_cast<DynObjAura const*>(this); else return nullptr; }

    template <class Script>
    Script* GetScript(std::string const& scriptName) const
    {
        return dynamic_cast<Script*>(GetScriptByName(scriptName));
    }

    std::vector<AuraScript*> m_loadedScripts;

    ChannelTargetData const* GetChannelTargetData() const { return m_channelData; }
    TriggerCastFlags GetTriggerCastFlags() const { return m_castFlags; }

private:
    AuraScript * GetScriptByName(std::string const& scriptName) const;
    void _DeleteRemovedApplications();

protected:

    uint8 m_effIndex;
    SpellInfo const* const m_spellInfo;
    ObjectGuid const m_casterGuid;
    int32 m_maxDuration;
    CasterInfo _casterInfo;
    int32 m_duration;
    int32 m_timeCla;
    int32 m_updateTargetMapInterval;                        // Timer for UpdateTargetMapOfEffect
    ObjectGuid m_castItemGuid;                              // it is NOT safe to keep a pointer to the item because it may get deleted
    time_t m_applyTime;
    bool m_active;
    WorldObject* const m_owner;
    
    bool m_positive:1;
    bool m_isPeriodic:1;
    bool m_isAreaAura:1;
    bool m_isPersistent:1;
    bool m_isRemoved:1;
    bool m_updated:1;
    bool m_isSingleTarget:1;                            // true if it's a single target spell and registered at caster - can change at spell steal for example
    bool m_isUsingCharges : 1;

    //channel information for channel triggering
    ChannelTargetData* m_channelData;

    //int32 m_periodicTimer; //time until next tick
    int32 m_amplitude;
    uint32 m_PeriodicEventId;
    DiminishingGroup m_AuraDRGroup;

    int32 m_stackAmount;

    AuraEffect* m_effects[MAX_SPELL_EFFECTS];
    ApplicationMap m_applications;

    ChargeDropEvent* m_dropEvent;

    uint32 m_heartBeatTimer;                        // Heartbeat resist timer

    std::chrono::steady_clock::time_point m_procCooldown;

    TriggerCastFlags m_castFlags; //flags used by spell

private:
    std::vector<AuraApplication*> _removedApplications;
};

class TC_GAME_API UnitAura : public Aura
{
    friend Aura* Aura::Create(AuraCreateInfo& createInfo);
protected:
    explicit UnitAura(AuraCreateInfo const& createInfo);
public:
    void _ApplyForTarget(Unit* target, Unit* caster, AuraApplication* aurApp) override;
    void _UnapplyForTarget(Unit* target, Unit* caster, AuraApplication* aurApp) override;

    void Remove(AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT) override;

    void FillTargetMap(std::unordered_map<Unit*, uint8>& targets, Unit* caster) override;

    // Allow Apply Aura Handler to modify and access m_AuraDRGroup
    void SetDiminishGroup(DiminishingGroup group) { m_AuraDRGroup = group; }
    DiminishingGroup GetDiminishGroup() const { return m_AuraDRGroup; }

    void AddStaticApplication(Unit* target, uint8 effMask);

private:
    DiminishingGroup m_AuraDRGroup;               // Diminishing
    std::unordered_map<ObjectGuid, uint8> _staticApplications; // non-area auras
};

class TC_GAME_API DynObjAura : public Aura
{
    friend Aura* Aura::Create(AuraCreateInfo& createInfo);
protected:
    explicit DynObjAura(AuraCreateInfo const& createInfo);
public:
    void Remove(AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT) override;

    void FillTargetMap(std::unordered_map<Unit*, uint8>& targets, Unit* caster) override;
};

class TC_GAME_API ChargeDropEvent : public BasicEvent
{
    friend class Aura;
protected:
    ChargeDropEvent(Aura* base, AuraRemoveMode mode) : _base(base), _mode(mode) { }
    bool Execute(uint64 /*e_time*/, uint32 /*p_time*/) override;

private:
    Aura * _base;
    AuraRemoveMode _mode;
};
#endif

