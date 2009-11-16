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
        Player* player = GetPlayerForEscort();

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
            if (Player* player = GetPlayerForEscort())
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

enum eEnums
{
    SAY_DS_START        = -1000402,
    SAY_DS_DOWN_1       = -1000403,
    SAY_DS_DOWN_2       = -1000404,
    SAY_DS_DOWN_3       = -1000405,
    SAY_DS_PROLOGUE     = -1000406,

    SPELL_SHOOT         = 6660,
    QUEST_TOME_VALOR    = 1651,
    NPC_DEFIAS_RAIDER   = 6180,
    EQUIP_ID_RIFLE      = 2511
};

struct TRINITY_DLL_DECL npc_daphne_stilwellAI : public npc_escortAI
{
    npc_daphne_stilwellAI(Creature* pCreature) : npc_escortAI(pCreature) {}

    uint32 uiWPHolder;
    uint32 uiShootTimer;

    void Reset()
    {
        if (IsBeingEscorted)
        {
            switch(uiWPHolder)
            {
                case 7: DoScriptText(SAY_DS_DOWN_1, m_creature); break;
                case 8: DoScriptText(SAY_DS_DOWN_2, m_creature); break;
                case 9: DoScriptText(SAY_DS_DOWN_3, m_creature); break;
            }
        }
        else
            uiWPHolder = 0;

        uiShootTimer = 0;
    }
    
    void Aggro(Unit* who) {}

    void WaypointReached(uint32 uiPoint)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        uiWPHolder = uiPoint;

        switch(uiPoint)
        {
            case 4:
                SetEquipmentSlots(false, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE, EQUIP_ID_RIFLE);
                m_creature->SetSheath(SHEATH_STATE_RANGED);
                m_creature->HandleEmoteCommand(EMOTE_STATE_USESTANDING_NOSHEATHE);
                break;
            case 7:
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836, 1569.755, 54.267, 4.230, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697, 1569.124, 54.421, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237, 1568.307, 54.620, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 8:
                m_creature->SetSheath(SHEATH_STATE_RANGED);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836, 1569.755, 54.267, 4.230, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697, 1569.124, 54.421, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237, 1568.307, 54.620, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11448.037, 1570.213, 54.961, 4.283, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 9:
                m_creature->SetSheath(SHEATH_STATE_RANGED);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836, 1569.755, 54.267, 4.230, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697, 1569.124, 54.421, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237, 1568.307, 54.620, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11448.037, 1570.213, 54.961, 4.283, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                m_creature->SummonCreature(NPC_DEFIAS_RAIDER, -11449.018, 1570.738, 54.828, 4.220, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 10:
                SetRun(false);
                break;
            case 11:
                DoScriptText(SAY_DS_PROLOGUE, m_creature);
                break;
            case 13:
                SetEquipmentSlots(true);
                m_creature->SetSheath(SHEATH_STATE_UNARMED);
                m_creature->HandleEmoteCommand(EMOTE_STATE_USESTANDING_NOSHEATHE);
                break;
            case 17:
                pPlayer->GroupEventHappens(QUEST_TOME_VALOR, m_creature);
                break;
        }
    }

    void AttackStart(Unit* pWho)
    {
        if (!pWho)
            return;

        if (m_creature->Attack(pWho, false))
        {
            m_creature->AddThreat(pWho, 0.0f);
            m_creature->SetInCombatWith(pWho);
            pWho->SetInCombatWith(m_creature);

            m_creature->GetMotionMaster()->MoveChase(pWho, 30.0f);
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->AI()->AttackStart(m_creature);
    }

    void Update(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        if (uiShootTimer <= diff)
        {
            uiShootTimer = 1500;

            //if (!m_creature->IsWithinDist(m_creature->getVictim(), ATTACK_DISTANCE))
            if (m_creature->GetDistance2d(m_creature->getVictim()) > ATTACK_DISTANCE)
                DoCast(m_creature->getVictim(), SPELL_SHOOT);
            else
                DoMeleeAttackIfReady();
        } else uiShootTimer -= diff;
        
        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_daphne_stilwell(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_TOME_VALOR)
    {
        DoScriptText(SAY_DS_START, pCreature);

        if (npc_escortAI* pEscortAI = CAST_AI(npc_daphne_stilwellAI, pCreature->AI()))
            pEscortAI->Start(true, true, true, pPlayer->GetGUID());
    }

    return true;
}

CreatureAI* GetAI_npc_daphne_stilwell(Creature* pCreature)
{
    npc_daphne_stilwellAI* daphneAI = new npc_daphne_stilwellAI(pCreature);
    
    daphneAI->AddWaypoint(0, -11480.684570, 1545.091187, 49.898571),
    daphneAI->AddWaypoint(1, -11466.825195, 1530.151733, 50.263611),
    daphneAI->AddWaypoint(2, -11465.213867, 1528.343750, 50.954369),
    daphneAI->AddWaypoint(3, -11462.990234, 1525.235596, 50.937702),
    daphneAI->AddWaypoint(4, -11461.000000, 1526.614014, 50.937702, 5000),
    daphneAI->AddWaypoint(5, -11462.990234, 1525.235596, 50.937702),
    daphneAI->AddWaypoint(6, -11465.213867, 1528.343750, 50.954369),
    daphneAI->AddWaypoint(7, -11468.353516, 1535.075562, 50.400948, 15000),
    daphneAI->AddWaypoint(8, -11468.353516, 1535.075562, 50.400948, 15000),
    daphneAI->AddWaypoint(9, -11468.353516, 1535.075562, 50.400948, 10000),
    daphneAI->AddWaypoint(10, -11467.898438, 1532.459595, 50.348885),
    daphneAI->AddWaypoint(11, -11466.064453, 1529.855225, 50.209351),
    daphneAI->AddWaypoint(12, -11462.990234, 1525.235596, 50.937702),
    daphneAI->AddWaypoint(13, -11461.000000, 1526.614014, 50.937702, 5000),
    daphneAI->AddWaypoint(14, -11462.990234, 1525.235596, 50.937702),
    daphneAI->AddWaypoint(15, -11465.213867, 1528.343750, 50.954369),
    daphneAI->AddWaypoint(16, -11470.260742, 1537.276733, 50.378487),
    daphneAI->AddWaypoint(17, -11475.581055, 1548.678833, 50.184380),
    daphneAI->AddWaypoint(18, -11482.299805, 1557.410034, 48.624519);
    
    return (CreatureAI*)daphneAI;
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

