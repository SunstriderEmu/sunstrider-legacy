#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum PriestSpells
{
    SPELL_PRIEST_VAMPIRIC_TOUCH_PROC                = 34919,
    SPELL_PRIEST_VAMPIRIC_EMBRACE_HEAL              = 15290,
    SPELL_PRIEST_DIVINE_BLESSING                    = 40440,
    SPELL_PRIEST_DIVINE_WRATH                       = 40441,
    SPELL_PRIEST_ORACULAR_HEAL                      = 26170,
    SPELL_PRIEST_MIND_BLAST_R1                      = 8092,
    SPELL_PRIEST_SHADOW_WORD_DEATH_R1               = 32379,
    SPELL_PRIEST_ARMOR_OF_FAITH                     = 28810,
    SPELL_PRIEST_ABOLISH_DISEASE                    = 552,
    SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED        = 33619,
    SPELL_PRIEST_REFLECTIVE_SHIELD_R1               = 33201,
    SPELL_PRIEST_SHADOW_WORD_DEATH                  = 32409,
    SPELL_PRIEST_ITEM_EFFICIENCY                    = 37595,
    SPELL_PRIEST_MANA_LEECH_PROC                    = 34650,
    SPELL_PRIEST_BLESSED_RECOVERY_R1                = 27813,
};

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

// 15286 - Vampiric Embrace
class spell_pri_vampiric_embrace : public SpellScriptLoader
{
public:
    spell_pri_vampiric_embrace() : SpellScriptLoader("spell_pri_vampiric_embrace") { }

    class spell_pri_vampiric_embrace_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_vampiric_embrace_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PRIEST_VAMPIRIC_EMBRACE_HEAL });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            Unit* triggerTarget = eventInfo.GetProcTarget();
            if (!triggerTarget || !triggerTarget->IsAlive())
                return false;

            // only proc on shadowpriest damage
            if (GetCasterGUID() != triggerTarget->GetGUID())
                return false;

            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            DamageInfo* damageInfo = eventInfo.GetDamageInfo();
            if (!damageInfo || !damageInfo->GetDamage())
                return;

            int32 partyHeal = CalculatePct(static_cast<int32>(damageInfo->GetDamage()), aurEff->GetAmount());
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(partyHeal);
            eventInfo.GetActor()->CastSpell(nullptr, SPELL_PRIEST_VAMPIRIC_EMBRACE_HEAL, args);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_pri_vampiric_embrace_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_pri_vampiric_embrace_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_vampiric_embrace_AuraScript();
    }
};

// -34914 - Vampiric Touch
class spell_pri_vampiric_touch : public SpellScriptLoader
{
public:
    spell_pri_vampiric_touch() : SpellScriptLoader("spell_pri_vampiric_touch") { }

    class spell_pri_vampiric_touch_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_vampiric_touch_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PRIEST_VAMPIRIC_TOUCH_PROC });
        }

        bool CheckProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            Unit* triggerTarget = eventInfo.GetProcTarget();
            if (!triggerTarget || !triggerTarget->IsAlive())
                return false;

            // only proc on shadowpriest damage
            if (GetCasterGUID() != triggerTarget->GetGUID())
                return false;

            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (!eventInfo.GetDamageInfo())
                return;

            // energize amount
            uint32 basepoints0 = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), aurEff->GetAmount());
            CastSpellExtraArgs args(aurEff->GetBase());
            args.AddSpellBP0(int32(basepoints0));
            GetCaster()->CastSpell(GetCaster(), SPELL_PRIEST_VAMPIRIC_TOUCH_PROC, args);
        }

        void Register() override
        {
            DoCheckEffectProc += AuraCheckEffectProcFn(spell_pri_vampiric_touch_AuraScript::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
            OnEffectProc += AuraEffectProcFn(spell_pri_vampiric_touch_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_vampiric_touch_AuraScript();
    }
};

// 40438 - Priest Tier 6 Trinket
class spell_pri_item_t6_trinket : public SpellScriptLoader
{
public:
    spell_pri_item_t6_trinket() : SpellScriptLoader("spell_pri_item_t6_trinket") { }

    class spell_pri_item_t6_trinket_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_item_t6_trinket_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_PRIEST_DIVINE_BLESSING,
                    SPELL_PRIEST_DIVINE_WRATH
                });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* caster = eventInfo.GetActor();
            if (eventInfo.GetSpellTypeMask() & PROC_SPELL_TYPE_HEAL)
                caster->CastSpell(nullptr, SPELL_PRIEST_DIVINE_BLESSING, aurEff);

            if (eventInfo.GetSpellTypeMask() & PROC_SPELL_TYPE_DAMAGE)
                caster->CastSpell(nullptr, SPELL_PRIEST_DIVINE_WRATH, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pri_item_t6_trinket_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_item_t6_trinket_AuraScript();
    }
};

// 26169 - Oracle Healing Bonus
class spell_pri_aq_3p_bonus : public SpellScriptLoader
{
public:
    spell_pri_aq_3p_bonus() : SpellScriptLoader("spell_pri_aq_3p_bonus") { }

    class spell_pri_aq_3p_bonus_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_aq_3p_bonus_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PRIEST_ORACULAR_HEAL });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* caster = eventInfo.GetActor();
            if (caster == eventInfo.GetProcTarget())
                return;

            HealInfo* healInfo = eventInfo.GetHealInfo();
            if (!healInfo || !healInfo->GetHeal())
                return;

            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(CalculatePct(healInfo->GetHeal(), 10));
            caster->CastSpell(caster, SPELL_PRIEST_ORACULAR_HEAL, args);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pri_aq_3p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_aq_3p_bonus_AuraScript();
    }
};

// 28809 - Greater Heal
class spell_pri_t3_4p_bonus : public SpellScriptLoader
{
public:
    spell_pri_t3_4p_bonus() : SpellScriptLoader("spell_pri_t3_4p_bonus") { }

    class spell_pri_t3_4p_bonus_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_t3_4p_bonus_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PRIEST_ARMOR_OF_FAITH });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), SPELL_PRIEST_ARMOR_OF_FAITH, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pri_t3_4p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_t3_4p_bonus_AuraScript();
    }
};


// 28305 - Mana Leech (Passive) (Priest Pet Aura)
class spell_pri_mana_leech : public SpellScriptLoader
{
public:
    spell_pri_mana_leech() : SpellScriptLoader("spell_pri_mana_leech") { }

    class spell_pri_mana_leech_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_mana_leech_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PRIEST_MANA_LEECH_PROC });
        }

        bool CheckProc(ProcEventInfo& /*eventInfo*/)
        {
            _procTarget = GetTarget()->GetOwner();
            return _procTarget != nullptr;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
#ifdef LICH_KING
            GetTarget()->CastSpell(_procTarget, SPELL_PRIEST_MANA_LEECH_PROC, aurEff);
#else
            if (!eventInfo.GetDamageInfo())
                return;
            //http://wowwiki.wikia.com/wiki/Shadowfiend?oldid=1630551
            uint32 const damage = eventInfo.GetDamageInfo()->GetDamage() * 2.5f; // The mana gained from Shadowfiends' attack is 250% of the damage dealt by the fiend.
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(damage);
            GetTarget()->CastSpell(_procTarget, SPELL_PRIEST_MANA_LEECH_PROC, args);
#endif
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_pri_mana_leech_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_pri_mana_leech_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }

        Unit* _procTarget = nullptr;
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_mana_leech_AuraScript();
    }
};

// -27811 - Blessed Recovery
class spell_pri_blessed_recovery : public SpellScriptLoader
{
public:
    spell_pri_blessed_recovery() : SpellScriptLoader("spell_pri_blessed_recovery") { }

    class spell_pri_blessed_recovery_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_blessed_recovery_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PRIEST_BLESSED_RECOVERY_R1 });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            DamageInfo* dmgInfo = eventInfo.GetDamageInfo();
            if (!dmgInfo || !dmgInfo->GetDamage())
                return;

            Unit* target = eventInfo.GetActionTarget();
            uint32 triggerSpell = sSpellMgr->GetSpellWithRank(SPELL_PRIEST_BLESSED_RECOVERY_R1, aurEff->GetSpellInfo()->GetRank());
            SpellInfo const* triggerInfo = sSpellMgr->AssertSpellInfo(triggerSpell);

            int32 bp = CalculatePct(static_cast<int32>(dmgInfo->GetDamage()), aurEff->GetAmount());

            ASSERT(triggerInfo->GetMaxTicks() > 0);
            bp /= triggerInfo->GetMaxTicks();
            bp += target->GetRemainingPeriodicAmount(target->GetGUID(), triggerSpell, SPELL_AURA_PERIODIC_HEAL);

            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(bp);
            target->CastSpell(target, triggerSpell, args);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pri_blessed_recovery_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_blessed_recovery_AuraScript();
    }
};

void AddSC_priest_spell_scripts()
{
    new spell_shadowfiend();
    new spell_pri_power_word_shield();
    new spell_pri_consume_magic();
    new spell_pri_prayer_of_mending();
    new spell_pri_vampiric_touch();
    new spell_pri_vampiric_embrace();
    new spell_pri_item_t6_trinket();
    new spell_pri_aq_3p_bonus();
    new spell_pri_t3_4p_bonus();
    new spell_pri_mana_leech();
    new spell_pri_blessed_recovery();
}
