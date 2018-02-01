#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"

enum RogueSpells
{
    SPELL_ROGUE_BLADE_FLURRY_EXTRA_ATTACK = 22482,
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


void AddSC_rogue_spell_scripts()
{
    new spell_rog_blade_flurry();
}
