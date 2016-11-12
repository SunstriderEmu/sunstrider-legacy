/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Boss_Nightbane
SD%Complete: 80
SDComment: SDComment: Timers may incorrect
SDCategory: Karazhan
EndScriptData */


#include "def_karazhan.h"

//phase 1
#define SPELL_BELLOWING_ROAR        39427
#define SPELL_CHARRED_EARTH         30129
#define SPELL_DISTRACTING_ASH       30130
#define SPELL_SMOLDERING_BREATH     30210
#define SPELL_TAIL_SWEEP            25653
#define SPELL_CLEAVE                30131
//phase 2
#define SPELL_RAIN_OF_BONES         37098
#define SPELL_SMOKING_BLAST         37057
#define SPELL_FIREBALL_BARRAGE      30282
#define SPELL_SEARING_CINDERS       30127
#define SPELL_SUMMON_SKELETON       30170

#define EMOTE_SUMMON                "Un être ancien apparait au loin..."
#define YELL_AGGRO                  "Pauvres fous ! Je devrais apporter une fin rapide à votre souffrance !"
#define YELL_FLY_PHASE              "Misérable vermine. Je vais vous exterminer depuis les airs !"
#define YELL_LAND_PHASE_1           "Assez ! Je vais atterrir et vous écraser moi-même !"
#define YELL_LAND_PHASE_2           "Insectes ! Laissez-moi vous montrer ma force !"
#define EMOTE_BREATH                "Plaie-de-nuit prend une profonde respiration."

float IntroWay[8][3] =
{
    {-11053.37,-1794.48,149},
    {-11141.07,-1841.40,125},
    {-11187.28,-1890.23,125},
    {-11189.20,-1931.25,125},
    {-11153.76,-1948.93,125},
    {-11128.73,-1929.75,125},
    {-11140   , -1915  ,122},
    {-11163   , -1903  ,91.473}
};

struct boss_nightbaneAI : public ScriptedAI
{
    boss_nightbaneAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
        Intro = true;
    }

    InstanceScript* pInstance;

    uint32 Phase;

    bool RainBones;
    bool Skeletons;

    uint32 BellowingRoarTimer;
    uint32 CharredEarthTimer;
    uint32 DistractingAshTimer;
    uint32 SmolderingBreathTimer;
    uint32 TailSweepTimer;
    uint32 RainofBonesTimer;
    uint32 SmokingBlastTimer;
    uint32 FireballBarrageTimer;
    uint32 SearingCindersTimer;
    uint32 CleaveTimer;

    uint32 FlyCount;
    uint32 FlyTimer;

    bool Intro;
    bool Flying;
    bool Movement;

    uint32 WaitTimer;
    uint32 MovePhase;

    void Reset()
    override {
        BellowingRoarTimer = 30000;
        CharredEarthTimer = 15000;
        DistractingAshTimer = 20000;
        SmolderingBreathTimer = 10000;
        TailSweepTimer = 12000;
        RainofBonesTimer = 10000;
        SmokingBlastTimer = 20000;
        FireballBarrageTimer = 13000;
        SearingCindersTimer = 14000;
        CleaveTimer = 10000;            // Guessed
        WaitTimer = 1000;

        Phase =1;
        FlyCount = 0;
        MovePhase = 0;

        me->SetSpeedRate(MOVE_RUN, 2.0f);
        me->SetDisableGravity(true);
        me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
        me->SetKeepActive(30*MINUTE*IN_MILLISECONDS);

        me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);

        if(pInstance->GetData(DATA_NIGHTBANE_EVENT) == DONE || pInstance->GetData(DATA_NIGHTBANE_EVENT) == IN_PROGRESS)
        {
            me->DealDamage(me, me->GetMaxHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            me->RemoveCorpse();
        }
        else
        {
            if(pInstance)
                pInstance->SetData(DATA_NIGHTBANE_EVENT, NOT_STARTED);
        }

        HandleTerraceDoors(true);

        Flying = false;
        Movement = false;

        if(!Intro)
        {
            me->SetHomePosition(IntroWay[7][0],IntroWay[7][1],IntroWay[7][2],0);
            me->GetMotionMaster()->MoveTargetedHome();
        }
    }

    void HandleTerraceDoors(bool open)
    {
        if(GameObject *Door = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_MASTERS_TERRACE_DOOR_1)))
            Door->SetUInt32Value(GAMEOBJECT_STATE, open ? 0 : 1);
        if(GameObject *Door = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_MASTERS_TERRACE_DOOR_2)))
            Door->SetUInt32Value(GAMEOBJECT_STATE, open ? 0 : 1);
    }

    void EnterCombat(Unit *who)
    override {
        if(pInstance)
            pInstance->SetData(DATA_NIGHTBANE_EVENT, IN_PROGRESS);

        HandleTerraceDoors(false);
        me->Yell(YELL_AGGRO, LANG_UNIVERSAL, nullptr);
    }

    void AttackStart(Unit* who)
    override {
        if(!Intro && !Flying)
            ScriptedAI::AttackStart(who);
    }

    void JustDied(Unit* killer)
    override {
        if(pInstance)
            pInstance->SetData(DATA_NIGHTBANE_EVENT, DONE);

        HandleTerraceDoors(true);
    }

    void MoveInLineOfSight(Unit *who)
    override {
        if(!Intro && !Flying)
        {
            if(!me->GetVictim() && me->CanAggro(who) == CAN_ATTACK_RESULT_OK)
                ScriptedAI::AttackStart(who);
        }
    }

    void MovementInform(uint32 type, uint32 id)
    override {
        if(type != POINT_MOTION_TYPE)
                return;

        if(Intro)
        {
            if(id >= 8)
            {
                Intro = false;
                me->SetHomePosition(IntroWay[7][0],IntroWay[7][1],IntroWay[7][2],0);
                return;
            }

            WaitTimer = 1;
        }

        if(Flying)
        {
            if(id == 0)
            {
                me->TextEmote(EMOTE_BREATH, nullptr, true);
                Flying = false;
                Phase = 2;
                return;
            }

            if(id == 3)
            {
                MovePhase = 4;
                WaitTimer = 1;
                return;
            }

            if(id == 8)
            {
                Flying = false;
                Phase = 1;
                Movement = true;
                return;
            }

            WaitTimer = 1;
        }
    }

    void JustSummoned(Creature *summoned)
    override {
        summoned->AI()->AttackStart(me->GetVictim());
    }

    void TakeOff()
    {
        me->Yell(YELL_FLY_PHASE, LANG_UNIVERSAL, nullptr);

        me->InterruptSpell(CURRENT_GENERIC_SPELL);
        me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
        me->SetDisableGravity(true);
        (*me).GetMotionMaster()->Clear(false);
        (*me).GetMotionMaster()->MovePoint(0,IntroWay[2][0],IntroWay[2][1],IntroWay[2][2]);

        Flying = true;

        FlyTimer = 45000+rand()%15000; //timer wrong between 45 and 60 seconds
        ++FlyCount;

        RainofBonesTimer = 5000; //timer wrong (maybe)
        RainBones = false;
        Skeletons = false;
     }

    void UpdateAI(const uint32 diff)
    override {
        me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);

        if(WaitTimer)
        {
            if(WaitTimer <= diff)
            {
                if(Intro)
                {
                    if(MovePhase >= 7)
                    {
                        me->SetDisableGravity(false);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                        me->GetMotionMaster()->MovePoint(8,IntroWay[7][0],IntroWay[7][1],IntroWay[7][2]);
                    }
                    else
                    {
                        me->GetMotionMaster()->MovePoint(MovePhase,IntroWay[MovePhase][0],IntroWay[MovePhase][1],IntroWay[MovePhase][2]);
                        ++MovePhase;
                    }
                }

                if(Flying)
                {
                    if(MovePhase >= 7)
                    {
                        me->SetDisableGravity(false);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                        me->GetMotionMaster()->MovePoint(8,IntroWay[7][0],IntroWay[7][1],IntroWay[7][2]);
                    }
                    else
                    {
                        me->GetMotionMaster()->MovePoint(MovePhase,IntroWay[MovePhase][0],IntroWay[MovePhase][1],IntroWay[MovePhase][2]);
                        ++MovePhase;
                    }
                }

                WaitTimer = 0;
            } else { WaitTimer -= diff; }
        }

        if(!UpdateVictim())
            return;

        if(Flying)
            return;

        //  Phase 1 "GROUND FIGHT"
        if(Phase == 1)
        {
            if(Movement)
            {
                DoStartMovement(me->GetVictim());
                Movement = false;
            }

            if (BellowingRoarTimer < diff)
            {
                DoCast(me->GetVictim(),SPELL_BELLOWING_ROAR);
                BellowingRoarTimer = 30000+rand()%10000 ; //Timer
            }else BellowingRoarTimer -= diff;

            if (SmolderingBreathTimer < diff)
            {
                DoCast(me->GetVictim(),SPELL_SMOLDERING_BREATH);
                SmolderingBreathTimer = 20000;//timer
            }else SmolderingBreathTimer -= diff;

            if (CharredEarthTimer < diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target,SPELL_CHARRED_EARTH);
                CharredEarthTimer = 20000; //timer
            }else CharredEarthTimer -= diff;

            if (TailSweepTimer < diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    if (!me->HasInArc( M_PI, target))
                        DoCast(target,SPELL_TAIL_SWEEP);
                TailSweepTimer = 15000;//timer
            }else TailSweepTimer -= diff;

            if (SearingCindersTimer < diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target,SPELL_SEARING_CINDERS);
                SearingCindersTimer = 10000; //timer
            }else SearingCindersTimer -= diff;
            
            if (CleaveTimer < diff)
            {
                if (me->GetVictim())
                    DoCast(me->GetVictim(), SPELL_CLEAVE);
                CleaveTimer = 10000;
            }else CleaveTimer -= diff;

            uint32 Prozent;
            Prozent = (me->GetHealth()*100) / me->GetMaxHealth();

            if (Prozent < 75 && FlyCount == 0) // first take off 75%
                TakeOff();

            if (Prozent < 50 && FlyCount == 1) // secound take off 50%
                TakeOff();

            if (Prozent < 25 && FlyCount == 2) // third take off 25%
                TakeOff();

            DoMeleeAttackIfReady();
        }

        //Phase 2 "FLYING FIGHT"
        if (Phase == 2)
        {
            if (!RainBones)
            {
                if (!Skeletons)
                {
                    for (uint8 i = 0; i <= 3; ++i)
                    {
                        me->InterruptNonMeleeSpells(true);
                        DoCast(me->GetVictim(), SPELL_SUMMON_SKELETON);
                        Skeletons = true;
                    }
                }

                if (RainofBonesTimer < diff && !RainBones) // only once at the beginning of phase 2
                {
                    DoCast(me->GetVictim(),SPELL_RAIN_OF_BONES);
                    RainBones = true;
                    SmokingBlastTimer = 20000;
                }else RainofBonesTimer -= diff;

                if (DistractingAshTimer < diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(target,SPELL_DISTRACTING_ASH);
                    DistractingAshTimer = 2000;//timer wrong
                }else DistractingAshTimer -= diff;
            }

            if (RainBones)
            {
                if (SmokingBlastTimer < diff)
                 {
                    DoCast(me->GetVictim(),SPELL_SMOKING_BLAST);
                    SmokingBlastTimer = 1500 ; //timer wrong
                 }else SmokingBlastTimer -= diff;
            }

            if (FireballBarrageTimer < diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0))
                    DoCast(target,SPELL_FIREBALL_BARRAGE);
                FireballBarrageTimer = 20000; //Timer
            }else FireballBarrageTimer -= diff;

            if (FlyTimer < diff) //landing
            {
                if(rand()%2 == 0)
                    me->Yell(YELL_LAND_PHASE_1, LANG_UNIVERSAL, nullptr);
                else
                    me->Yell(YELL_LAND_PHASE_2, LANG_UNIVERSAL, nullptr);

                (*me).GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MovePoint(3,IntroWay[3][0],IntroWay[3][1],IntroWay[3][2]);

                Flying = true;
            }else FlyTimer -= diff;
        }
    }
};

CreatureAI* GetAI_boss_nightbane(Creature *_Creature)
{
    return new boss_nightbaneAI (_Creature);
}

void AddSC_boss_nightbane()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name="boss_nightbane";
    newscript->GetAI = &GetAI_boss_nightbane;
    sScriptMgr->RegisterOLDScript(newscript);
}

