#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Player.h"
#include "SpellAuraEffects.h"

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
        void HandleDummy(SpellEffIndex /*effIndex*/, int32& /*damage*/)
        {
            if (Unit* unitTarget = GetHitUnit())
                GetCaster()->CastSpell(unitTarget, GetEffectValue(), true);
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_pal_judgement_of_command_SpellScript::CheckTarget);
            OnEffectHitTarget += SpellEffectFn(spell_pal_judgement_of_command_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_pal_judgement_of_command_SpellScript();
    }
};

void AddSC_paladin_spell_scripts()
{
    new spell_pal_judgement_of_command();
}
