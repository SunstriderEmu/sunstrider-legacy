

enum {
    SAY_TELEPORT            = 0,

    SPELL_MARK_OF_FROST     = 23182,
    SPELL_AURA_OF_FROST     = 23186,
    SPELL_MANASTORM         = 21097,
    SPELL_CHILL             = 21098,
    SPELL_FROSTBREATH       = 21099,
    SPELL_REFLECT           = 22067,
    SPELL_CLEAVE            = 8255,                     // Perhaps not right ID
    SPELL_ENRAGE            = 23537
};

enum {
    EV_MANASTORM,
    EV_CHILL,
    EV_BREATH,
    EV_TELEPORT,
    EV_REFLECT,
    EV_CLEAVE
};

class Boss_azuregos : public CreatureScript
{
public:
    Boss_azuregos() : CreatureScript("boss_azuregos_new") {}

    class Boss_azuregos_newAI : public ScriptedAI
    {
    public:
        Boss_azuregos_newAI(Creature* creature) : ScriptedAI(creature) {}
        
        EventMap events;

        void Reset()
        override {
            events.RescheduleEvent(EV_MANASTORM, urand(5000, 17000));
            events.RescheduleEvent(EV_CHILL, urand(10000, 30000));
            events.RescheduleEvent(EV_BREATH, urand(2000, 8000));
            events.RescheduleEvent(EV_TELEPORT, urand(30000, 30000));
            events.RescheduleEvent(EV_REFLECT, urand(15000, 30000));
            events.RescheduleEvent(EV_CLEAVE, 7000);
            
            enraged = false;
        }
        
        void KilledUnit(Unit* killed)
        override {
            me->AddAura(SPELL_MARK_OF_FROST, killed);
        }
        
        void SpellHit(Unit* who, const SpellInfo*)
        override {
            if (who->HasAuraEffect(SPELL_MARK_OF_FROST) && !who->HasAuraEffect(SPELL_AURA_OF_FROST))
                who->CastSpell(me, SPELL_AURA_OF_FROST, true);
        }

        void UpdateAI(uint32 const diff)
        override {
            if (!UpdateVictim())
                return;

            events.Update(diff);
                
            if (me->IsBelowHPPercent(25.0f) && !enraged) {
                me->CastSpell(me, SPELL_ENRAGE, true);
                enraged = true;
            }

            switch (events.GetEvent())
            {
                case EV_MANASTORM:
                {
                    if (me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_MANASTORM) == SPELL_CAST_OK)
                        events.RescheduleEvent(EV_MANASTORM, urand(7500, 12500));
                    break;
                }
                case EV_CHILL:
                {
                    if (me->CastSpell(me->GetVictim(), SPELL_CHILL) == SPELL_CAST_OK)
                        events.RescheduleEvent(EV_CHILL, urand(13000, 25000));
                    break;
                }
                case EV_BREATH:
                {
                    if (me->CastSpell(me->GetVictim(), SPELL_FROSTBREATH) == SPELL_CAST_OK)
                        events.RescheduleEvent(EV_BREATH, urand(1000, 15000));
                    break;
                }
                case EV_TELEPORT:
                {
                    float x, y, z;
                    me->GetNearPoint(me, x, y, z, me->GetObjectSize(), 5.0f, 0.0f);

                    std::list<Player*> players;
                    Trinity::AnyPlayerInObjectRangeCheck check(me, 25.0f);
                    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, check);
                    me->VisitNearbyGridObject(25.0f, searcher);
                    
                    for (std::list<Player*>::const_iterator itr = players.begin(); itr != players.end(); itr++) {
                        (*itr)->TeleportTo(me->GetMapId(), x, y, z, (*itr)->GetOrientation(), TELE_TO_NOT_LEAVE_COMBAT);
                        me->GetHostileRefManager().deleteReference((*itr));
                    }
                    
                    Talk(SAY_TELEPORT);
                    events.RescheduleEvent(EV_TELEPORT, 30000);

                    break;
                }
                case EV_REFLECT:
                {
                    if (me->CastSpell(me, SPELL_REFLECT) == SPELL_CAST_OK)
                        events.RescheduleEvent(EV_REFLECT, urand(20000, 35000));
                    break;
                }
                case EV_CLEAVE:
                {
                    if (me->CastSpell(me->GetVictim(), SPELL_CLEAVE))
                        events.RescheduleEvent(EV_CLEAVE, 7000);
                    break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
        
    private:
        bool enraged;
    };
    
    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Boss_azuregos_newAI(creature);
    }
};

void AddSC_boss_azuregos()
{
    new Boss_azuregos();
}
