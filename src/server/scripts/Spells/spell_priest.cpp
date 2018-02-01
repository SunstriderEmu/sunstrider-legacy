#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

class spell_shadowfiend : public SpellScriptLoader
{
public:
    spell_shadowfiend() : SpellScriptLoader("spell_shadowfiend") { }

    enum Shadowfiend_Spell
    {
        SPELL_MANA_LEECH = 28305,
    };

    class spell_shadowfiend_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_shadowfiend_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_MANA_LEECH))
                return false;
            return true;
        }

        void HandleAfterCast()
        {
            if (Unit* pet = GetCaster()->GetFirstMinion())
            {
                if (pet->GetEntry() != 19668) //shadowfiend npc
                    return;

                if (GetCaster()->GetVictim())
                    pet->EngageWithTarget(GetCaster()->GetVictim());

                pet->CastSpell(pet, SPELL_MANA_LEECH, true);
            }
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_shadowfiend_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_shadowfiend_SpellScript();
    }
};

// -17 - Power Word: Shield
class spell_pri_power_word_shield : public SpellScriptLoader
{
public:
    spell_pri_power_word_shield() : SpellScriptLoader("spell_pri_power_word_shield") { }

    class spell_pri_power_word_shield_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_power_word_shield_AuraScript);

        /*bool Validate(SpellInfo const* ) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED,
                    SPELL_PRIEST_REFLECTIVE_SHIELD_R1
                });
        }*/

        void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
        {
            canBeRecalculated = false;
            if (Unit* caster = GetCaster())
            {
#ifdef LICH_KING
                // +80.68% from sp bonus
                float bonus = 0.8068f;

                // Borrowed Time
                if (AuraEffect const* borrowedTime = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, PRIEST_ICON_ID_BORROWED_TIME, EFFECT_1))
                    bonus += CalculatePct(1.0f, borrowedTime->GetAmount());
#else
                float bonus = 0.3f; //to confirm
#endif

                bonus *= caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask());

                // Improved PW: Shield: its weird having a SPELLMOD_ALL_EFFECTS here but its blizzards doing :)
                // Improved PW: Shield is only applied at the spell healing bonus because it was already applied to the base value in CalculateSpellDamage
                bonus = caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bonus);
                bonus *= caster->CalculateSpellpowerCoefficientLevelPenalty(GetSpellInfo());

                amount += int32(bonus);

#ifdef LICH_KING
                // Twin Disciplines
                if (AuraEffect const* twinDisciplines = caster->GetAuraEffect(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_PRIEST, 0x400000, 0, 0, GetCasterGUID()))
                    AddPct(amount, twinDisciplines->GetAmount());
#endif

                // Focused Power
                amount *= caster->GetTotalAuraMultiplier(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
            }
        }

        /*
        void ReflectDamage(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            Unit* target = GetTarget();
            if (dmgInfo.GetAttacker() == target)
                return;

            if (AuraEffect* talentAurEff = target->GetAuraEffectOfRankedSpell(SPELL_PRIEST_REFLECTIVE_SHIELD_R1, EFFECT_0))
            {
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(CalculatePct(absorbAmount, talentAurEff->GetAmount()));
                target->CastSpell(dmgInfo.GetAttacker(), SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED, args);
            }
        }*/

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_power_word_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            //AfterEffectAbsorb += AuraEffectAbsorbFn(spell_pri_power_word_shield_AuraScript::ReflectDamage, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_power_word_shield_AuraScript();
    }
};

// 32676
class spell_pri_consume_magic : public SpellScriptLoader
{
public:
    spell_pri_consume_magic() : SpellScriptLoader("spell_pri_consume_magic") { }

    class spell_pri_consume_magic_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pri_consume_magic_SpellScript);

        SpellCastResult HandleCheckCast()
        {
            auto appliedAuras = GetCaster()->GetAppliedAuras();
            for (auto itr : appliedAuras)
            {
                SpellInfo const* spellInfo = itr.second->GetBase()->GetSpellInfo();
                if (!spellInfo->IsPassive() && spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST)
                    return SPELL_CAST_OK;
            }
            return SPELL_FAILED_CASTER_AURASTATE;
        }

        void HandleEffectHit(SpellEffIndex /*effIndex*/, int32& /*damage*/)
        {
            auto appliedAuras = GetCaster()->GetAppliedAuras();
            for (auto itr : appliedAuras)
            {
                SpellInfo const* spellInfo = itr.second->GetBase()->GetSpellInfo();
                if (!spellInfo->IsPassive() && spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST)
                {
                    GetCaster()->RemoveAurasDueToSpell(spellInfo->Id);
                    break;
                }
            }
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_pri_consume_magic_SpellScript::HandleCheckCast);
            OnEffectHitTarget += SpellEffectFn(spell_pri_consume_magic_SpellScript::HandleEffectHit, EFFECT_0, SPELL_EFFECT_ENERGIZE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_pri_consume_magic_SpellScript();
    }
};

// 33076
class spell_pri_prayer_of_mending : public SpellScriptLoader
{
public:
    spell_pri_prayer_of_mending() : SpellScriptLoader("spell_pri_prayer_of_mending") { }

    /*
    Prayer of mending is:
    Player cast spell 33076 (SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE) which triggers:
        - 41637, jump visual
        - 41635, PoM aura which does the healing when procced. 
    41635 aura will use amount from this aura and keep it with each jump, see handling in AuraEffect::HandleRaidProcFromChargeWithValueAuraProc
    */
    class spell_pri_prayer_of_mending_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pri_prayer_of_mending_SpellScript);

        void OnLaunch(SpellEffIndex effIndex, int32& damage)
        {
            if (Unit* caster = GetCaster())
            {
                float bonus = 0.43f; //value from DrDamage (per proc)
                bonus *= caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask());

                // ? bonus = caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bonus);

                damage += int32(bonus);

                // Focused Power
                damage *= caster->GetTotalAuraMultiplier(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
            }
        }

        void Register() override
        {
            OnEffectLaunchTarget += SpellEffectFn(spell_pri_prayer_of_mending_SpellScript::OnLaunch, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_pri_prayer_of_mending_SpellScript();
    }
};

void AddSC_priest_spell_scripts()
{
    new spell_shadowfiend();
    new spell_pri_power_word_shield();
    new spell_pri_consume_magic();
    new spell_pri_prayer_of_mending();
}
