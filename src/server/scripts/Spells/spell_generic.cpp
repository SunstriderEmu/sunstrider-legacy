#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "templates.h"

enum CannibalizeSpells
{
    SPELL_CANNIBALIZE_TRIGGERED = 20578
};

class spell_gen_cannibalize : public SpellScriptLoader
{
public:
    spell_gen_cannibalize() : SpellScriptLoader("spell_gen_cannibalize") { }

    class spell_gen_cannibalize_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gen_cannibalize_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_CANNIBALIZE_TRIGGERED))
                return false;
            return true;
        }

        SpellCastResult CheckIfCorpseNear()
        {
            Unit* caster = GetCaster();
            float max_range = GetSpellInfo()->GetMaxRange(false);
            WorldObject* result = nullptr;
            // search for nearby enemy corpse in range
            Trinity::CannibalizeObjectCheck u_check(caster, max_range);
            Trinity::WorldObjectSearcher<Trinity::CannibalizeObjectCheck > searcher(caster, result, u_check);
            Cell::VisitGridObjects(caster, searcher, max_range);
            if (!result)
                Cell::VisitWorldObjects(caster, searcher, max_range);

            if (!result)
                return SPELL_FAILED_NO_EDIBLE_CORPSES;
            return SPELL_CAST_OK;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/, int32& /*damage*/)
        {
            GetCaster()->CastSpell(GetCaster(), SPELL_CANNIBALIZE_TRIGGERED, TRIGGERED_NONE);
        }

        void Register() override
        {
            OnEffectHit += SpellEffectFn(spell_gen_cannibalize_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnCheckCast += SpellCheckCastFn(spell_gen_cannibalize_SpellScript::CheckIfCorpseNear);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_gen_cannibalize_SpellScript();
    }
};

// 20594 - Dwarf racial stoneform - Only armor and poison resistance are in aura effects on BC, need to add both bleed and disease
class spell_racial_stoneform : public SpellScriptLoader
{
public:
    spell_racial_stoneform() : SpellScriptLoader("spell_racial_stoneform") { }

    class spell_racial_stoneform_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_racial_stoneform_AuraScript)

        void HandleEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            GetSpellInfo()->ApplyDispelImmune(GetTarget(), DISPEL_DISEASE, true);
            GetSpellInfo()->ApplyMechanicImmune(GetTarget(), (1 << MECHANIC_BLEED), true);
        }

        void HandleEffectRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            GetSpellInfo()->ApplyDispelImmune(GetTarget(), DISPEL_DISEASE, false);
            GetSpellInfo()->ApplyMechanicImmune(GetTarget(), (1 << MECHANIC_BLEED), false);
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_racial_stoneform_AuraScript::HandleEffectApply, EFFECT_1, SPELL_AURA_DISPEL_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_racial_stoneform_AuraScript::HandleEffectRemove, EFFECT_1, SPELL_AURA_DISPEL_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_racial_stoneform_AuraScript();
    }
};

class spell_archimonde_doomfire : public SpellScriptLoader
{
public:
    spell_archimonde_doomfire() : SpellScriptLoader("spell_archimonde_doomfire") { }

    class spell_archimonde_doomfire_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_archimonde_doomfire_AuraScript)

        //MARCH PA + utiliser le tick count pour les degats + prendre les degats du spell parent
        void PeriodicTick(AuraEffect const* aurEff)
        {
            int32 dmg = aurEff->GetAmount() - aurEff->GetTickNumber() * 150;
            if (dmg < 0)
                return;
            CastSpellExtraArgs args(GetTarget()->GetGUID());
            args.AddSpellBP0(int32(dmg));
            GetTarget()->CastSpell(GetTarget(), 31969, args);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_archimonde_doomfire_AuraScript::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_archimonde_doomfire_AuraScript();
    }
};

// 28764 - Adaptive Warding (Frostfire Regalia Set)
enum AdaptiveWarding
{
    SPELL_GEN_ADAPTIVE_WARDING_FIRE     = 28765,
    SPELL_GEN_ADAPTIVE_WARDING_NATURE   = 28768,
    SPELL_GEN_ADAPTIVE_WARDING_FROST    = 28766,
    SPELL_GEN_ADAPTIVE_WARDING_SHADOW   = 28769,
    SPELL_GEN_ADAPTIVE_WARDING_ARCANE   = 28770
};

class spell_gen_adaptive_warding : public AuraScript
{
    PrepareAuraScript(spell_gen_adaptive_warding);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
            {
                SPELL_GEN_ADAPTIVE_WARDING_FIRE,
                SPELL_GEN_ADAPTIVE_WARDING_NATURE,
                SPELL_GEN_ADAPTIVE_WARDING_FROST,
                SPELL_GEN_ADAPTIVE_WARDING_SHADOW,
                SPELL_GEN_ADAPTIVE_WARDING_ARCANE
            });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetSpellInfo())
            return false;

        // find Mage Armor
        if (!GetTarget()->GetAuraEffect(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT, SPELLFAMILY_MAGE, 0x10000000))
            return false;

        switch (GetFirstSchoolInMask(eventInfo.GetSchoolMask()))
        {
        case SPELL_SCHOOL_NORMAL:
        case SPELL_SCHOOL_HOLY:
            return false;
        default:
            break;
        }
        return true;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        uint32 spellId = 0;
        switch (GetFirstSchoolInMask(eventInfo.GetSchoolMask()))
        {
        case SPELL_SCHOOL_FIRE:
            spellId = SPELL_GEN_ADAPTIVE_WARDING_FIRE;
            break;
        case SPELL_SCHOOL_NATURE:
            spellId = SPELL_GEN_ADAPTIVE_WARDING_NATURE;
            break;
        case SPELL_SCHOOL_FROST:
            spellId = SPELL_GEN_ADAPTIVE_WARDING_FROST;
            break;
        case SPELL_SCHOOL_SHADOW:
            spellId = SPELL_GEN_ADAPTIVE_WARDING_SHADOW;
            break;
        case SPELL_SCHOOL_ARCANE:
            spellId = SPELL_GEN_ADAPTIVE_WARDING_ARCANE;
            break;
        default:
            return;
        }
        GetTarget()->CastSpell(GetTarget(), spellId, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_gen_adaptive_warding::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_gen_adaptive_warding::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

enum ObsidianArmor
{
    SPELL_GEN_OBSIDIAN_ARMOR_HOLY       = 27536,
    SPELL_GEN_OBSIDIAN_ARMOR_FIRE       = 27533,
    SPELL_GEN_OBSIDIAN_ARMOR_NATURE     = 27538,
    SPELL_GEN_OBSIDIAN_ARMOR_FROST      = 27534,
    SPELL_GEN_OBSIDIAN_ARMOR_SHADOW     = 27535,
    SPELL_GEN_OBSIDIAN_ARMOR_ARCANE     = 27540
};

// 27539 - Obsidian Armor
class spell_gen_obsidian_armor : public AuraScript
{
    PrepareAuraScript(spell_gen_obsidian_armor);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
            {
                SPELL_GEN_OBSIDIAN_ARMOR_HOLY,
                SPELL_GEN_OBSIDIAN_ARMOR_FIRE,
                SPELL_GEN_OBSIDIAN_ARMOR_NATURE,
                SPELL_GEN_OBSIDIAN_ARMOR_FROST,
                SPELL_GEN_OBSIDIAN_ARMOR_SHADOW,
                SPELL_GEN_OBSIDIAN_ARMOR_ARCANE
            });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetSpellInfo())
            return false;

        if (GetFirstSchoolInMask(eventInfo.GetSchoolMask()) == SPELL_SCHOOL_NORMAL)
            return false;

        return true;
    }

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        uint32 spellId = 0;
        switch (GetFirstSchoolInMask(eventInfo.GetSchoolMask()))
        {
        case SPELL_SCHOOL_HOLY:
            spellId = SPELL_GEN_OBSIDIAN_ARMOR_HOLY;
            break;
        case SPELL_SCHOOL_FIRE:
            spellId = SPELL_GEN_OBSIDIAN_ARMOR_FIRE;
            break;
        case SPELL_SCHOOL_NATURE:
            spellId = SPELL_GEN_OBSIDIAN_ARMOR_NATURE;
            break;
        case SPELL_SCHOOL_FROST:
            spellId = SPELL_GEN_OBSIDIAN_ARMOR_FROST;
            break;
        case SPELL_SCHOOL_SHADOW:
            spellId = SPELL_GEN_OBSIDIAN_ARMOR_SHADOW;
            break;
        case SPELL_SCHOOL_ARCANE:
            spellId = SPELL_GEN_OBSIDIAN_ARMOR_ARCANE;
            break;
        default:
            return;
        }
        GetTarget()->CastSpell(GetTarget(), spellId, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_gen_obsidian_armor::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_gen_obsidian_armor::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 35095 - Enlightenment (trigger only from mana cost spells)
// No idea what this is for, this a condition ported from old core proc system
class spell_gen_enlightenment : public AuraScript
{
    PrepareAuraScript(spell_gen_enlightenment);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const * procSpell = eventInfo.GetSpellInfo(); 
        if ((procSpell->PowerType != POWER_MANA) || (procSpell->ManaCost == 0 && procSpell->ManaCostPercentage == 0 && procSpell->ManaCostPerlevel == 0))
            return false;

        return true;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_gen_enlightenment::CheckProc);
    }
};

void AddSC_generic_spell_scripts()
{
    new spell_gen_cannibalize();
    new spell_racial_stoneform();
    new spell_archimonde_doomfire();
    RegisterAuraScript(spell_gen_adaptive_warding);
    RegisterAuraScript(spell_gen_obsidian_armor);
    RegisterAuraScript(spell_gen_enlightenment);
}
