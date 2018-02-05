#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"

enum RogueSpells
{
    SPELL_ROGUE_BLADE_FLURRY_EXTRA_ATTACK       = 22482,
    SPELL_ROGUE_DEADLY_THROW_INTERRUPT_PROC     = 32747,
    SPELL_ROGUE_QUICK_RECOVERY_ENERGY           = 31663,
    SPELL_ROGUE_MASTER_OF_SUBTLETY_BUFF         = 31665,
    SPELL_ROGUE_SHIV_TRIGGERED                  =  5940,
    SPELL_ROGUE_CHEAT_DEATH_COOLDOWN            = 31231,
};

// 13877, 33735, (check 51211, 65956) - Blade Flurry
class spell_rog_blade_flurry : public SpellScriptLoader
{
public:
    spell_rog_blade_flurry() : SpellScriptLoader("spell_rog_blade_flurry") { }

    class spell_rog_blade_flurry_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_blade_flurry_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_ROGUE_BLADE_FLURRY_EXTRA_ATTACK });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            _procTarget = eventInfo.GetActor()->SelectNearbyTarget(eventInfo.GetProcTarget());
            return _procTarget != nullptr;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (DamageInfo* damageInfo = eventInfo.GetDamageInfo())
            {
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(damageInfo->GetDamage());
                GetTarget()->CastSpell(_procTarget, SPELL_ROGUE_BLADE_FLURRY_EXTRA_ATTACK, args);
            }
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_rog_blade_flurry_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_rog_blade_flurry_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_MOD_MELEE_HASTE);
        }

        Unit* _procTarget = nullptr;
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_rog_blade_flurry_AuraScript();
    }
};

// 28716 - Dreamwalker set 2 pieces
// "Your Rejuvenation ticks have a chance to restore 60 mana, 8 energy, or 2 rage to your target."
class spell_rog_deadly_throw_interrupt : public SpellScriptLoader
{
public:
    spell_rog_deadly_throw_interrupt() : SpellScriptLoader("spell_rog_deadly_throw_interrupt") { }

    class spell_rog_deadly_throw_interrupt_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_deadly_throw_interrupt_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_ROGUE_DEADLY_THROW_INTERRUPT_PROC });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            // Prevent cast Deadly Throw Interrupt on self from last effect (apply dummy) of Deadly Throw
            if (GetTarget() == eventInfo.GetProcTarget())
                return false;

            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_ROGUE_DEADLY_THROW_INTERRUPT_PROC, aurEff);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_rog_deadly_throw_interrupt_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_rog_deadly_throw_interrupt_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_rog_deadly_throw_interrupt_AuraScript();
    }
};

// -31244 - Quick Recovery
class spell_rog_quick_recovery : public SpellScriptLoader
{
public:
    spell_rog_quick_recovery() : SpellScriptLoader("spell_rog_quick_recovery") { }

    class spell_rog_quick_recovery_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_quick_recovery_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_ROGUE_QUICK_RECOVERY_ENERGY });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            // Prevent cast Deadly Throw Interrupt on self from last effect (apply dummy) of Deadly Throw
            if (GetTarget() == eventInfo.GetProcTarget())
                return false;

            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
            if (!spellInfo)
                return;

            Unit* caster = eventInfo.GetActor();
            int32 amount = CalculatePct(spellInfo->CalcPowerCost(caster, spellInfo->GetSchoolMask()), aurEff->GetAmount());
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(amount);
            caster->CastSpell(nullptr, SPELL_ROGUE_QUICK_RECOVERY_ENERGY, args);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_rog_quick_recovery_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_rog_quick_recovery_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_rog_quick_recovery_AuraScript();
    }
};

void AddSC_rogue_spell_scripts()
{
    new spell_rog_blade_flurry();
    new spell_rog_deadly_throw_interrupt();
    new spell_rog_quick_recovery();
}
