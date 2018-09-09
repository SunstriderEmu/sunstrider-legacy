#include "ScriptMgr.h"
#include "CreatureAIImpl.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "TemporarySummon.h"


/*####
## npc_snake_trap_serpents
####*/

enum HunterSpells
{
    SPELL_HUNTER_CRIPPLING_POISON = 30981, // Viper
    SPELL_HUNTER_DEADLY_POISON_PASSIVE = 34657, // Venomous Snake
    SPELL_HUNTER_MIND_NUMBING_POISON = 25810  // Viper
};

enum HunterCreatures
{
    NPC_HUNTER_VIPER = 19921
};

class npc_snake_trap_serpents : public CreatureScript
{
public:
    npc_snake_trap_serpents() : CreatureScript("npc_snake_trap_serpents") { }

    struct npc_pet_hunter_snake_trapAI : public ScriptedAI
    {
        npc_pet_hunter_snake_trapAI(Creature* creature) : ScriptedAI(creature), _isViper(false), _spellTimer(0) { }

        void JustEngagedWith(Unit* /*who*/) override { }

        void Reset() override
        {
            _isViper = me->GetEntry() == NPC_HUNTER_VIPER ? true : false;

            me->SetMaxHealth(uint32(107 * (me->GetLevel() - 40) * 0.025f));
            // Add delta to make them not all hit the same time
            me->SetAttackTime(BASE_ATTACK, me->GetAttackTime(BASE_ATTACK) + urandms(0, 6));

            if (!_isViper && !me->HasAura(SPELL_HUNTER_DEADLY_POISON_PASSIVE))
                DoCast(me, SPELL_HUNTER_DEADLY_POISON_PASSIVE, true);
        }

        // Redefined for random target selection:
        void MoveInLineOfSight(Unit* /*who*/) override { }

        void UpdateAI(uint32 diff) override
        {
            if (me->GetVictim() && me->GetVictim()->HasBreakableByDamageCrowdControlAura())
            { // don't break cc
                me->GetThreatManager().ClearFixate();
                me->InterruptNonMeleeSpells(false);
                me->AttackStop();
                return;
            }

            if (me->IsSummon() && !me->GetThreatManager().GetFixateTarget())
            { // find new target
                Unit* summoner = me->ToTempSummon()->GetSummoner();

                std::vector<Unit*> targets;
                for (std::pair<ObjectGuid const, PvPCombatReference*> const& pair : summoner->GetCombatManager().GetPvPCombatRefs())
                {
                    Unit* enemy = pair.second->GetOther(summoner);
                    if (!enemy->HasBreakableByDamageCrowdControlAura() && me->_CanCreatureAttack(enemy) == CAN_ATTACK_RESULT_OK && me->IsWithinDistInMap(enemy, me->GetAggroRange(enemy)))
                        targets.push_back(enemy);
                }

                if (targets.empty())
                    for (std::pair<ObjectGuid const, CombatReference*> const& pair : summoner->GetCombatManager().GetPvECombatRefs())
                    {
                        Unit* enemy = pair.second->GetOther(summoner);
                        if (!enemy->HasBreakableByDamageCrowdControlAura() && me->_CanCreatureAttack(enemy) == CAN_ATTACK_RESULT_OK && me->IsWithinDistInMap(enemy, me->GetAggroRange(enemy)))
                            targets.push_back(enemy);
                    }

                for (Unit* target : targets)
                    me->EngageWithTarget(target);

                if (!targets.empty())
                {
                    Unit* target = Trinity::Containers::SelectRandomContainerElement(targets);
                    me->GetThreatManager().FixateTarget(target);
                }
            }

            if (!UpdateVictim())
                return;

            // Viper
            if (_isViper)
            {
                if (_spellTimer <= diff)
                {
                    if (!urand(0, 2)) // 33% chance to cast
                        DoCastVictim(RAND(SPELL_HUNTER_MIND_NUMBING_POISON, SPELL_HUNTER_CRIPPLING_POISON));

                    _spellTimer = 3000;
                }
                else
                    _spellTimer -= diff;
            }

            DoMeleeAttackIfReady();
        }

    private:
        bool _isViper;
        uint32 _spellTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_pet_hunter_snake_trapAI(creature);
    }
};

void AddSC_hunter_pet_scripts()
{
    new npc_snake_trap_serpents();
}
