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
SDName: boss_cannon_master_willey
SD%Complete: 100
SDComment:
SDCategory: Stratholme
EndScriptData */


#include "def_stratholme.h"
#include "GameEventMgr.h"

struct Location
{
    float x, y, z, o;
};

static Location SpawnLocations[] =
{
    { 3553.85, -2945.88, 125.001, 0.592 },
    { 3559.20, -2952.92, 125.001, 0.592 },
    { 3552.41, -2948.66, 125.001, 0.592 },
    { 3555.65, -2953.51, 125.001, 0.592 },
    { 3547.92, -2950.97, 125.001, 0.592 },
    { 3553.09, -2952.12, 125.001, 0.592 },
    { 3552.72, -2957.77, 125.001, 0.592 },
    { 3547.15, -2953.16, 125.001, 0.592 },
    { 3550.20, -2957.43, 125.001, 0.592 }
};

enum eWilleySpells {
    SPELL_KNOCKAWAY     = 10101,
    SPELL_PUMMEL        = 15615,
    SPELL_SHOOT         = 16496
};
//#define SPELL_SUMMONCRIMSONRIFLEMAN    17279

struct boss_cannon_master_willeyAI : public ScriptedAI
{
    boss_cannon_master_willeyAI(Creature *c) : ScriptedAI(c), summons(me) {}

    uint32 KnockAway_Timer;
    uint32 Pummel_Timer;
    uint32 Shoot_Timer;
    uint32 SummonRifleman_Timer;
    
    SummonList summons;

    bool isEventActive()
    {
        const GameEventMgr::ActiveEvents& activeEvents = sGameEventMgr->GetActiveEventList();
        bool active = activeEvents.find(57) != activeEvents.end();

        return active;
    }

    void Reset() override
    {
        Shoot_Timer = 1000;
        Pummel_Timer = 7000;
        KnockAway_Timer = 11000;
        SummonRifleman_Timer = 10000;
        
        if (InstanceScript* pInstance = ((InstanceScript*)me->GetInstanceScript()))
            pInstance->SetData(TYPE_CANNONMASTER, FAIL);
            
        summons.DespawnAll();
        
        if (isEventActive())
            me->SetDisplayId(15733);
    }

    void JustDied(Unit* Victim) override
    {
        summons.DespawnAll();
    }
    
    void JustSummoned(Creature* pSummon) override
    {
        summons.Summon(pSummon);
        
        /*if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM,0))
            pSummon->AI()->AttackStart(target);*/
    }
    
    void SummonedCreatureDespawn(Creature* pSummon) override { summons.Despawn(pSummon); }

    void EnterCombat(Unit *who) override
    {
        if (InstanceScript* pInstance = ((InstanceScript*)me->GetInstanceScript()))
            pInstance->SetData(TYPE_CANNONMASTER, IN_PROGRESS);
    }
    
    void CannonFired()
    {
        summons.DespawnAll();
    }

    void UpdateAI(const uint32 diff) override
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        //Pummel
        if (Pummel_Timer < diff)
        {
            //Cast
            if (rand()%100 < 90) //90% chance to cast
                DoCast(me->GetVictim(),SPELL_PUMMEL);
            //12 seconds until we should cast this again
            Pummel_Timer = 12000;
        }else Pummel_Timer -= diff;

        //KnockAway
        if (KnockAway_Timer < diff)
        {
            //Cast
            if (rand()%100 < 80) //80% chance to cast
                DoCast(me->GetVictim(),SPELL_KNOCKAWAY);
            //14 seconds until we should cast this again
            KnockAway_Timer = 14000;
        }else KnockAway_Timer -= diff;

        //Shoot
        if (Shoot_Timer < diff)
        {
            //Cast
            DoCast(me->GetVictim(),SPELL_SHOOT);
            //1 seconds until we should cast this again
            Shoot_Timer = 1000;
        }else Shoot_Timer -= diff;

        //SummonRifleman
        if (SummonRifleman_Timer < diff)
        {
            //Cast
            switch (rand()%9)
            {
            case 0:
                me->SummonCreature(11054, SpawnLocations[0].x, SpawnLocations[0].y, SpawnLocations[0].z, SpawnLocations[0].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[1].x, SpawnLocations[1].y, SpawnLocations[1].z, SpawnLocations[1].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[3].x, SpawnLocations[3].y, SpawnLocations[3].z, SpawnLocations[3].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                break;
            case 1:
                me->SummonCreature(11054, SpawnLocations[1].x, SpawnLocations[1].y, SpawnLocations[1].z, SpawnLocations[1].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[2].x, SpawnLocations[2].y, SpawnLocations[2].z, SpawnLocations[2].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[4].x, SpawnLocations[4].y, SpawnLocations[4].z, SpawnLocations[4].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                break;
            case 2:
                me->SummonCreature(11054, SpawnLocations[2].x, SpawnLocations[2].y, SpawnLocations[2].z, SpawnLocations[2].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[3].x, SpawnLocations[3].y, SpawnLocations[3].z, SpawnLocations[3].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[5].x, SpawnLocations[5].y, SpawnLocations[5].z, SpawnLocations[5].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                break;
            case 3:
                me->SummonCreature(11054, SpawnLocations[3].x, SpawnLocations[3].y, SpawnLocations[3].z, SpawnLocations[3].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[4].x, SpawnLocations[4].y, SpawnLocations[4].z, SpawnLocations[4].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[6].x, SpawnLocations[6].y, SpawnLocations[6].z, SpawnLocations[6].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                break;
            case 4:
                me->SummonCreature(11054, SpawnLocations[4].x, SpawnLocations[4].y, SpawnLocations[4].z, SpawnLocations[4].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[5].x, SpawnLocations[5].y, SpawnLocations[5].z, SpawnLocations[5].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[7].x, SpawnLocations[7].y, SpawnLocations[7].z, SpawnLocations[7].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                break;
            case 5:
                me->SummonCreature(11054, SpawnLocations[5].x, SpawnLocations[5].y, SpawnLocations[5].z, SpawnLocations[5].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[6].x, SpawnLocations[6].y, SpawnLocations[6].z, SpawnLocations[6].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[8].x, SpawnLocations[8].y, SpawnLocations[8].z, SpawnLocations[8].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                break;
            case 6:
                me->SummonCreature(11054, SpawnLocations[6].x, SpawnLocations[6].y, SpawnLocations[6].z, SpawnLocations[6].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[7].x, SpawnLocations[7].y, SpawnLocations[7].z, SpawnLocations[7].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[0].x, SpawnLocations[0].y, SpawnLocations[0].z, SpawnLocations[0].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                break;
            case 7:
                me->SummonCreature(11054, SpawnLocations[7].x, SpawnLocations[7].y, SpawnLocations[7].z, SpawnLocations[7].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[8].x, SpawnLocations[8].y, SpawnLocations[8].z, SpawnLocations[8].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[1].x, SpawnLocations[1].y, SpawnLocations[1].z, SpawnLocations[1].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                break;
            case 8:
                me->SummonCreature(11054, SpawnLocations[8].x, SpawnLocations[8].y, SpawnLocations[8].z, SpawnLocations[8].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[0].x, SpawnLocations[0].y, SpawnLocations[0].z, SpawnLocations[0].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                me->SummonCreature(11054, SpawnLocations[2].x, SpawnLocations[2].y, SpawnLocations[2].z, SpawnLocations[2].o, TEMPSUMMON_TIMED_DESPAWN, 240000);
                break;
            }
            //30 seconds until we should cast this again
            SummonRifleman_Timer = 30000;
        }else SummonRifleman_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_cannon_master_willey(Creature *_Creature)
{
    return new boss_cannon_master_willeyAI (_Creature);
}

class ScarletCannon : public GameObjectScript
{
public:
    ScarletCannon() : GameObjectScript("go_scarlet_cannon")
    {}

    bool OnGossipHello(Player* pPlayer, GameObject* pGo) override
    {
        if (Creature* willey = pGo->FindNearestCreature(10997, 10.0f, true)) {
            CAST_AI(boss_cannon_master_willeyAI, (willey->AI()))->CannonFired();
            pGo->Respawn();

            return false;
        }

        return true;
    }
};

void AddSC_boss_cannon_master_willey()
{
    OLDScript *newscript;
    
    newscript = new OLDScript;
    newscript->Name="boss_cannon_master_willey";
    newscript->GetAI = &GetAI_boss_cannon_master_willey;
    sScriptMgr->RegisterOLDScript(newscript);
    
    new ScarletCannon();
}

