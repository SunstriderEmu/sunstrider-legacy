#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Player.h"
#include "SpellAuraEffects.h"

enum PaladinSpells
{
    SPELL_PALADIN_HOLY_SHOCK_R1                  = 20473,
    SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE           = 25912,
    SPELL_PALADIN_HOLY_SHOCK_R1_HEALING          = 25914,
    SPELL_PALADIN_ILLUMINATION_ENERGIZE          = 20272,

    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_DRUID   = 37878,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PALADIN = 37879,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PRIEST  = 37880,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_SHAMAN  = 37881,

    SPELL_PALADIN_HOLY_LIGHT                     = 635,

    SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE          = 25997,

    SPELL_PALADIN_FORBEARANCE                    = 25771,
    SPELL_PALADIN_ITEM_HEALING_TRANCE            = 37706,

    SPELL_PALADIN_JUDGEMENT_OF_JUSTICE           = 20184,
    SPELL_PALADIN_JUDGEMENT_OF_LIGHT             = 20185,
    SPELL_PALADIN_JUDGEMENT_OF_WISDOM            = 20186,

    SPELL_PALADIN_JUDGEMENT_OF_LIGHT_HEAL        = 20267,
    SPELL_PALADIN_JUDGEMENT_OF_WISDOM_MANA       = 20268,

    SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT        = 31790,

    SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS          = 25742,

    SPELL_PALADIN_CONCENTRACTION_AURA            = 19746,
    SPELL_PALADIN_SANCTIFIED_RETRIBUTION_R1      = 31869,

    SPELL_PALADIN_JUDGEMENTS_OF_THE_WISE_MANA    = 31930,
    SPELL_PALADIN_HEART_OF_THE_CRUSADER_EFF_R1   = 21183,

    SPELL_PALADIN_HOLY_POWER_ARMOR               = 28790,
    SPELL_PALADIN_HOLY_POWER_ATTACK_POWER        = 28791,
    SPELL_PALADIN_HOLY_POWER_SPELL_POWER         = 28793,
    SPELL_PALADIN_HOLY_POWER_MP5                 = 28795,

    SPELL_PALADIN_HOLY_VENGEANCE                 = 31803,
    SPELL_PALADIN_SEAL_OF_VENGEANCE_DAMAGE       = 42463,

    SPELL_PALADIN_SPIRITUAL_ATTUNEMENT_MANA      = 31786,

    SPELL_PALADIN_ENDURING_LIGHT                 = 40471,
    SPELL_PALADIN_ENDURING_JUDGEMENT             = 40472,

    SPELL_PALADIN_SEAL_OF_BLOOD_DAMAGE_PROC      = 31893,
    SPELL_PALADIN_SEAL_OF_BLOOD_SELF_DAMAGE      = 32221,

};

// -20467 - Judgement of Command
class spell_pal_judgement_of_command : public SpellScriptLoader
{
public:
    spell_pal_judgement_of_command() : SpellScriptLoader("spell_pal_judgement_of_command") { }

    class spell_pal_judgement_of_command_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_judgement_of_command_SpellScript);

        SpellCastResult CheckTarget()
        {
            if (GetHitUnit() && GetHitUnit()->HasUnitState(UNIT_STATE_STUNNED))
                return SPELL_CAST_OK;

            return SPELL_FAILED_DONT_REPORT;

        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_pal_judgement_of_command_SpellScript::CheckTarget);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_pal_judgement_of_command_SpellScript();
    }
};

// -31785 - Spiritual Attunement
class spell_pal_spiritual_attunement : public SpellScriptLoader
{
public:
    spell_pal_spiritual_attunement() : SpellScriptLoader("spell_pal_spiritual_attunement") { }

    class spell_pal_spiritual_attunement_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_spiritual_attunement_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PALADIN_SPIRITUAL_ATTUNEMENT_MANA });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            // "when healed by other friendly targets' spells"
            if (eventInfo.GetProcTarget() == eventInfo.GetActionTarget())
                return false;

            return eventInfo.GetHealInfo() && eventInfo.GetHealInfo()->GetEffectiveHeal();
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            HealInfo* healInfo = eventInfo.GetHealInfo();
            int32 amount = CalculatePct(static_cast<int32>(healInfo->GetEffectiveHeal()), aurEff->GetAmount());

            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(amount);
            eventInfo.GetActionTarget()->CastSpell(nullptr, SPELL_PALADIN_SPIRITUAL_ATTUNEMENT_MANA, args);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_pal_spiritual_attunement_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_pal_spiritual_attunement_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_spiritual_attunement_AuraScript();
    }
};

// 40470 - Paladin Tier 6 Trinket
class spell_pal_item_t6_trinket : public SpellScriptLoader
{
public:
    spell_pal_item_t6_trinket() : SpellScriptLoader("spell_pal_item_t6_trinket") { }

    class spell_pal_item_t6_trinket_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_item_t6_trinket_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_PALADIN_ENDURING_LIGHT,
                    SPELL_PALADIN_ENDURING_JUDGEMENT
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

            // Holy Light & Flash of Light
            if (spellInfo->SpellFamilyFlags & 0xC0000000)
            {
                spellId = SPELL_PALADIN_ENDURING_LIGHT;
                chance = 15;
            }
            // Judgements
            else if (spellInfo->SpellFamilyFlags & 0x00800000)
            {
                spellId = SPELL_PALADIN_ENDURING_JUDGEMENT;
                chance = 50;
            }
            else
                return;

            if (roll_chance_i(chance))
                eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), spellId, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pal_item_t6_trinket_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_item_t6_trinket_AuraScript();
    }
};

// 28789 - Holy Power
class spell_pal_t3_6p_bonus : public SpellScriptLoader
{
public:
    spell_pal_t3_6p_bonus() : SpellScriptLoader("spell_pal_t3_6p_bonus") { }

    class spell_pal_t3_6p_bonus_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_t3_6p_bonus_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_PALADIN_HOLY_POWER_ARMOR,
                    SPELL_PALADIN_HOLY_POWER_ATTACK_POWER,
                    SPELL_PALADIN_HOLY_POWER_SPELL_POWER,
                    SPELL_PALADIN_HOLY_POWER_MP5
                });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            uint32 spellId;
            Unit* caster = eventInfo.GetActor();
            Unit* target = eventInfo.GetProcTarget();

            switch (target->GetClass())
            {
            case CLASS_PALADIN:
            case CLASS_PRIEST:
            case CLASS_SHAMAN:
            case CLASS_DRUID:
                spellId = SPELL_PALADIN_HOLY_POWER_MP5;
                break;
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                spellId = SPELL_PALADIN_HOLY_POWER_SPELL_POWER;
                break;
            case CLASS_HUNTER:
            case CLASS_ROGUE:
                spellId = SPELL_PALADIN_HOLY_POWER_ATTACK_POWER;
                break;
            case CLASS_WARRIOR:
                spellId = SPELL_PALADIN_HOLY_POWER_ARMOR;
                break;
            default:
                return;
            }

            caster->CastSpell(target, spellId, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pal_t3_6p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_t3_6p_bonus_AuraScript();
    }
};

// 31801 - Seal of Vengeance
template <uint32 DoTSpellId, uint32 DamageSpellId>
class spell_pal_seal_of_vengeance : public SpellScriptLoader
{
    public:
        spell_pal_seal_of_vengeance(char const* ScriptName) : SpellScriptLoader(ScriptName) { }

        template <uint32 DoTSpell, uint32 DamageSpell>
        class spell_pal_seal_of_vengeance_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_seal_of_vengeance_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    DoTSpell,
                    DamageSpell
                });
            }

            /*
            When an auto-attack lands (does not dodge/parry/miss) that can proc a seal the of the following things happen independently of each other (see 2 roll system).

            1) A "hidden strike" which uses melee combat mechanics occurs. If it lands it refreshes/stacks SoV DoT. Only white swings can trigger a refresh or stack. (This hidden strike mechanic can also proc things like berserking..)
            2) A weapon damage based proc will occur if you used a special (CS/DS/judge) or if you have a 5 stack (from auto attacks). This attack can not be avoided.

            Remember #2 happens regardless of #1 landing, it just requires the initial attack (autos, cs, etc) to land.

            Stack Number    % of Weapon Damage  % with SotP
            0               0%                  0%
            1               6.6%                7.6%
            2               13.2%               15.2%
            3               19.8%               22.8%
            4               26.4%               30.4%
            5               33%                 38%
            */

            void HandleApplyDoT(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!(eventInfo.GetTypeMask() & PROC_FLAG_DONE_MELEE_AUTO_ATTACK))
                    return;

                // don't cast triggered, spell already has SPELL_ATTR4_CAN_CAST_WHILE_CASTING attr
                eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), DoTSpell, CastSpellExtraArgs(TRIGGERED_DONT_RESET_PERIODIC_TIMER).SetTriggeringAura(aurEff));
            }

            //spell also does additional damage if we reached max stacks
            void HandleSeal(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                Unit* caster = eventInfo.GetActor();
                Unit* target = eventInfo.GetProcTarget();

                // get current aura on target, if any
                AuraEffect const* sealDot = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_PALADIN, 0x0000080000000000, caster->GetGUID());
                if (!sealDot)
                    return;

                uint8 const stacks = sealDot->GetBase()->GetStackAmount();
                uint8 const maxStacks = sealDot->GetSpellInfo()->StackAmount;

                if (stacks < maxStacks)
                    return;

                SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(DamageSpell);
                int32 amount = caster->SpellDamageBonusDone(target, spellInfo, sealDot->GetAmount(), DOT, aurEff->GetEffIndex()) / 2;

                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(amount);
                caster->CastSpell(target, DamageSpell, args);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_seal_of_vengeance_AuraScript::HandleApplyDoT, EFFECT_0, SPELL_AURA_DUMMY);
                OnEffectProc += AuraEffectProcFn(spell_pal_seal_of_vengeance_AuraScript::HandleSeal, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_seal_of_vengeance_AuraScript<DoTSpellId, DamageSpellId>();
        }
};

// 20375 - Seal of Command
// 21084 - Seal of Righteousness
// 31801 - Seal of Vengeance
// 31892 - Seal of Blood
// 33127 - Seal of Command
// 38008 - Seal of Blood
// 41459 - Seal of Blood
class spell_pal_seals : public SpellScriptLoader
{
public:
    spell_pal_seals() : SpellScriptLoader("spell_pal_seals") { }

    class spell_pal_seals_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_seals_AuraScript);

        // Effect 2 is used by Judgement code, we prevent the proc to avoid console logging of unknown spell trigger
        bool CheckDummyProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
        {
            return false;
        }

        void Register() override
        {
            DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_seals_AuraScript::CheckDummyProc, EFFECT_2, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_seals_AuraScript();
    }
};


// -31892 Seal of blood
class spell_pal_seal_of_blood : public SpellScriptLoader
{
public:
    spell_pal_seal_of_blood() : SpellScriptLoader("spell_pal_seal_of_blood") { }

    class spell_pal_seal_of_blood_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_seal_of_blood_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo( { SPELL_PALADIN_SEAL_OF_BLOOD_DAMAGE_PROC, SPELL_PALADIN_SEAL_OF_BLOOD_SELF_DAMAGE });
        }

        void HandleTargetDamage(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            if (!(eventInfo.GetTypeMask() & PROC_FLAG_DONE_MELEE_AUTO_ATTACK))
                return;

            eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), SPELL_PALADIN_SEAL_OF_BLOOD_DAMAGE_PROC, true);
        }

        //spell also does additional damage if we reached max stacks
        void HandleSelfDamage(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            Unit* caster = GetTarget();
            if (!caster)
                return;

            //todo: how to get exact damage from first effect? For now, just recalc new weapon damage... 
            //but should this take armor or real damage inflicted? Idea: we should enter here with a new proc event when effect0 does its damage
            uint32 damage = CalculatePct(caster->CalculateDamage(BASE_ATTACK, false, true), 35.0f); //35% weapon damage
            damage = CalculatePct(damage, aurEff->GetAmount()); //10% damage to self

            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(damage);
            caster->CastSpell(caster, SPELL_PALADIN_SEAL_OF_BLOOD_SELF_DAMAGE, args);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pal_seal_of_blood_AuraScript::HandleTargetDamage, EFFECT_0, SPELL_AURA_DUMMY);
            OnEffectProc += AuraEffectProcFn(spell_pal_seal_of_blood_AuraScript::HandleSelfDamage, EFFECT_1, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_seal_of_blood_AuraScript();
    }
};

// 20154, 21084 - Seal of Righteousness - melee proc dummy (addition ${$MWS*(0.022*$AP+0.044*$SPH)} damage)
class spell_pal_seal_of_righteousness : public SpellScriptLoader
{
public:
    spell_pal_seal_of_righteousness() : SpellScriptLoader("spell_pal_seal_of_righteousness") { }

    class spell_pal_seal_of_righteousness_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_seal_of_righteousness_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return eventInfo.GetProcTarget() != nullptr;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            uint32 spellId;
            switch (aurEff->GetId())
            {
            case 21084: spellId = 25742; break;     // Rank 1
            case 20287: spellId = 25740; break;     // Rank 2
            case 20288: spellId = 25739; break;     // Rank 3
            case 20289: spellId = 25738; break;     // Rank 4
            case 20290: spellId = 25737; break;     // Rank 5
            case 20291: spellId = 25736; break;     // Rank 6
            case 20292: spellId = 25735; break;     // Rank 7
            case 20293: spellId = 25713; break;     // Rank 8
            case 27155: spellId = 27156; break;     // Rank 9
            default:
                TC_LOG_ERROR("spell", "spell_pal_seal_of_righteousness: non handled possibly SoR (Id = %u)", aurEff->GetId());
                return;
            }

            SpellInfo const* procSpellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!procSpellInfo)
                return;

            /* TC
            float ap = GetTarget()->GetTotalAttackPowerValue(BASE_ATTACK);
            int32 holy = GetTarget()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY);
            holy += eventInfo.GetProcTarget()->SpellBaseDamageBonusTaken(SPELL_SCHOOL_MASK_HOLY);
            int32 bp = int32((ap * 0.022f + 0.044f * holy) * GetTarget()->GetAttackTime(BASE_ATTACK) / 1000);
            */
            //BC logic from WoWWiki http://wowwiki.wikia.com/wiki/Seal_of_Righteousness_(old)?oldid=1432433
            Unit* triggerCaster = GetTarget();

            Item* item = (triggerCaster->ToPlayer())->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            float speed = (item ? item->GetTemplate()->Delay : BASE_ATTACK_TIME) / 1000.0f;

            float magicCoef = 1.2f * 1.03f; //this allow to match WoWWiki data about tooltips
            float damageBasePoints;
            bool twoHand = item && item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;
            if (twoHand)
                // two hand weapon
                damageBasePoints = 1.20f * aurEff->GetAmount() * magicCoef * speed / 100.0f + 1;
            else
                // one hand weapon/no weapon
                damageBasePoints = 0.85f * aurEff->GetAmount() * magicCoef * speed / 100.0f - 1;

            int32 damagePoint = int32(std::ceil(damageBasePoints + 0.03f * (triggerCaster->GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE) + triggerCaster->GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE)) / 2.0f)) + 1; //ceil to match tooltip

            // apply damage bonuses manually
            if (damagePoint >= 0)
            {
                /*   9.2% per 1.0 weapon speed, One-handed Seal of Righteousness
                     10.8% per 1.0 weapon speed, Two-handed Seal of Righteousness */
                float spellPowerCoef = (twoHand ? 0.108f : 0.092f) * speed;

                damagePoint += triggerCaster->SpellBaseDamageBonusDone(procSpellInfo->GetSchoolMask()) * spellPowerCoef;
            }

            // apply spellmod to Done damage
            if (Player* modOwner = triggerCaster->GetSpellModOwner())
                modOwner->ApplySpellMod(aurEff->GetSpellInfo()->Id, SPELLMOD_DAMAGE, damagePoint); //using this aura spell info because proc has no spellfamilyflags

            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(damagePoint);
            GetTarget()->CastSpell(eventInfo.GetProcTarget(), spellId, args);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_pal_seal_of_righteousness_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_pal_seal_of_righteousness_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_seal_of_righteousness_AuraScript();
    }
};

// -20210 - Illumination
class spell_pal_illumination : public SpellScriptLoader
{
public:
    spell_pal_illumination() : SpellScriptLoader("spell_pal_illumination") { }

    class spell_pal_illumination_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_illumination_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_PALADIN_HOLY_SHOCK_R1_HEALING,
                    SPELL_PALADIN_ILLUMINATION_ENERGIZE,
                    SPELL_PALADIN_HOLY_SHOCK_R1
                });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            // this script is valid only for the Holy Shock procs of illumination
            if (eventInfo.GetHealInfo() && eventInfo.GetHealInfo()->GetSpellInfo())
            {
                SpellInfo const* originalSpell = nullptr;

                // if proc comes from the Holy Shock heal, need to get mana cost of original spell - else it's the original heal itself
                if (eventInfo.GetHealInfo()->GetSpellInfo()->SpellFamilyFlags & 0x0001000000000000)
                    originalSpell = sSpellMgr->GetSpellInfo(sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1, eventInfo.GetHealInfo()->GetSpellInfo()->GetRank()));
                else
                    originalSpell = eventInfo.GetHealInfo()->GetSpellInfo();

                if (originalSpell && aurEff->GetSpellInfo())
                {
                    Unit* target = eventInfo.GetActor(); // Paladin is the target of the energize
                    uint32 bp = CalculatePct(originalSpell->CalcPowerCost(target, originalSpell->GetSchoolMask()), aurEff->GetSpellInfo()->Effects[EFFECT_1].CalcValue());
                    CastSpellExtraArgs args(aurEff);
                    args.AddSpellBP0(bp);
                    target->CastSpell(target, SPELL_PALADIN_ILLUMINATION_ENERGIZE, args);
                }
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pal_illumination_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_illumination_AuraScript();
    }
};

// 37705 - Healing Discount
class spell_pal_item_healing_discount : public SpellScriptLoader
{
public:
    spell_pal_item_healing_discount() : SpellScriptLoader("spell_pal_item_healing_discount") { }

    class spell_pal_item_healing_discount_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_item_healing_discount_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PALADIN_ITEM_HEALING_TRANCE });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();
            GetTarget()->CastSpell(GetTarget(), SPELL_PALADIN_ITEM_HEALING_TRANCE, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pal_item_healing_discount_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_item_healing_discount_AuraScript();
    }
};

// -9799 - Eye for an Eye
class spell_pal_eye_for_an_eye : public SpellScriptLoader
{
public:
    spell_pal_eye_for_an_eye() : SpellScriptLoader("spell_pal_eye_for_an_eye") { }

    class spell_pal_eye_for_an_eye_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_eye_for_an_eye_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            SpellInfo const * procSpell = eventInfo.GetSpellInfo();
            // prevent damage back from weapon special attacks
            if (!procSpell || procSpell->DmgClass != SPELL_DAMAGE_CLASS_MAGIC)
                return false;

            return true;
        }

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            DamageInfo* damageInfo = eventInfo.GetDamageInfo();
            if (!damageInfo || !damageInfo->GetDamage())
                return;

            // return damage % to attacker but < 50% own total health
            int32 damage = std::min(CalculatePct(static_cast<int32>(damageInfo->GetDamage()), aurEff->GetAmount()), static_cast<int32>(GetTarget()->GetMaxHealth()) / 2);
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(damage);
            GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE, args);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_pal_eye_for_an_eye_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_pal_eye_for_an_eye_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_eye_for_an_eye_AuraScript();
    }
};

// 20185 - Judgement of Light
class spell_pal_judgement_of_light_heal : public SpellScriptLoader
{
public:
    spell_pal_judgement_of_light_heal() : SpellScriptLoader("spell_pal_judgement_of_light_heal") { }

    class spell_pal_judgement_of_light_heal_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_judgement_of_light_heal_AuraScript);

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            Unit* caster = eventInfo.GetProcTarget();

            CastSpellExtraArgs args(aurEff);
            args.OriginalCaster = GetCasterGUID();
#ifdef LICH_KING
            args.AddSpellBP0(caster->CountPctFromMaxHealth(aurEff->GetAmount()));
            caster->CastSpell(nullptr, SPELL_PALADIN_JUDGEMENT_OF_LIGHT_HEAL, args);
#else
            caster->CastSpell(nullptr, GetSpellInfo()->Effects[EFFECT_0].TriggerSpell, args);
#endif
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pal_judgement_of_light_heal_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_judgement_of_light_heal_AuraScript();
    }
};

// 31789 - Righteous Defense
class spell_pal_righteous_defense : public SpellScript
{
    PrepareSpellScript(spell_pal_righteous_defense);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        if (caster->GetTypeId() != TYPEID_PLAYER)
            return SPELL_FAILED_DONT_REPORT;

        if (Unit* target = GetExplTargetUnit())
        {
            if (!target->IsFriendlyTo(caster) || target == caster || target->GetAttackers().empty())
                return SPELL_FAILED_BAD_TARGETS;
        }
        else
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/, int32& damage)
    {
        if (Unit* target = GetHitUnit())
        {
            auto const& attackers = target->GetAttackers();

            std::vector<Unit*> list(attackers.cbegin(), attackers.cend());
            Trinity::Containers::RandomResize(list, 3);

            for (Unit* attacker : list)
                GetCaster()->CastSpell(attacker, SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT, TRIGGERED_FULL_MASK);
        }
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_pal_righteous_defense::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_pal_righteous_defense::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_paladin_spell_scripts()
{
    new spell_pal_judgement_of_light_heal();
    new spell_pal_judgement_of_command();
    new spell_pal_spiritual_attunement();
    new spell_pal_item_t6_trinket();
    new spell_pal_t3_6p_bonus();
    new spell_pal_seal_of_vengeance<SPELL_PALADIN_HOLY_VENGEANCE, SPELL_PALADIN_SEAL_OF_VENGEANCE_DAMAGE>("spell_pal_seal_of_vengeance");
    new spell_pal_seals();
    new spell_pal_seal_of_blood();
    new spell_pal_seal_of_righteousness();
    new spell_pal_illumination();
    new spell_pal_item_healing_discount();
    new spell_pal_eye_for_an_eye();
    RegisterSpellScript(spell_pal_righteous_defense);
}
