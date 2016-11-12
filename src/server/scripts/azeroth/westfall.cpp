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


#include "EscortAI.h"

/*######
## npc_defias_traitor
######*/

#define SAY_START                   -1000101
#define SAY_PROGRESS                -1000102
#define SAY_END                     -1000103
#define SAY_AGGRO_1                 -1000104
#define SAY_AGGRO_2                 -1000105

#define QUEST_DEFIAS_BROTHERHOOD    155

struct npc_defias_traitorAI : public npc_escortAI
{
    npc_defias_traitorAI(Creature *c) : npc_escortAI(c) {}

    bool IsWalking;
    bool complete;

    void WaypointReached(uint32 i)
    override {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        if (IsWalking && !me->HasUnitMovementFlag(MOVEMENTFLAG_WALKING))
            me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);

        switch (i)
        {
            case 35:
                IsWalking = true;
                break;
            case 36:
                DoScriptText(SAY_PROGRESS, me, player);
                break;
            case 44:
                DoScriptText(SAY_END, me, player);
                {
                    if (player && player->GetTypeId() == TYPEID_PLAYER)
                        (player->ToPlayer())->GroupEventHappens(QUEST_DEFIAS_BROTHERHOOD,me);
                        
                    complete = true;
                }
                break;
        }
    }
    void EnterCombat(Unit* who)
    override {
        switch(rand()%2)
        {
            case 0: DoScriptText(SAY_AGGRO_1, me, who); break;
            case 1: DoScriptText(SAY_AGGRO_2, me, who); break;
        }
    }

    void Reset()
    override {
        if (IsWalking && !me->HasUnitMovementFlag(MOVEMENTFLAG_WALKING))
        {
            me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
            return;
        }
        IsWalking = false;
        complete = false;
    }

    void JustDied(Unit* killer)
    override {
        if (PlayerGUID && !complete)
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(QUEST_DEFIAS_BROTHERHOOD);
        }
    }

    void UpdateAI(const uint32 diff)
    override {
        npc_escortAI::UpdateAI(diff);
    }
};

bool QuestAccept_npc_defias_traitor(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_DEFIAS_BROTHERHOOD)
    {
        ((npc_escortAI*)(creature->AI()))->Start(true, true, true, player->GetGUID(), creature->GetEntry());
        DoScriptText(SAY_START, creature, player);
    }

    return true;
}

CreatureAI* GetAI_npc_defias_traitor(Creature *pCreature)
{
    return new npc_defias_traitorAI(pCreature);
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

struct npc_daphne_stilwellAI : public npc_escortAI
{
    npc_daphne_stilwellAI(Creature* pCreature) : npc_escortAI(pCreature) {}

    uint32 uiWPHolder;
    uint32 uiShootTimer;

    void Reset()
    override {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            switch(uiWPHolder)
            {
                case 7: DoScriptText(SAY_DS_DOWN_1, me); break;
                case 8: DoScriptText(SAY_DS_DOWN_2, me); break;
                case 9: DoScriptText(SAY_DS_DOWN_3, me); break;
            }
        }
        else
            uiWPHolder = 0;

        uiShootTimer = 0;
    }
    
    void EnterCombat(Unit* who) override {}

    void WaypointReached(uint32 uiPoint)
    override {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        uiWPHolder = uiPoint;

        switch(uiPoint)
        {
            case 4:
                SetEquipmentSlots(false, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE, EQUIP_ID_RIFLE);
                me->SetSheath(SHEATH_STATE_RANGED);
                me->HandleEmoteCommand(EMOTE_STATE_USESTANDING_NO_SHEATHE);
                break;
            case 7:
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836, 1569.755, 54.267, 4.230, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697, 1569.124, 54.421, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237, 1568.307, 54.620, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 8:
                me->SetSheath(SHEATH_STATE_RANGED);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836, 1569.755, 54.267, 4.230, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697, 1569.124, 54.421, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237, 1568.307, 54.620, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.037, 1570.213, 54.961, 4.283, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 9:
                me->SetSheath(SHEATH_STATE_RANGED);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11450.836, 1569.755, 54.267, 4.230, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.697, 1569.124, 54.421, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.237, 1568.307, 54.620, 4.206, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11448.037, 1570.213, 54.961, 4.283, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DEFIAS_RAIDER, -11449.018, 1570.738, 54.828, 4.220, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 10:
                SetRun(false);
                break;
            case 11:
                DoScriptText(SAY_DS_PROLOGUE, me);
                break;
            case 13:
                SetEquipmentSlots(true);
                me->SetSheath(SHEATH_STATE_UNARMED);
                me->HandleEmoteCommand(EMOTE_STATE_USESTANDING_NO_SHEATHE);
                break;
            case 17:
                pPlayer->GroupEventHappens(QUEST_TOME_VALOR, me);
                break;
        }
    }

    void AttackStart(Unit* pWho)
    override {
        if (!pWho)
            return;

        if (me->Attack(pWho, false))
        {
            me->AddThreat(pWho, 0.0f);
            me->SetInCombatWith(pWho);
            pWho->SetInCombatWith(me);

            me->GetMotionMaster()->MoveChase(pWho, 30.0f);
        }
    }

    void JustSummoned(Creature* pSummoned)
    override {
        pSummoned->AI()->AttackStart(me);
    }

    void Update(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        if (uiShootTimer <= diff)
        {
            uiShootTimer = 1500;

            //if (!me->IsWithinDist(me->GetVictim(), ATTACK_DISTANCE))
            if (me->GetDistance2d(me->GetVictim()) > ATTACK_DISTANCE)
                DoCast(me->GetVictim(), SPELL_SHOOT);
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
            pEscortAI->Start(true, true, true, pPlayer->GetGUID(), pCreature->GetEntry());
    }

    return true;
}

CreatureAI* GetAI_npc_daphne_stilwell(Creature* pCreature)
{
    return new npc_daphne_stilwellAI(pCreature);
}

/*######
## AddSC
######*/

void AddSC_westfall()
{
    OLDScript *newscript;

    newscript = new OLDScript;
    newscript->Name="npc_defias_traitor";
    newscript->GetAI = &GetAI_npc_defias_traitor;
    newscript->OnQuestAccept = &QuestAccept_npc_defias_traitor;
    sScriptMgr->RegisterOLDScript(newscript);
    
    newscript = new OLDScript;
    newscript->Name="npc_daphne_stilwell";
    newscript->GetAI = &GetAI_npc_daphne_stilwell;
    newscript->OnQuestAccept = &QuestAccept_npc_daphne_stilwell;
    sScriptMgr->RegisterOLDScript(newscript);
}

