#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Player.h"
#include "SpellAuraEffects.h"

enum DruidSpells
{
    SPELL_DRUID_FORMS_TRINKET_BEAR          = 37340,
    SPELL_DRUID_FORMS_TRINKET_CAT           = 37341,
    SPELL_DRUID_FORMS_TRINKET_MOONKIN       = 37343,
    SPELL_DRUID_FORMS_TRINKET_NONE          = 37344,
    SPELL_DRUID_FORMS_TRINKET_TREE          = 37342,
    SPELL_DRUID_BEAR_FORM_PASSIVE           = 1178,
    SPELL_DRUID_DIRE_BEAR_FORM_PASSIVE      = 9635,
    SPELL_DRUID_ENRAGE                      = 5229,
    SPELL_DRUID_INCREASED_MOONFIRE_DURATION = 38414,
    SPELL_DRUID_LIFEBLOOM_FINAL_HEAL        = 33778,
    //Dreamwalker set 2 pieces bonus
    SPELL_DRUID_REJUVENATION_MANA           = 28722,
    SPELL_DRUID_REJUVENATION_RAGE           = 28723,
    SPELL_DRUID_REJUVENATION_ENERGY         = 28724,
    ////Dreamwalker set 6 pieces bonus
    SPELL_DRUID_BLESSING_OF_THE_CLAW        = 28750,
    // -
    SPELL_DRUID_BLESSING_OF_REMULOS         = 40445,
    SPELL_DRUID_BLESSING_OF_ELUNE           = 40446,
    SPELL_DRUID_BLESSING_OF_CENARIUS        = 40452,
    SPELL_DRUID_FRENZIED_REGENERATION_HEAL  = 22845,
    SPELL_DRUID_T3_PROC_ENERGIZE_MANA       = 28722,
    SPELL_DRUID_T3_PROC_ENERGIZE_RAGE       = 28723,
    SPELL_DRUID_T3_PROC_ENERGIZE_ENERGY     = 28724,
    SPELL_DRUID_IMP_LEADER_OF_THE_PACK_R1   = 34297,
    SPELL_DRUID_IMP_LEADER_OF_THE_PACK_HEAL = 34299,
    SPELL_DRUID_EXHILARATE                  = 28742,
    SPELL_DRUID_INFUSION                    = 37238,
};

// 34246 - Idol of the Emerald Queen
// 60779 - Idol of Lush Moss
class spell_dru_idol_lifebloom : public SpellScriptLoader
{
public:
    spell_dru_idol_lifebloom() : SpellScriptLoader("spell_dru_idol_lifebloom") { }

    class spell_dru_idol_lifebloom_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_idol_lifebloom_AuraScript);

        void HandleEffectCalcSpellMod(AuraEffect const* aurEff, SpellModifier*& spellMod)
        {
            if (!spellMod)
            {
                spellMod = new SpellModifier(GetAura());
                spellMod->op = SPELLMOD_DOT;
                spellMod->type = SPELLMOD_FLAT;
                spellMod->spellId = GetId();
                spellMod->mask = GetSpellInfo()->Effects[aurEff->GetEffIndex()].SpellClassMask;
            }
            spellMod->value = aurEff->GetAmount() / 7; //lifebloom total ticks
        }

        void Register()
        {
            DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_dru_idol_lifebloom_AuraScript::HandleEffectCalcSpellMod, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dru_idol_lifebloom_AuraScript();
    }
};


// 28716 - Dreamwalker set 2 pieces
// "Your Rejuvenation ticks have a chance to restore 60 mana, 8 energy, or 2 rage to your target."
class spell_dru_dreamwalker_raiment_2 : public SpellScriptLoader
{
public:
    spell_dru_dreamwalker_raiment_2() : SpellScriptLoader("spell_dru_dreamwalker_raiment_2") { }

    class spell_dru_dreamwalker_raiment_2_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_dreamwalker_raiment_2_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_DRUID_REJUVENATION_MANA }) 
                && ValidateSpellInfo({ SPELL_DRUID_REJUVENATION_RAGE })
                && ValidateSpellInfo({ SPELL_DRUID_REJUVENATION_ENERGY });
        }

        bool CheckProc(ProcEventInfo& /*eventInfo*/)
        {
            // Chance 50%
            return roll_chance_i(50);
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();

            uint32 triggered_spell_id = 0;
            switch (GetTarget()->GetPowerType())
            {
            case POWER_MANA:   triggered_spell_id = SPELL_DRUID_REJUVENATION_MANA; break;
            case POWER_RAGE:   triggered_spell_id = SPELL_DRUID_REJUVENATION_RAGE; break;
            case POWER_ENERGY: triggered_spell_id = SPELL_DRUID_REJUVENATION_ENERGY; break;
            default:
                return;
            }

            GetTarget()->CastSpell(GetTarget(), triggered_spell_id, aurEff);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_dru_dreamwalker_raiment_2_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_dru_dreamwalker_raiment_2_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_dreamwalker_raiment_2_AuraScript();
    }
};

// 28744 - Dreamwalker set 6 pieces
// "Your initial cast and Regrowth ticks will increase the maximum health of your target by up to 50, stacking up to 7 times."
class spell_dru_dreamwalker_raiment_6 : public SpellScriptLoader
{
public:
    spell_dru_dreamwalker_raiment_6() : SpellScriptLoader("spell_dru_dreamwalker_raiment_6") { }

    class spell_dru_dreamwalker_raiment_6_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_dreamwalker_raiment_6_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_DRUID_BLESSING_OF_THE_CLAW });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();
            GetTarget()->CastSpell(GetTarget(), SPELL_DRUID_BLESSING_OF_THE_CLAW, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dru_dreamwalker_raiment_6_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_dreamwalker_raiment_6_AuraScript();
    }
};

// 40442 - Druid Tier 6 Trinket
class spell_dru_item_t6_trinket : public SpellScriptLoader
{
public:
    spell_dru_item_t6_trinket() : SpellScriptLoader("spell_dru_item_t6_trinket") { }

    class spell_dru_item_t6_trinket_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_item_t6_trinket_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({
                SPELL_DRUID_BLESSING_OF_REMULOS,
                SPELL_DRUID_BLESSING_OF_ELUNE,
                SPELL_DRUID_BLESSING_OF_CENARIUS
                });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
            if (!spellInfo)
                return;

            uint32 spellId;
            int32 chance;

            // Starfire
            if (spellInfo->SpellFamilyFlags & 0x00000004)
            {
                spellId = SPELL_DRUID_BLESSING_OF_REMULOS;
                chance = 25;
            }
            // Rejuvenation
            else if (spellInfo->SpellFamilyFlags & 0x00000010)
            {
                spellId = SPELL_DRUID_BLESSING_OF_ELUNE;
                chance = 25;
            }
            // Mangle (Bear) and Mangle (Cat)
            else if (spellInfo->SpellFamilyFlags & 0x0000044000000000LL)
            {
                spellId = SPELL_DRUID_BLESSING_OF_CENARIUS;
                chance = 40;
            }
            else
                return;

            if (roll_chance_i(chance))
                eventInfo.GetActor()->CastSpell(nullptr, spellId, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dru_item_t6_trinket_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_item_t6_trinket_AuraScript();
    }
};

// 28719 - Healing Touch
class spell_dru_t3_8p_bonus : public SpellScriptLoader
{
public:
    spell_dru_t3_8p_bonus() : SpellScriptLoader("spell_dru_t3_8p_bonus") { }

    class spell_dru_t3_8p_bonus_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_t3_8p_bonus_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_DRUID_EXHILARATE });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
            if (!spellInfo)
                return;

            Unit* caster = eventInfo.GetActor();
            //int32 amount = CalculatePct(spellInfo->CalcPowerCost(caster, spellInfo->GetSchoolMask()), aurEff->GetAmount()); //no points in aura for BC
            int32 amount = CalculatePct(spellInfo->CalcPowerCost(caster, spellInfo->GetSchoolMask()), 30);
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(amount);
            caster->CastSpell(nullptr, SPELL_DRUID_EXHILARATE, args);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dru_t3_8p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_t3_8p_bonus_AuraScript();
    }
};

// 37288 - Mana Restore
// 37295 - Mana Restore
class spell_dru_t4_2p_bonus : public SpellScriptLoader
{
public:
    spell_dru_t4_2p_bonus() : SpellScriptLoader("spell_dru_t4_2p_bonus") { }

    class spell_dru_t4_2p_bonus_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_t4_2p_bonus_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_DRUID_INFUSION });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            eventInfo.GetActor()->CastSpell(nullptr, SPELL_DRUID_INFUSION, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dru_t4_2p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_t4_2p_bonus_AuraScript();
    }
};

// 37336 - Druid Forms Trinket
class spell_dru_forms_trinket : public SpellScriptLoader
{
public:
    spell_dru_forms_trinket() : SpellScriptLoader("spell_dru_forms_trinket") { }

    class spell_dru_forms_trinket_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_forms_trinket_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_DRUID_FORMS_TRINKET_BEAR,
                    SPELL_DRUID_FORMS_TRINKET_CAT,
                    SPELL_DRUID_FORMS_TRINKET_MOONKIN,
                    SPELL_DRUID_FORMS_TRINKET_NONE,
                    SPELL_DRUID_FORMS_TRINKET_TREE
                });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            Unit* target = eventInfo.GetActor();

            switch (target->GetShapeshiftForm())
            {
            case FORM_BEAR:
            case FORM_DIREBEAR:
            case FORM_CAT:
            case FORM_MOONKIN:
            case FORM_NONE:
            case FORM_TREE:
                return true;
            default:
                break;
            }

            return false;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* target = eventInfo.GetActor();
            uint32 triggerspell = 0;

            switch (target->GetShapeshiftForm())
            {
            case FORM_BEAR:
            case FORM_DIREBEAR:
                triggerspell = SPELL_DRUID_FORMS_TRINKET_BEAR;
                break;
            case FORM_CAT:
                triggerspell = SPELL_DRUID_FORMS_TRINKET_CAT;
                break;
            case FORM_MOONKIN:
                triggerspell = SPELL_DRUID_FORMS_TRINKET_MOONKIN;
                break;
            case FORM_NONE:
                triggerspell = SPELL_DRUID_FORMS_TRINKET_NONE;
                break;
            case FORM_TREE:
                triggerspell = SPELL_DRUID_FORMS_TRINKET_TREE;
                break;
            default:
                return;
            }

            target->CastSpell(target, triggerspell, aurEff);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_dru_forms_trinket_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_dru_forms_trinket_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_forms_trinket_AuraScript();
    }
};

void AddSC_druid_spell_scripts()
{
    new spell_dru_idol_lifebloom();
    new spell_dru_dreamwalker_raiment_2();
    new spell_dru_dreamwalker_raiment_6();
    new spell_dru_item_t6_trinket();
    new spell_dru_t3_8p_bonus();
    new spell_dru_t4_2p_bonus();
    new spell_dru_forms_trinket();
}
