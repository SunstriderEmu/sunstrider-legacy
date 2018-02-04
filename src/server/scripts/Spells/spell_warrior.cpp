#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"

enum WarriorSpells
{
    SPELL_RELATIATION_PROC            = 22858,
    SPELL_SECOND_WIND_PROC_RNK1       = 29841,
    SPELL_SECOND_WIND_PROC_RNK2       = 29842,
};

// 5246 - Intimidating Shout - remove main target from effect 1 and 2
class spell_warr_intimidating_shout : public SpellScriptLoader
{
public:
    spell_warr_intimidating_shout() : SpellScriptLoader("spell_warr_intimidating_shout") { }

    class spell_warr_intimidating_shout_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_warr_intimidating_shout_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove(GetExplTargetWorldObject());
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warr_intimidating_shout_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warr_intimidating_shout_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_warr_intimidating_shout_SpellScript();
    }
};

// 20230 - Warrior relatiation - attacks in back do not enable proc
class spell_warr_retaliation : public SpellScriptLoader
{
public:
    spell_warr_retaliation() : SpellScriptLoader("spell_warr_retaliation") { }

    class spell_warr_retaliation_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warr_retaliation_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_RELATIATION_PROC });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            // check attack comes not from behind
            if (!GetCaster()->HasInArc(M_PI, eventInfo.GetActor()) || GetCaster()->HasUnitState(UNIT_STATE_STUNNED))
                return false;

            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            GetCaster()->CastSpell(eventInfo.GetActor(), SPELL_RELATIATION_PROC, aurEff);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_warr_retaliation_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_warr_retaliation_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_warr_retaliation_AuraScript();
    }
};

// -29834 - Second wind
class spell_warr_second_wind : public SpellScriptLoader
{
public:
    spell_warr_second_wind() : SpellScriptLoader("spell_warr_second_wind") { }

    class spell_warr_second_wind_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warr_second_wind_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_SECOND_WIND_PROC_RNK1, SPELL_SECOND_WIND_PROC_RNK2 });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            Spell const* procSpell = eventInfo.GetProcSpell();
            // only for spells and hit/crit (trigger start always) and not start from self casted spells (5530 Mace Stun Effect for example)
            if (procSpell == nullptr || !(eventInfo.GetHitMask() & (PROC_HIT_NORMAL | PROC_HIT_CRITICAL)) || GetCaster() == eventInfo.GetActor())
                return false;

            return procSpell->GetSpellInfo()->GetAllEffectsMechanicMask() & ((1 << MECHANIC_ROOT) | (1 << MECHANIC_STUN));
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            uint32 triggered_spell_id;
            switch (GetSpellInfo()->Id)
            {
            case 29834: triggered_spell_id = SPELL_SECOND_WIND_PROC_RNK1; break;
            case 29838: triggered_spell_id = SPELL_SECOND_WIND_PROC_RNK2; break;
            default:
                TC_LOG_ERROR("spell", "spell_warr_second_wind non handled spell id: %u", GetSpellInfo()->Id);
                return;
            }

            PreventDefaultAction();
            GetCaster()->CastSpell(GetCaster(), triggered_spell_id, aurEff);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_warr_second_wind_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_warr_second_wind_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_warr_second_wind_AuraScript();
    }
};

void AddSC_warrior_spell_scripts()
{
    new spell_warr_intimidating_shout();
    new spell_warr_retaliation();
    new spell_warr_second_wind();
}
