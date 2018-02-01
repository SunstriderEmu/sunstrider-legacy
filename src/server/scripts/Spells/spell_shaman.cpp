#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "Player.h"

//TODO: should this be moved to DB?
class spell_sham_imprvd_weapon_totems : public SpellScriptLoader
{
public:
    spell_sham_imprvd_weapon_totems() : SpellScriptLoader("spell_sham_imprvd_weapon_totems") { }

    class spell_sham_imprvd_weapon_totems_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sham_imprvd_weapon_totems_AuraScript);

        void HandleEffectCalcSpellMod(AuraEffect const* aurEff, SpellModifier*& spellMod)
        {
            if (!spellMod)
            {
                // + effect value for Aspect of the Viper
                spellMod = new SpellModifier(aurEff->GetBase());
                spellMod->op = SPELLMOD_EFFECT1;
                spellMod->type = SPELLMOD_PCT;
                spellMod->spellId = GetId();
                switch (aurEff->GetEffIndex())
                {
                case 0:
                    spellMod->mask = 0x00200000000LL;    // Windfury Totem
                    break;
                case 1:
                    spellMod->mask = 0x00400000000LL;    // Flametongue Totem
                    break;
                }
            }
            spellMod->value = aurEff->GetAmount();
        }

        void Register()
        {
            DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_sham_imprvd_weapon_totems_AuraScript::HandleEffectCalcSpellMod, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sham_imprvd_weapon_totems_AuraScript();
    }
};

void AddSC_shaman_spell_scripts()
{
    new spell_sham_imprvd_weapon_totems();
}
