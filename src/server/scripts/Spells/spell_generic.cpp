#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"

enum CannibalizeSpells
{
    SPELL_CANNIBALIZE_TRIGGERED = 20578
};

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

class spell_gen_cannibalize : public SpellScriptLoader
{
public:
    spell_gen_cannibalize() : SpellScriptLoader("spell_gen_cannibalize") { }

    class spell_gen_cannibalize_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gen_cannibalize_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_CANNIBALIZE_TRIGGERED))
                return false;
            return true;
        }

        SpellCastResult CheckIfCorpseNear()
        {
            Unit* caster = GetCaster();
            float max_range = GetSpellInfo()->GetMaxRange(false);
            WorldObject* result = nullptr;
            // search for nearby enemy corpse in range
            Trinity::CannibalizeObjectCheck u_check(caster, max_range);
            Trinity::WorldObjectSearcher<Trinity::CannibalizeObjectCheck > searcher(caster, result, u_check);
            Cell::VisitGridObjects(caster, searcher, max_range);
            if (!result)
                Cell::VisitWorldObjects(caster, searcher, max_range);

            if (!result)
                return SPELL_FAILED_NO_EDIBLE_CORPSES;
            return SPELL_CAST_OK;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/, int32& /*damage*/)
        {
            GetCaster()->CastSpell(GetCaster(), SPELL_CANNIBALIZE_TRIGGERED, TRIGGERED_NONE);
        }

        void Register() override
        {
            OnEffectHit += SpellEffectFn(spell_gen_cannibalize_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnCheckCast += SpellCheckCastFn(spell_gen_cannibalize_SpellScript::CheckIfCorpseNear);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_gen_cannibalize_SpellScript();
    }
};

// 20594 - Dwarf racial stoneform - Only armor and poison resistance are in aura effects on BC, need to add both bleed and disease
class spell_racial_stoneform : public SpellScriptLoader
{
public:
    spell_racial_stoneform() : SpellScriptLoader("spell_racial_stoneform") { }

    class spell_racial_stoneform_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_racial_stoneform_AuraScript)

        void HandleEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            GetSpellInfo()->ApplyDispelImmune(GetTarget(), DISPEL_DISEASE, true);
            GetSpellInfo()->ApplyMechanicImmune(GetTarget(), (1 << MECHANIC_BLEED), true);
        }

        void HandleEffectRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            GetSpellInfo()->ApplyDispelImmune(GetTarget(), DISPEL_DISEASE, false);
            GetSpellInfo()->ApplyMechanicImmune(GetTarget(), (1 << MECHANIC_BLEED), false);
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_racial_stoneform_AuraScript::HandleEffectApply, EFFECT_1, SPELL_AURA_DISPEL_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_racial_stoneform_AuraScript::HandleEffectRemove, EFFECT_1, SPELL_AURA_DISPEL_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_racial_stoneform_AuraScript();
    }
};

class spell_archimonde_doomfire : public SpellScriptLoader
{
public:
    spell_archimonde_doomfire() : SpellScriptLoader("spell_archimonde_doomfire") { }

    class spell_archimonde_doomfire_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_archimonde_doomfire_AuraScript)

        //MARCH PA + utiliser le tick count pour les degats + prendre les degats du spell parent
        void PeriodicTick(AuraEffect const* aurEff)
        {
            int32 dmg = aurEff->GetAmount() - aurEff->GetTickNumber() * 150;
            if (dmg < 0)
                return;
            CastSpellExtraArgs args(GetTarget()->GetGUID());
            args.AddSpellBP0(int32(dmg));
            GetTarget()->CastSpell(GetTarget(), 31969, args);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_archimonde_doomfire_AuraScript::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_archimonde_doomfire_AuraScript();
    }
};


void AddSC_generic_spell_scripts()
{
    new spell_gen_cannibalize();
    new spell_racial_stoneform();
    new spell_gen_proc_below_pct_damaged("spell_item_commendation_of_kaelthas");
    new spell_archimonde_doomfire();
}
