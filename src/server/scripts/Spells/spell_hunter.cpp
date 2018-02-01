#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Player.h"
#include "SpellAuraEffects.h"

enum HunterSpells
{
    SPELL_HUNTER_ASPECT_OF_THE_BEAST                = 13161,
    SPELL_HUNTER_ASPECT_OF_THE_VIPER                = 34074,
    SPELL_HUNTER_ASPECT_OF_THE_VIPER_ENERGIZE       = 34075,
    SPELL_HUNTER_BESTIAL_WRATH                      = 19574,
    SPELL_HUNTER_IMPROVED_MEND_PET                  = 24406,
    SPELL_HUNTER_ENTRAPMENT_PROC                    = 19185,
};

//TODO: should this be moved to DB?
class spell_hunt_imprv_aspect_viper : public SpellScriptLoader
{
public:
    spell_hunt_imprv_aspect_viper() : SpellScriptLoader("spell_hunt_imprv_aspect_viper") { }

    class spell_hunt_imprv_aspect_viper_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hunt_imprv_aspect_viper_AuraScript);

        void HandleEffectCalcSpellMod(AuraEffect const* aurEff, SpellModifier*& spellMod)
        {
            if (!spellMod)
            {
                // + effect value for Aspect of the Viper
                spellMod = new SpellModifier(aurEff->GetBase());
                spellMod->op = SPELLMOD_EFFECT1;
                spellMod->type = SPELLMOD_FLAT;
                spellMod->spellId = GetId();
                spellMod->mask = 0x4000000000000LL;
            }
            spellMod->value = aurEff->GetAmount();
        }

        void Register()
        {
            DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_hunt_imprv_aspect_viper_AuraScript::HandleEffectCalcSpellMod, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hunt_imprv_aspect_viper_AuraScript();
    }
};

// -19572 - Improved Mend Pet
class spell_hun_improved_mend_pet : public SpellScriptLoader
{
public:
    spell_hun_improved_mend_pet() : SpellScriptLoader("spell_hun_improved_mend_pet") { }

    class spell_hun_improved_mend_pet_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_improved_mend_pet_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_HUNTER_IMPROVED_MEND_PET });
        }

        bool CheckProc(ProcEventInfo& /*eventInfo*/)
        {
            return roll_chance_i(GetEffect(EFFECT_0)->GetAmount());
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();
            GetTarget()->CastSpell(GetTarget(), SPELL_HUNTER_IMPROVED_MEND_PET, aurEff);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_hun_improved_mend_pet_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_hun_improved_mend_pet_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_hun_improved_mend_pet_AuraScript();
    }
};

class spell_hunt_hunters_mark : public SpellScriptLoader
{
public:
    spell_hunt_hunters_mark() : SpellScriptLoader("spell_hunt_hunters_mark") { }

    class spell_hunt_hunters_mark_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hunt_hunters_mark_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return eventInfo.GetSpellInfo()->IsRangedWeaponSpell();
        }

        float GetImprovedMarkCoef()
        {
            Unit::AuraEffectList const& m_OverrideClassScript = GetCaster()->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
            for (auto i : m_OverrideClassScript)
            {
                switch (i->GetMiscValue())
                {
                case 5240: //rank 1
                case 5237:
                case 5238:
                case 5236:
                case 5239: //rank 5
                {
                    return i->GetAmount() / 100.0f;
                }
                }
            }
            return 0.0f;
        }

        //increase ranged AP effect
        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            uint32 basevalue = aurEff->GetBaseAmount();
            AuraEffect* auraEffect = const_cast<AuraEffect*>(aurEff);
            uint32 maxAmount = (basevalue + 1) * 4;
            uint32 newAmount = aurEff->GetAmount() + (basevalue + 1) / 10.0f;
            auraEffect->SetAmount(newAmount);
            //cap it
            if (auraEffect->GetAmount() > maxAmount)
                auraEffect->SetAmount(maxAmount);
        }

        //Melee effect, only has effect if hunter has Improved Hunter's Mark talent 
        void HandleProc2(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            float coef = GetImprovedMarkCoef();
            if (coef == 0.0f)
                return;
           
            if (AuraEffect const* effect1 = aurEff->GetBase()->GetEffect(EFFECT_1))
            {
                AuraEffect* auraEffect = const_cast<AuraEffect*>(aurEff);
                auraEffect->SetAmount(std::round(effect1->GetAmount() * coef));
            }
        }

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & /*amount*/, bool & canBeRecalculated)
        {
            canBeRecalculated = false; //this allow to refresh aura without losing gained AP
        }

        void CalculateAmount2(AuraEffect const* aurEff, int32 & amount, bool & canBeRecalculated)
        {
            canBeRecalculated = false; //this allow to refresh aura without losing gained AP
            float coef = GetImprovedMarkCoef();
            amount = std::round(aurEff->GetBase()->GetSpellInfo()->Effects[EFFECT_1].BasePoints * coef); // value from effect 1 (ranged AP) * coef from improved mark talent
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_hunt_hunters_mark_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_hunt_hunters_mark_AuraScript::HandleProc, EFFECT_1, SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS);
            OnEffectProc += AuraEffectProcFn(spell_hunt_hunters_mark_AuraScript::HandleProc2, EFFECT_2, SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hunt_hunters_mark_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hunt_hunters_mark_AuraScript::CalculateAmount2, EFFECT_2, SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_hunt_hunters_mark_AuraScript();
    }
};


// -19184 - Entrapment (talent)
/* Frost trap wasn't working because the target is a dynamic object and not a unit. Since this is probably only affecting this spell... I'm doing it here in a script instead of changing core logic.
*/
class spell_hun_entrapment : public SpellScriptLoader
{
public:
    spell_hun_entrapment() : SpellScriptLoader("spell_hun_entrapment") { }

    class spell_hun_entrapment_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_entrapment_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_HUNTER_ENTRAPMENT_PROC });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (!eventInfo.GetProcSpell())
                return;
            WorldObject const* wTarget = eventInfo.GetProcSpell()->GetOriginalTarget();
            if(Unit const* target = wTarget->ToUnit())
                GetCaster()->CastSpell(target, SPELL_HUNTER_ENTRAPMENT_PROC, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_hun_entrapment_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_hun_entrapment_AuraScript();
    }
};

void AddSC_hunter_spell_scripts()
{
    new spell_hunt_imprv_aspect_viper();
    new spell_hun_improved_mend_pet();
    new spell_hunt_hunters_mark();
    new spell_hun_entrapment();
}
