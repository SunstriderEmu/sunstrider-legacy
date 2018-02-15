#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "templates.h"


// Serpent-Coil Braid item (30720)
// 37447 - Improved Mana Gems	
// "You gain 25 % more mana when you use a mana gem.In addition, using a mana gem grants you 5097 spell power for 15 sec."
class spell_item_improved_mana_gems : public SpellScriptLoader
{
public:
    enum Spells
    {
        SPELL_MANA_SURGE = 37445,
    };

    spell_item_improved_mana_gems() : SpellScriptLoader("spell_item_improved_mana_gems") { }

    class spell_item_improved_mana_gems_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_improved_mana_gems_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_MANA_SURGE });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();
            GetTarget()->CastSpell(GetTarget(), SPELL_MANA_SURGE, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_item_improved_mana_gems_AuraScript::HandleProc, EFFECT_1, SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_item_improved_mana_gems_AuraScript();
    }
};

enum PetHealing
{
    SPELL_HEALTH_LINK = 37382
};

// 37381 - Pet Healing
// Hunter T5 2P Bonus
// Warlock T5 2P Bonus
class spell_item_pet_healing : public AuraScript
{
    PrepareAuraScript(spell_item_pet_healing);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HEALTH_LINK });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        Unit* actor = eventInfo.GetActor();
        Unit* target = actor->GetPet();
        if (!target)
            return;

        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(CalculatePct(damageInfo->GetDamage(), aurEff->GetAmount()));
        eventInfo.GetActor()->CastSpell(target, SPELL_HEALTH_LINK, args);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_pet_healing::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

enum FrozenShadoweave
{
    SPELL_SHADOWMEND = 39373
};

// 39372 - Frozen Shadoweave
// Frozen Shadoweave set 3p bonus
class spell_item_frozen_shadoweave : public AuraScript
{
    PrepareAuraScript(spell_item_frozen_shadoweave);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHADOWMEND });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        Unit* caster = eventInfo.GetActor();
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(CalculatePct(damageInfo->GetDamage(), aurEff->GetAmount()));
        caster->CastSpell(nullptr, SPELL_SHADOWMEND, args);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_frozen_shadoweave::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// Item 23004 - Idol of Longevity
// 28847 - Healing Touch Refund
enum IdolOfLongevity
{
    SPELL_HEALING_TOUCH_MANA = 28848
};

class spell_item_healing_touch_refund : public AuraScript
{
    PrepareAuraScript(spell_item_healing_touch_refund);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HEALING_TOUCH_MANA });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        eventInfo.GetActor()->CastSpell(nullptr, SPELL_HEALING_TOUCH_MANA, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_healing_touch_refund::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

enum ZandalarianCharms
{
    SPELL_UNSTABLE_POWER_AURA_STACK = 24659,
    SPELL_RESTLESS_STRENGTH_AURA_STACK = 24662
};

// Item - 19950: Zandalarian Hero Charm
// 24658 - Unstable Power

// Item - 19949: Zandalarian Hero Medallion
// 24661 - Restless Strength
class spell_item_zandalarian_charm : public SpellScriptLoader
{
public:
    spell_item_zandalarian_charm(char const* ScriptName, uint32 SpellId, bool caster) : SpellScriptLoader(ScriptName), _spellId(SpellId), _caster(caster) { }

    class spell_item_zandalarian_charm_AuraScript : public AuraScript
    {
        friend class spell_item_zandalarian_charm;
        spell_item_zandalarian_charm_AuraScript(uint32 SpellId, bool caster) : AuraScript(), _spellId(SpellId), _caster(caster) { }

        PrepareAuraScript(spell_item_zandalarian_charm_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ _spellId });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (_caster)
            {
                if (SpellInfo const* spellInfo = eventInfo.GetSpellInfo())
                    if (spellInfo->Id != m_scriptSpellId)
                        return true;

                return false;
            }
            return true;
        }

        void HandleStackDrop(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();
            GetTarget()->RemoveAuraFromStack(_spellId);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_item_zandalarian_charm_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_item_zandalarian_charm_AuraScript::HandleStackDrop, EFFECT_0, SPELL_AURA_DUMMY);
        }

        uint32 _spellId;
        bool _caster;
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_item_zandalarian_charm_AuraScript(_spellId, _caster);
    }

private:
    uint32 _spellId;
    bool _caster;
};

enum AuraOfMadness
{
    SPELL_SOCIOPATH         = 39511, // Sociopath: +35 strength(Paladin, Rogue, Druid, Warrior)
    SPELL_DELUSIONAL        = 40997, // Delusional: +70 attack power(Rogue, Hunter, Paladin, Warrior, Druid)
    SPELL_KLEPTOMANIA       = 40998, // Kleptomania: +35 agility(Warrior, Rogue, Paladin, Hunter, Druid)
    SPELL_MEGALOMANIA       = 40999, // Megalomania: +41 damage / healing(Druid, Shaman, Priest, Warlock, Mage, Paladin)
    SPELL_PARANOIA          = 41002, // Paranoia: +35 spell / melee / ranged crit strike rating(All classes)
    SPELL_MANIC             = 41005, // Manic: +35 haste(spell, melee and ranged) (All classes)
    SPELL_NARCISSISM        = 41009, // Narcissism: +35 intellect(Druid, Shaman, Priest, Warlock, Mage, Paladin, Hunter)
    SPELL_MARTYR_COMPLEX    = 41011, // Martyr Complex: +35 stamina(All classes)
    SPELL_DEMENTIA          = 41404, // Dementia: Every 5 seconds either gives you +5/-5%  damage/healing. (Druid, Shaman, Priest, Warlock, Mage, Paladin)

    SPELL_DEMENTIA_POS      = 41406,
    SPELL_DEMENTIA_NEG      = 41409,

    SAY_MADNESS             = 21954
};

// Item - 31859: Darkmoon Card: Madness
// 39446 - Aura of Madness
class spell_item_aura_of_madness : public AuraScript
{
    PrepareAuraScript(spell_item_aura_of_madness);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_SOCIOPATH,
            SPELL_DELUSIONAL,
            SPELL_KLEPTOMANIA,
            SPELL_MEGALOMANIA,
            SPELL_PARANOIA,
            SPELL_MANIC,
            SPELL_NARCISSISM,
            SPELL_MARTYR_COMPLEX,
            SPELL_DEMENTIA
        }) && sObjectMgr->GetBroadcastText(SAY_MADNESS);
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        static std::vector<uint32> const triggeredSpells[MAX_CLASSES] =
        {
            //CLASS_NONE
            { },
            //CLASS_WARRIOR
            { SPELL_SOCIOPATH, SPELL_DELUSIONAL, SPELL_KLEPTOMANIA, SPELL_PARANOIA, SPELL_MANIC, SPELL_MARTYR_COMPLEX },
            //CLASS_PALADIN
            { SPELL_SOCIOPATH, SPELL_DELUSIONAL, SPELL_KLEPTOMANIA, SPELL_MEGALOMANIA, SPELL_PARANOIA, SPELL_MANIC, SPELL_NARCISSISM, SPELL_MARTYR_COMPLEX, SPELL_DEMENTIA },
            //CLASS_HUNTER
            { SPELL_DELUSIONAL, SPELL_MEGALOMANIA, SPELL_PARANOIA, SPELL_MANIC, SPELL_NARCISSISM, SPELL_MARTYR_COMPLEX, SPELL_DEMENTIA },
            //CLASS_ROGUE
            { SPELL_SOCIOPATH, SPELL_DELUSIONAL, SPELL_KLEPTOMANIA, SPELL_PARANOIA, SPELL_MANIC, SPELL_MARTYR_COMPLEX },
            //CLASS_PRIEST
            { SPELL_MEGALOMANIA, SPELL_PARANOIA, SPELL_MANIC, SPELL_NARCISSISM, SPELL_MARTYR_COMPLEX, SPELL_DEMENTIA },
            //CLASS_DEATH_KNIGHT
            { SPELL_SOCIOPATH, SPELL_DELUSIONAL, SPELL_KLEPTOMANIA, SPELL_PARANOIA, SPELL_MANIC, SPELL_MARTYR_COMPLEX },
            //CLASS_SHAMAN
            { SPELL_MEGALOMANIA, SPELL_PARANOIA, SPELL_MANIC, SPELL_NARCISSISM, SPELL_MARTYR_COMPLEX, SPELL_DEMENTIA },
            //CLASS_MAGE
            { SPELL_MEGALOMANIA, SPELL_PARANOIA, SPELL_MANIC, SPELL_NARCISSISM, SPELL_MARTYR_COMPLEX, SPELL_DEMENTIA },
            //CLASS_WARLOCK
            { SPELL_MEGALOMANIA, SPELL_PARANOIA, SPELL_MANIC, SPELL_NARCISSISM, SPELL_MARTYR_COMPLEX, SPELL_DEMENTIA },
            //CLASS_UNK
            { },
            //CLASS_DRUID
            { SPELL_SOCIOPATH, SPELL_DELUSIONAL, SPELL_KLEPTOMANIA, SPELL_MEGALOMANIA, SPELL_PARANOIA, SPELL_MANIC, SPELL_NARCISSISM, SPELL_MARTYR_COMPLEX, SPELL_DEMENTIA }
        };

        PreventDefaultAction();
        Unit* caster = eventInfo.GetActor();
        uint32 spellId = Trinity::Containers::SelectRandomContainerElement(triggeredSpells[caster->GetClass()]);
        caster->CastSpell(caster, spellId, aurEff);

        if (roll_chance_i(10))
            caster->Unit::Say(SAY_MADNESS);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_aura_of_madness::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

enum ExaltedSunwellNeck
{
    SPELL_LIGHTS_WRATH      = 45479, // Light's Wrath if Exalted by Aldor
    SPELL_ARCANE_BOLT       = 45429, // Arcane Bolt if Exalted by Scryers

    SPELL_LIGHTS_STRENGTH   = 45480, // Light's Strength if Exalted by Aldor
    SPELL_ARCANE_STRIKE     = 45428, // Arcane Strike if Exalted by Scryers

    SPELL_LIGHTS_WARD       = 45432, // Light's Ward if Exalted by Aldor
    SPELL_ARCANE_INSIGHT    = 45431, // Arcane Insight if Exalted by Scryers

    SPELL_LIGHTS_SALVATION  = 45478, // Light's Salvation if Exalted by Aldor
    SPELL_ARCANE_SURGE      = 45430, // Arcane Surge if Exalted by Scryers

    FACTION_ALDOR           = 932,
    FACTION_SCRYERS         = 934
};

// Item - 34678: Shattered Sun Pendant of Acumen
// 45481 - Sunwell Exalted Caster Neck

// Item - 34679: Shattered Sun Pendant of Might
// 45482 - Sunwell Exalted Melee Neck

// Item - 34680: Shattered Sun Pendant of Resolve
// 45483 - Sunwell Exalted Tank Neck

// Item - 34677: Shattered Sun Pendant of Restoration
// 45484 Sunwell Exalted Healer Neck
template <uint32 AldorSpellId, uint32 ScryersSpellId>
class spell_item_sunwell_neck : public SpellScriptLoader
{
public:
    spell_item_sunwell_neck(char const* ScriptName) : SpellScriptLoader(ScriptName) { }

    template <uint32 Aldors, uint32 Scryers>
    class spell_item_sunwell_neck_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_sunwell_neck_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ Aldors, Scryers }) &&
                sFactionStore.LookupEntry(FACTION_ALDOR) &&
                sFactionStore.LookupEntry(FACTION_SCRYERS);
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetActor()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Player* player = eventInfo.GetActor()->ToPlayer();
            Unit* target = eventInfo.GetProcTarget();

            // Aggression checks are in the spell system... just cast and forget
            if (player->GetReputationRank(FACTION_ALDOR) == REP_EXALTED)
                player->CastSpell(target, Aldors, aurEff);

            if (player->GetReputationRank(FACTION_SCRYERS) == REP_EXALTED)
                player->CastSpell(target, Scryers, aurEff);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_item_sunwell_neck_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_item_sunwell_neck_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_item_sunwell_neck_AuraScript<AldorSpellId, ScryersSpellId>();
    }
};

// 43820 - Amani Charm of the Witch Doctor
enum CharmWitchDoctor
{
    SPELL_CHARM_WITCH_DOCTOR_PROC = 43821
};

class spell_item_charm_witch_doctor : public AuraScript
{
    PrepareAuraScript(spell_item_charm_witch_doctor);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CHARM_WITCH_DOCTOR_PROC });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        if (Unit* target = eventInfo.GetActionTarget())
        {
            int32 bp = CalculatePct(target->GetCreateHealth(), aurEff->GetSpellInfo()->Effects[1].CalcValue());
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(bp);
            eventInfo.GetActor()->CastSpell(target, SPELL_CHARM_WITCH_DOCTOR_PROC, args);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_charm_witch_doctor::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 27522,40336 - Mana Drain
enum ManaDrainSpells
{
    SPELL_MANA_DRAIN_ENERGIZE = 29471,
    SPELL_MANA_DRAIN_LEECH    = 27526
};

class spell_item_mana_drain : public AuraScript
{
    PrepareAuraScript(spell_item_mana_drain);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_MANA_DRAIN_ENERGIZE,
            SPELL_MANA_DRAIN_LEECH
        });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetActionTarget();

        if (target && target->IsAlive() && target->GetPowerType() == POWER_MANA)
            caster->CastSpell(target, SPELL_MANA_DRAIN_LEECH, aurEff);
        else
            caster->CastSpell(caster, SPELL_MANA_DRAIN_ENERGIZE, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_mana_drain::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

enum AlchemistStone
{
    SPELL_ALCHEMISTS_STONE_EXTRA_HEAL       = 21399,
    SPELL_ALCHEMISTS_STONE_EXTRA_MANA       = 21400
};

// Item - 13503: Alchemist's Stone
// Item - 35748: Guardian's Alchemist Stone
// Item - 35749: Sorcerer's Alchemist Stone
// Item - 35750: Redeemer's Alchemist Stone
// Item - 35751: Assassin's Alchemist Stone
// Item - 44322: Mercurial Alchemist Stone
// Item - 44323: Indestructible Alchemist's Stone
// Item - 44324: Mighty Alchemist's Stone

// 17619 - Alchemist's Stone
class spell_item_alchemists_stone : public AuraScript
{
    PrepareAuraScript(spell_item_alchemists_stone);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_ALCHEMISTS_STONE_EXTRA_HEAL,
            SPELL_ALCHEMISTS_STONE_EXTRA_MANA
        });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return;

        Unit* caster = eventInfo.GetActionTarget();
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            uint32 spellId;
            switch (spellInfo->Effects[i].Effect)
            {
                case SPELL_EFFECT_HEAL:
                    spellId = SPELL_ALCHEMISTS_STONE_EXTRA_HEAL;
                    break;
                case SPELL_EFFECT_ENERGIZE:
                    spellId = SPELL_ALCHEMISTS_STONE_EXTRA_MANA;
                    break;
                default:
                    continue;
            }

            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(CalculatePct(spellInfo->Effects[i].CalcValue(caster), 40));
            caster->CastSpell(nullptr, spellId, args);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_alchemists_stone::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

void AddSC_item_spell_scripts()
{
    new spell_item_improved_mana_gems();
    new spell_gen_proc_below_pct_damaged("spell_item_commendation_of_kaelthas");
    RegisterAuraScript(spell_item_pet_healing);
    RegisterAuraScript(spell_item_frozen_shadoweave);
    RegisterAuraScript(spell_item_healing_touch_refund);
    new spell_item_zandalarian_charm("spell_item_unstable_power", SPELL_UNSTABLE_POWER_AURA_STACK, true);
    new spell_item_zandalarian_charm("spell_item_restless_strength", SPELL_RESTLESS_STRENGTH_AURA_STACK, false);
    RegisterAuraScript(spell_item_aura_of_madness);
    new spell_item_sunwell_neck<SPELL_LIGHTS_WRATH, SPELL_ARCANE_BOLT>("spell_item_sunwell_exalted_caster_neck");
    new spell_item_sunwell_neck<SPELL_LIGHTS_STRENGTH, SPELL_ARCANE_STRIKE>("spell_item_sunwell_exalted_melee_neck");
    new spell_item_sunwell_neck<SPELL_LIGHTS_WARD, SPELL_ARCANE_INSIGHT>("spell_item_sunwell_exalted_tank_neck");
    new spell_item_sunwell_neck<SPELL_LIGHTS_SALVATION, SPELL_ARCANE_SURGE>("spell_item_sunwell_exalted_healer_neck");
    RegisterAuraScript(spell_item_charm_witch_doctor);
    RegisterAuraScript(spell_item_mana_drain);
    RegisterAuraScript(spell_item_alchemists_stone);
}
