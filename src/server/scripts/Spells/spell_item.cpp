#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "templates.h"


// Serpent-Coil Braid item (30720)
// 37447 - Improved Mana Gems	
// "You gain 25 % more mana when you use a mana gem.In addition, using a mana gem grants you 5097 spell power for 15 sec."
class spell_item_improved_mana_gems : public SpellScriptLoader
{
public:
    enum Spells
    {
        SPELL_MANA_SURGE = 37445,
    };

    spell_item_improved_mana_gems() : SpellScriptLoader("spell_item_improved_mana_gems") { }

    class spell_item_improved_mana_gems_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_improved_mana_gems_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_MANA_SURGE });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();
            GetTarget()->CastSpell(GetTarget(), SPELL_MANA_SURGE, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_item_improved_mana_gems_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_item_improved_mana_gems_AuraScript();
    }
};

enum PetHealing
{
    SPELL_HEALTH_LINK = 37382
};

// 37381 - Pet Healing
// Hunter T5 2P Bonus
// Warlock T5 2P Bonus
class spell_item_pet_healing : public AuraScript
{
    PrepareAuraScript(spell_item_pet_healing);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HEALTH_LINK });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        Unit* actor = eventInfo.GetActor();
        Unit* target = actor->GetPet();
        if (!target)
            return;

        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(CalculatePct(damageInfo->GetDamage(), aurEff->GetAmount()));
        eventInfo.GetActor()->CastSpell(target, SPELL_HEALTH_LINK, args);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_pet_healing::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


void AddSC_item_spell_scripts()
{
    new spell_item_improved_mana_gems();
    new spell_gen_proc_below_pct_damaged("spell_item_commendation_of_kaelthas");
    RegisterAuraScript(spell_item_pet_healing);
}
