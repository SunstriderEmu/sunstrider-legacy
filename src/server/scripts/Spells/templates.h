#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"

class spell_gen_proc_below_pct_damaged : public SpellScriptLoader
{
public:
    spell_gen_proc_below_pct_damaged(char const* name) : SpellScriptLoader(name) { }

    class spell_gen_proc_below_pct_damaged_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_gen_proc_below_pct_damaged_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            DamageInfo* damageInfo = eventInfo.GetDamageInfo();
            if (!damageInfo || !damageInfo->GetDamage())
                return false;

            int32 pct = GetSpellInfo()->Effects[EFFECT_0].CalcValue();

            if (eventInfo.GetActionTarget()->HealthBelowPctDamaged(pct, damageInfo->GetDamage()))
                return true;

            return false;
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_gen_proc_below_pct_damaged_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_gen_proc_below_pct_damaged_AuraScript();
    }
};