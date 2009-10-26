/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Westfall
SD%Complete: 90
SDComment: Quest support: 155, 1651
SDCategory: Westfall
EndScriptData */

/* ContentData
npc_defias_traitor
npc_daphne_stilwell
EndContentData */

#include "precompiled.h"
#include "../../npc/npc_escortAI.h"

/*######
## npc_defias_traitor
######*/

#define SAY_START                   -1000101
#define SAY_PROGRESS                -1000102
#define SAY_END                     -1000103
#define SAY_AGGRO_1                 -1000104
#define SAY_AGGRO_2                 -1000105

#define QUEST_DEFIAS_BROTHERHOOD    155

struct TRINITY_DLL_DECL npc_defias_traitorAI : public npc_escortAI
{
    npc_defias_traitorAI(Creature *c) : npc_escortAI(c) {}

    bool IsWalking;

    void WaypointReached(uint32 i)
    {
        Player* player = Unit::GetPlayer(PlayerGUID);

        if (!player)
            return;

        if (IsWalking && !m_creature->HasUnitMovementFlag(MOVEMENTFLAG_WALK_MODE))
            m_creature->AddUnitMovementFlag(MOVEMENTFLAG_WALK_MODE);

        switch (i)
        {
            case 35:
                IsWalking = true;
                break;
            case 36:
                DoScriptText(SAY_PROGRESS, m_creature, player);
                break;
            case 44:
                DoScriptText(SAY_END, m_creature, player);
                {
                    if (player && player->GetTypeId() == TYPEID_PLAYER)
                        ((Player*)player)->GroupEventHappens(QUEST_DEFIAS_BROTHERHOOD,m_creature);
                }
                break;
        }
    }
    void Aggro(Unit* who)
    {
        switch(rand()%2)
        {
            case 0: DoScriptText(SAY_AGGRO_1, m_creature, who); break;
            case 1: DoScriptText(SAY_AGGRO_2, m_creature, who); break;
        }
    }

    void Reset()
    {
        if (IsWalking && !m_creature->HasUnitMovementFlag(MOVEMENTFLAG_WALK_MODE))
        {
            m_creature->AddUnitMovementFlag(MOVEMENTFLAG_WALK_MODE);
            return;
        }
        IsWalking = false;
    }

    void JustDied(Unit* killer)
    {
        if (PlayerGUID)
        {
            if (Player* player = Unit::GetPlayer(PlayerGUID))
                player->FailQuest(QUEST_DEFIAS_BROTHERHOOD);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
    }
};

bool QuestAccept_npc_defias_traitor(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_DEFIAS_BROTHERHOOD)
    {
        ((npc_escortAI*)(creature->AI()))->Start(true, true, true, player->GetGUID());
        DoScriptText(SAY_START, creature, player);
    }

    return true;
}

CreatureAI* GetAI_npc_defias_traitor(Creature *_Creature)
{
    npc_defias_traitorAI* thisAI = new npc_defias_traitorAI(_Creature);

    thisAI->AddWaypoint(0, -10508.40, 1068.00, 55.21);
    thisAI->AddWaypoint(1, -10518.30, 1074.84, 53.96);
    thisAI->AddWaypoint(2, -10534.82, 1081.92, 49.88);
    thisAI->AddWaypoint(3, -10546.51, 1084.88, 50.13);
    thisAI->AddWaypoint(4, -10555.29, 1084.45, 45.75);
    thisAI->AddWaypoint(5, -10566.57, 1083.53, 42.10);
    thisAI->AddWaypoint(6, -10575.83, 1082.34, 39.46);
    thisAI->AddWaypoint(7, -10585.67, 1081.08, 37.77);
    thisAI->AddWaypoint(8, -10600.08, 1078.19, 36.23);
    thisAI->AddWaypoint(9, -10608.69, 1076.08, 35.88);
    thisAI->AddWaypoint(10, -10621.26, 1073.00, 35.40);
    thisAI->AddWaypoint(11, -10638.12, 1060.18, 33.61);
    thisAI->AddWaypoint(12, -10655.87, 1038.99, 33.48);
    thisAI->AddWaypoint(13, -10664.68, 1030.54, 32.70);
    thisAI->AddWaypoint(14, -10708.68, 1033.86, 33.32);
    thisAI->AddWaypoint(15, -10754.43, 1017.93, 32.79);
    thisAI->AddWaypoint(16, -10802.26, 1018.01, 32.16);
    thisAI->AddWaypoint(17, -10832.60, 1009.04, 32.71);
    thisAI->AddWaypoint(18, -10866.56, 1006.51, 31.71);     // Fix waypoints from roughly this point, test first to get proper one
    thisAI->AddWaypoint(19, -10879.98, 1005.10, 32.84);
    thisAI->AddWaypoint(20, -10892.45, 1001.32, 34.46);
    thisAI->AddWaypoint(21, -10906.14, 997.11, 36.15);
    thisAI->AddWaypoint(22, -10922.26, 1002.23, 35.74);
    thisAI->AddWaypoint(23, -10936.32, 1023.38, 36.52);
    thisAI->AddWaypoint(24, -10933.35, 1052.61, 35.85);
    thisAI->AddWaypoint(25, -10940.25, 1077.66, 36.49);
    thisAI->AddWaypoint(26, -10957.09, 1099.33, 36.83);
    thisAI->AddWaypoint(27, -10956.53, 1119.90, 36.73);
    thisAI->AddWaypoint(28, -10939.30, 1150.75, 37.42);
    thisAI->AddWaypoint(29, -10915.14, 1202.09, 36.55);
    thisAI->AddWaypoint(30, -10892.59, 1257.03, 33.37);
    thisAI->AddWaypoint(31, -10891.93, 1306.66, 35.45);
    thisAI->AddWaypoint(32, -10896.17, 1327.86, 37.77);
    thisAI->AddWaypoint(33, -10906.03, 1368.05, 40.91);
    thisAI->AddWaypoint(34, -10910.18, 1389.33, 42.62);
    thisAI->AddWaypoint(35, -10915.42, 1417.72, 42.93);
    thisAI->AddWaypoint(36, -10926.37, 1421.18, 43.04);     // walk here and say
    thisAI->AddWaypoint(37, -10952.31, 1421.74, 43.40);
    thisAI->AddWaypoint(38, -10980.04, 1411.38, 42.79);
    thisAI->AddWaypoint(39, -11006.06, 1420.47, 43.26);
    thisAI->AddWaypoint(40, -11021.98, 1450.59, 43.09);
    thisAI->AddWaypoint(41, -11025.36, 1491.59, 43.15);
    thisAI->AddWaypoint(42, -11036.09, 1508.32, 43.28);
    thisAI->AddWaypoint(43, -11060.68, 1526.72, 43.19);
    thisAI->AddWaypoint(44, -11072.75, 1527.77, 43.20, 5000);// say and quest credit

    return (CreatureAI*)thisAI;
}

/*######
## npc_daphne_stilwell
######*/

#define DEFIAS_RAIDER       6180

#define QUEST_TOME_VALOR    1651

struct Locations
{
    float x, y, z, o;
};

static Locations RaidersSpawnPoints[]=
{
    {-11429.18, 1610.41, 71.70, 4.26},
    {-11422.66, 1614.23, 74.14, 4.16},
    {-11425.00, 1615.71, 73.47, 4.24},
    {-11427.75, 1617.06, 73.15, 4.27},
    {-11430.20, 1618.12, 72.66, 4.31}
};

struct TRINITY_DLL_DECL npc_daphne_stilwellAI : public ScriptedAI
{
    npc_daphne_stilwellAI(Creature* c) : ScriptedAI(c) {}
    
    uint32 waveTimer;
    uint8 wave;
    
    bool eventRunning; //defines if the event is running or not
    
    Player* pPlayer;
    
    void Reset()
    {
        waveTimer = 5000; //is 30 sec enough ?
        wave = 1; //begin with first wave
        eventRunning = false;
    }
    
    void SetEventRunning(bool run)
    {
        eventRunning = run;
    }
    
    void SetPlayerDoingQuest(Player* player)
    {
        if (player)
        {
            pPlayer = player;
            m_creature->GetMotionMaster()->Clear(); //Daphne follows some waypoints when she's out of any event. I want her to stay where she is!
        }
    }
    
    void Aggro(Unit* who)
    {
        //say something ?
    }
    
    void UpdateAI(const uint32 diff)
    {
        if (!eventRunning)
            return;
            
        if (waveTimer < diff)
        {
            switch (wave)
            {
                case 1:
                    //spawn 3 raiders
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[0].x, RaidersSpawnPoints[0].y, RaidersSpawnPoints[0].z, RaidersSpawnPoints[0].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[1].x, RaidersSpawnPoints[1].y, RaidersSpawnPoints[1].z, RaidersSpawnPoints[1].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[2].x, RaidersSpawnPoints[2].y, RaidersSpawnPoints[2].z, RaidersSpawnPoints[2].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    wave++;
                    break;
                case 2:
                    //spawn 4 raiders
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[0].x, RaidersSpawnPoints[0].y, RaidersSpawnPoints[0].z, RaidersSpawnPoints[0].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[1].x, RaidersSpawnPoints[1].y, RaidersSpawnPoints[1].z, RaidersSpawnPoints[1].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[2].x, RaidersSpawnPoints[2].y, RaidersSpawnPoints[2].z, RaidersSpawnPoints[2].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[3].x, RaidersSpawnPoints[3].y, RaidersSpawnPoints[3].z, RaidersSpawnPoints[2].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    wave++;
                    break;
                case 3:
                    //spawn 5 raiders
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[0].x, RaidersSpawnPoints[0].y, RaidersSpawnPoints[0].z, RaidersSpawnPoints[0].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[1].x, RaidersSpawnPoints[1].y, RaidersSpawnPoints[1].z, RaidersSpawnPoints[1].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[2].x, RaidersSpawnPoints[2].y, RaidersSpawnPoints[2].z, RaidersSpawnPoints[2].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[3].x, RaidersSpawnPoints[3].y, RaidersSpawnPoints[3].z, RaidersSpawnPoints[3].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    m_creature->SummonCreature(DEFIAS_RAIDER, RaidersSpawnPoints[4].x, RaidersSpawnPoints[4].y, RaidersSpawnPoints[4].z, RaidersSpawnPoints[4].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(m_creature);
                    wave++;
                    break;
                default: //event finished, complete quest for player
                    if (pPlayer)
                        pPlayer->AreaExploredOrEventHappens(QUEST_TOME_VALOR);
                    SetEventRunning(false);
                    return;
            }
            
            waveTimer = 30000;
        }else waveTimer -= diff;
        
        m_creature->GetMotionMaster()->Clear(); //without this, Daphne begins to move again between the waves
        
        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_daphne_stilwell(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_TOME_VALOR)
    {
        ((npc_daphne_stilwellAI*)creature->AI())->SetEventRunning(true);
        creature->Say("Faites attention, ils arrivent !", LANG_UNIVERSAL, 0);
        ((npc_daphne_stilwellAI*)creature->AI())->SetPlayerDoingQuest(player);
    }
}

CreatureAI* GetAI_npc_daphne_stilwell(Creature *pCreature)
{
    return new npc_daphne_stilwellAI(pCreature);
}

/*######
## AddSC
######*/

void AddSC_westfall()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_defias_traitor";
    newscript->GetAI = &GetAI_npc_defias_traitor;
    newscript->pQuestAccept = &QuestAccept_npc_defias_traitor;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_daphne_stilwell";
    newscript->GetAI = &GetAI_npc_daphne_stilwell;
    newscript->pQuestAccept = &QuestAccept_npc_daphne_stilwell;
    newscript->RegisterSelf();
}

