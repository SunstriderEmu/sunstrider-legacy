#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"

enum WarlockSpells
{
    SPELL_WARLOCK_PYROCLASM_PROC           = 18093,
    SPELL_WARLOCK_DRAIN_SOUL_ENERGIZE      = 18371,
    SPELL_WARLOCK_SEED_OF_CORRUPTION_PROC  = 27285,
};

// -18096 - Pyroclasm (talent)
class spell_warl_pyroclasm : public SpellScriptLoader
{
public:
    spell_warl_pyroclasm() : SpellScriptLoader("spell_warl_pyroclasm") { }

    class spell_warl_pyroclasm_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pyroclasm_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            Unit* target = eventInfo.GetActionTarget();
            Spell const* procSpell = eventInfo.GetProcSpell();
            if (!target || !target->IsAlive() || target == GetCaster() || procSpell == nullptr)
                return false;
            
            //WoWWiki: Since Rain of Fire and Hellfire do damage over time, the chance to stun is distributed over all the ticks of the spell, not 13%/26% per tick.
            uint32 tickCount = 1;
            // Hellfire have 15 tick
            if (procSpell->GetSpellInfo()->SpellFamilyFlags & 0x0000000000000040LL)
                tickCount = 15;
            // Rain of Fire have 4 tick
            else if (procSpell->GetSpellInfo()->SpellFamilyFlags & 0x0000000000000020LL)
                tickCount = 4;  // was 4
            //else soulfire : 0x8000000000, default to 1 tick

            float chance = 0.0f;
            switch (GetAura()->GetId())
            {
            case 18096: chance = 13.0f / tickCount; break; //rank 1
            case 18073: chance = 26.0f / tickCount; break; //rank 2
            }
            if (!roll_chance_f(chance))
                return false;

            return true;
        }

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_WARLOCK_PYROCLASM_PROC });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* target = eventInfo.GetActionTarget();
            if(GetCaster() && target)
                GetCaster()->CastSpell(target, SPELL_WARLOCK_PYROCLASM_PROC, aurEff);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_warl_pyroclasm_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_warl_pyroclasm_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_warl_pyroclasm_AuraScript();
    }
};

// -1120 - Drain soul, improved drain soul talent handling
class spell_warl_drainsoul : public SpellScriptLoader
{
public:
    spell_warl_drainsoul() : SpellScriptLoader("spell_warl_drainsoul") { }

    class spell_warl_drainsoul_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_drainsoul_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_WARLOCK_DRAIN_SOUL_ENERGIZE });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (!GetCaster() || !GetTarget())
                return;

            uint32 basepoints0 = 0;
            Unit::AuraEffectList const& mAddFlatModifier = GetCaster()->GetAuraEffectsByType(SPELL_AURA_ADD_FLAT_MODIFIER);
            for (auto i : mAddFlatModifier)
            {
                if (i->GetMiscValue() == SPELLMOD_CHANCE_OF_SUCCESS && i->GetSpellInfo()->SpellIconID == 113) //improved drain soul (18213 and 18372)
                {
                    int32 value = GetCaster()->CalculateSpellDamage(GetCaster(), i->GetSpellInfo(), EFFECT_2, &i->GetSpellInfo()->Effects[2].BasePoints);
                    basepoints0 = value * GetCaster()->GetMaxPower(POWER_MANA) / 100;
                }
            }
            if (basepoints0 == 0)
                return;
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(basepoints0);
            GetCaster()->CastSpell(GetTarget(), SPELL_WARLOCK_DRAIN_SOUL_ENERGIZE, args);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_warl_drainsoul_AuraScript::HandleProc, EFFECT_2, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_warl_drainsoul_AuraScript();
    }
};

// -27285 - Seed of Corruption proc - exclude aura target
class spell_warl_seed_of_corruption_proc : public SpellScriptLoader
{
public:
    spell_warl_seed_of_corruption_proc() : SpellScriptLoader("spell_warl_seed_of_corruption_proc") { }

    class spell_warl_seed_of_corruption_proc_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_warl_seed_of_corruption_proc_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (GetExplTargetUnit())
                targets.remove(GetExplTargetUnit());
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_seed_of_corruption_proc_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_warl_seed_of_corruption_proc_SpellScript();
    }
};

// -27243 - Seed of corruption
class spell_warl_seed_of_corruption : public SpellScriptLoader
{
public:
    spell_warl_seed_of_corruption() : SpellScriptLoader("spell_warl_seed_of_corruption") { }

    class spell_warl_seed_of_corruption_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_seed_of_corruption_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_WARLOCK_SEED_OF_CORRUPTION_PROC });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            // if damage is more than need or target die from damage deal finish spell
            if (aurEff->GetAmount() <= eventInfo.GetDamageInfo()->GetDamage() || GetTarget()->GetHealth() <= eventInfo.GetDamageInfo()->GetDamage())
            {
                // Remove our seed aura before casting
                GetTarget()->RemoveAura(aurEff->GetId(), aurEff->GetCasterGUID());
                // Cast finish spell
                if (GetCaster())
                    GetCaster()->CastSpell(GetTarget(), SPELL_WARLOCK_SEED_OF_CORRUPTION_PROC, true);
                return;
            }

            // Damage counting
            AuraEffect* auraEffect = const_cast<AuraEffect*>(aurEff);
            auraEffect->SetAmount(aurEff->GetAmount() - eventInfo.GetDamageInfo()->GetDamage());
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_warl_seed_of_corruption_AuraScript::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_warl_seed_of_corruption_AuraScript();
    }
};

void AddSC_warlock_spell_scripts()
{
    new spell_warl_pyroclasm();
    new spell_warl_drainsoul();
    new spell_warl_seed_of_corruption();
    new spell_warl_seed_of_corruption_proc();
}
