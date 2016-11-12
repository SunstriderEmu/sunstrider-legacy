/* ScriptData
SDName: Boss_Curator
SD%Complete: 95
SDComment: Need to check timers.
SDCategory: Karazhan
EndScriptData */

#include "def_karazhan.h"

enum CuratorData
{
    // Texts
    SAY_AGGRO                       = -1532057,
    SAY_SUMMON1                     = -1532058,
    SAY_SUMMON2                     = -1532059,
    SAY_EVOCATE                     = -1532060,
    SAY_ENRAGE                      = -1532061,
    SAY_KILL1                       = -1532062,
    SAY_KILL2                       = -1532063,
    SAY_DEATH                       = -1532064,

    // Spells
    // -- Curator
    SPELL_HATEFUL_BOLT              = 30383,
    SPELL_EVOCATION                 = 30254,
    SPELL_ENRAGE                    = 30403,    // Arcane Infusion: Transforms Curator and adds damage.
    SPELL_BERSERK                   = 26662,

    // -- Astral Flare
    SPELL_ASTRAL_FLARE_PASSIVE      = 30234,    // Visual effect + Flare damage
    SPELL_ASTRAL_FLARE_VISUAL       = 30237,
//    SPELL_ASTRAL_FLARE_NE           = 30236,    // Not in DBC
//    SPELL_ASTRAL_FLARE_NW           = 30239,    // Not in DBC
//    SPELL_ASTRAL_FLARE_SE           = 30240,    // Not in DBC
//    SPELL_ASTRAL_FLARE_SW           = 30241,    // Not in DBC

    // Creatures
    NPC_ASTRAL_FLARE                = 17096
};

class boss_curator : public CreatureScript
{
  public:
    boss_curator() : CreatureScript("boss_curator") { }

    class boss_curatorAI : public CreatureAI
    {
      public:
        boss_curatorAI(Creature* creature) : CreatureAI(creature) 
        {
            instance = ((InstanceScript*)creature->GetInstanceScript());
            if (instance)
                instance->SetData(DATA_CURATOR_EVENT, NOT_STARTED);
        }
        
        EventMap events;

        enum CuratorEvents
        {
            EVENT_FLARE_SUMMON = 1,
            EVENT_HATEFUL_BOLT,
            EVENT_BERSERK
        };

        enum CuratorPhases
        {
            PHASE_NORMAL = 1,
            PHASE_ENRAGED,
        };

        void Reset()
        override {
            events.Reset();

            events.RescheduleEvent(EVENT_FLARE_SUMMON, 10 * IN_MILLISECONDS, PHASE_NORMAL);
            events.RescheduleEvent(EVENT_HATEFUL_BOLT, 15 * IN_MILLISECONDS, PHASE_NORMAL);
            events.RescheduleEvent(EVENT_BERSERK, 10 * MINUTE * IN_MILLISECONDS, PHASE_NORMAL);

            me->SetFullTauntImmunity(true);
        }

        void KilledUnit(Unit* /*victim*/)
        override {
            switch (rand()%2)
            {
                case 0: DoScriptText(SAY_KILL1, me); break;
                case 1: DoScriptText(SAY_KILL2, me); break;
            }
        }

        void JustDied(Unit* /*killer*/) 
        override {
            DoScriptText(SAY_DEATH, me);

            if (instance)
                instance->SetData(DATA_CURATOR_EVENT, DONE);
        }
        
        void EnterCombat(Unit* /*victim*/)
        override {
            DoScriptText(SAY_AGGRO, me);
            DoZoneInCombat();

            if (instance)
                instance->SetData(DATA_CURATOR_EVENT, IN_PROGRESS);
        }

        void JustSummoned(Creature* summoned)
        override {
            if (summoned->GetEntry() == NPC_ASTRAL_FLARE)
            {
                // Flare start with aggro on it's target, should be immune to arcane
                summoned->CastSpell(summoned, SPELL_ASTRAL_FLARE_PASSIVE, true);
                summoned->CastSpell(summoned, SPELL_ASTRAL_FLARE_VISUAL, true);
                summoned->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ARCANE, true);

                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    summoned->AI()->AttackStart(target);
            }
        }

        void JustReachedHome()
        override {
            if (instance)
                if(instance->GetData(DATA_CURATOR_EVENT != DONE))
                    instance->SetData(DATA_CURATOR_EVENT, FAIL);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
        override {
            if (me->GetHealthPct() < 15.0f)
            {
                events.SetPhase(PHASE_ENRAGED);

                me->InterruptNonMeleeSpells(false);
                me->CastSpell(me, SPELL_ENRAGE);
                DoScriptText(SAY_ENRAGE, me);
            }
        }

        void UpdateAI(uint32 const diff)
            override {
            if (!UpdateVictim())
                return;

            // not supposed to do anything while evocate
            if (me->HasAuraEffect(SPELL_EVOCATION))
                return;

            events.Update(diff);

            switch (events.GetEvent())
            {
                case 0:
                    break;
                case EVENT_BERSERK:
                {
                    if (me->CastSpell(me, SPELL_BERSERK) == SPELL_CAST_OK)
                        events.RescheduleEvent(EVENT_BERSERK, 1 * MINUTE * IN_MILLISECONDS);
                    break;
                }
                case EVENT_FLARE_SUMMON:
                {
                    // Summon Astral Flare
                    me->SummonCreature(NPC_ASTRAL_FLARE, me->GetPositionX() + rand()%37, me->GetPositionY() + rand()%37, me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5 * IN_MILLISECONDS);

                    // reduce mana by 10% of maximum
                    if (int32 iMana = me->GetMaxPower(POWER_MANA))
                    {
                        me->ModifyPower(POWER_MANA, -(iMana / 10));

                        // if this get's us below 10%, then we evocate (the 10th should be summoned now
                        if (me->GetPower(POWER_MANA) * 10 < me->GetMaxPower(POWER_MANA))
                        {
                            if (me->CastSpell(me, SPELL_EVOCATION) == SPELL_CAST_OK)
                            {
                                DoScriptText(SAY_EVOCATE, me);
                                // this small delay should make first flare appear fast after evocate, and also prevent possible spawn flood
                                events.RescheduleEvent(EVENT_FLARE_SUMMON, 1 * IN_MILLISECONDS, PHASE_NORMAL);
                            }

                            return;
                        }
                        else
                        {
                            switch (urand(0, 3))
                            {
                                case 0: DoScriptText(SAY_SUMMON1, me); break;
                                case 1: DoScriptText(SAY_SUMMON2, me); break;
                            }
                        }
                    }
                    break;
                }
                case EVENT_HATEFUL_BOLT:
                {
                    Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO, 1);
                    if (target)
                        if(me->CastSpell(target, SPELL_HATEFUL_BOLT) == SPELL_CAST_OK)
                            events.RescheduleEvent(EVENT_HATEFUL_BOLT, 15 * IN_MILLISECONDS, PHASE_NORMAL);
                    break;
                }
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }

      private:
        InstanceScript *instance;
    };
    
    CreatureAI* GetAI(Creature* creature) const
    override {
        return new boss_curatorAI(creature);
    }
};

void AddSC_boss_curator()
{
    new boss_curator();
}

