#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Player.h"
#include "SpellAuraEffects.h"

enum DruidSpells
{
    //Dreamwalker set 2 pieces bonus
    SPELL_DRUID_REJUVENATION_MANA = 28722,
    SPELL_DRUID_REJUVENATION_RAGE = 28723,
    SPELL_DRUID_REJUVENATION_ENERGY = 28724,
    ////Dreamwalker set 6 pieces bonus
    SPELL_DRUID_BLESSING_OF_THE_CLAW = 28750,
    // -
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

void AddSC_druid_spell_scripts()
{
    new spell_dru_idol_lifebloom();
    new spell_dru_dreamwalker_raiment_2();
    new spell_dru_dreamwalker_raiment_6();
}
