#define MAX_RANGE 75.0f
#define SPELL_SHOOT 22121
#define SPELL_INFINITE_ROOT 53107
#define TIMER_SHOOT urand(2000, 3000)


class alterac_bowman : public CreatureScript
{
public:
    alterac_bowman() : CreatureScript("alterac_bowman")
    { }

    class alterac_bowmanAI : public ScriptedAI
    {
        public:
        alterac_bowmanAI(Creature *c) : ScriptedAI(c) 
        {   
            shoot_timer = 0;
            me->SetCombatDistance(80.0f); //Disable melee visual
            SetCombatMovementAllowed(false);
            me->SetSheath(SHEATH_STATE_RANGED);
        }
    
        uint16 shoot_timer;
        ObjectGuid rangedTargetGUID;
        ObjectGuid lastMeleeUnitGUID;
    
        void JustReachedHome() override 
        {
           me->AddAura(SPELL_INFINITE_ROOT, me); //this creature can't be displaced even via CM
        }
    
        void Reset() override 
        {   
            rangedTargetGUID = ObjectGuid::Empty;
        }

        void UpdateAI(const uint32 diff) override 
        {    
            // Melee handling
            if (Unit* victim = me->GetVictim())
            {
                if (me->IsWithinMeleeRange(victim))
                {
                    DoMeleeAttackIfReady();
                    return;
                }
                else 
                {
                    if (Unit* lastMeleeUnit = ObjectAccessor::GetUnit(*me, lastMeleeUnitGUID))
                    {
                        if (me->IsWithinMeleeRange(lastMeleeUnit) && me->IsValidAttackTarget(lastMeleeUnit))
                        {
                            if (me->GetVictim() != lastMeleeUnit)
                                AttackStart(lastMeleeUnit);

                            DoMeleeAttackIfReady();
                            return;
                        }
                    }
                    lastMeleeUnitGUID = ObjectGuid::Empty;
                }
            }

            //okay, no target in melee, let's try to pick a distant one

            if (rangedTargetGUID)
            {
                Unit* rangedTarget = ObjectAccessor::GetUnit(*me, rangedTargetGUID);
                if (!rangedTarget || !IsValidRangedTarget(rangedTarget))
                {
                    rangedTarget = nullptr;
                    rangedTargetGUID = ObjectGuid::Empty;
                }

                if (rangedTarget)
                {
                    if (shoot_timer < diff)
                    {
                        if (me->GetVictim() != rangedTarget)
                        {
                            bool attackOk = me->Attack(rangedTarget, false);
                            if (!attackOk)
                            {
                                rangedTarget = nullptr;
                                rangedTargetGUID = ObjectGuid::Empty;
                                return;
                            }
                        }

                        // me->SetInFront(target);
                        if (DoCast(rangedTarget, SPELL_SHOOT, false) == SPELL_CAST_OK)
                        {
                            shoot_timer = TIMER_SHOOT;
                        }
                        else 
                        {
                            rangedTarget = nullptr;
                            rangedTargetGUID = ObjectGuid::Empty;
                        }
                    }
                    else
                        shoot_timer -= diff;
                }
            }

            if(!rangedTargetGUID)
                UpdateVictim(); //Evade if not in combat
        }
        
        void MoveInLineOfSight(Unit *who) override 
        {    
            if (rangedTargetGUID && me->IsWithinMeleeRange(who) && me->CanAggro(who) == CAN_ATTACK_RESULT_OK)
                lastMeleeUnitGUID = who->GetGUID();

            if (!rangedTargetGUID
                && IsValidRangedTarget(who))
            {
                rangedTargetGUID = who->GetGUID();
            }
        }

        bool IsValidRangedTarget(Unit* target)
        {
            float distance = me->GetDistance(target);
            if (me->CanAggro(target) == CAN_ATTACK_RESULT_OK
                && (distance < MAX_RANGE) 
                && !me->IsWithinMeleeRange(target)
                && me->IsWithinLOSInMap(target))
                    return true;
            
            return false;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new alterac_bowmanAI(creature);
    }
};


void AddSC_alterac_bowman()
{
    new alterac_bowman();
}

