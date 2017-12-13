#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellScript.h"

/*
 case 41967:
        {
            if (Unit* pet = m_caster->GetPet()) {
                if (m_caster->GetVictim())
                    pet->SendMeleeAttackStart(m_caster->GetVictim());
                pet->CastSpell(pet, 28305, true);
            }
            return;
        }
*/
class spell_shadowfiend : public SpellScriptLoader
{
public:
    spell_shadowfiend() : SpellScriptLoader("spell_shadowfiend") { }

    enum Shadowfiend_Spell
    {
        SPELL_MANA_LEECH = 28305,
    };

    class spell_shadowfiend_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_shadowfiend_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_MANA_LEECH))
                return false;
            return true;
        }

        void HandleAfterCast()
        {
            if (Unit* pet = GetCaster()->GetFirstMinion())
            {
                if (pet->GetEntry() != 19668) //shadowfiend npc
                    return;

                if (GetCaster()->GetVictim())
                    pet->EngageWithTarget(GetCaster()->GetVictim());

                pet->CastSpell(pet, SPELL_MANA_LEECH, true);
            }
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_shadowfiend_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_shadowfiend_SpellScript();
    }
};

void AddSC_priest_spell_scripts()
{
    new spell_shadowfiend();
}
