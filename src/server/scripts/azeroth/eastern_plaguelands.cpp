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
SDName: Eastern_Plaguelands
SD%Complete: 100
SDComment: Quest support: 5211, 5742, 9446. Special vendor Augustus the Touched
SDCategory: Eastern Plaguelands
EndScriptData */

/* ContentData
mobs_ghoul_flayer
npc_augustus_the_touched
npc_darrowshire_spirit
npc_tirion_fordring
npc_anchorite_truuen
EndContentData */


#include "EscortAI.h"

//id8530 - cannibal ghoul
//id8531 - gibbering ghoul
//id8532 - diseased flayer

struct mobs_ghoul_flayerAI : public ScriptedAI
{
    mobs_ghoul_flayerAI(Creature *c) : ScriptedAI(c) {}

    void Reset() override { }

    void EnterCombat(Unit* pWho) override { }

    void JustDied(Unit* pKiller)
    override {
        if (pKiller->GetTypeId() == TYPEID_PLAYER)
            DoSpawnCreature(11064, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 60000);
    }
};

CreatureAI* GetAI_mobs_ghoul_flayer(Creature* pCreature)
{
    return new mobs_ghoul_flayerAI (pCreature);
}

/*######
## npc_augustus_the_touched
######*/

bool GossipHello_npc_augustus_the_touched(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->IsQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pCreature->IsVendor() && pPlayer->GetQuestRewardStatus(6164))
        pPlayer->ADD_GOSSIP_ITEM(1, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    SEND_PREPARED_GOSSIP_MENU(pPlayer, pCreature);
    
    return true;
}

bool GossipSelect_npc_augustus_the_touched(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
        pPlayer->SEND_VENDORLIST(pCreature->GetGUID());
    return true;
}

/*######
## npc_darrowshire_spirit
######*/

#define SPELL_SPIRIT_SPAWNIN    17321

struct npc_darrowshire_spiritAI : public ScriptedAI
{
    npc_darrowshire_spiritAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    override {
        DoCast(me,SPELL_SPIRIT_SPAWNIN);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void EnterCombat(Unit* pWho) override { }

};
CreatureAI* GetAI_npc_darrowshire_spirit(Creature* pCreature)
{
    return new npc_darrowshire_spiritAI(pCreature);
}

bool GossipHello_npc_darrowshire_spirit(Player* pPlayer, Creature* pCreature)
{
    pPlayer->SEND_GOSSIP_MENU_TEXTID(3873, pCreature->GetGUID());
    pPlayer->TalkedToCreature(pCreature->GetEntry(), pCreature->GetGUID());
    pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    
    return true;
}

/*######
## npc_tirion_fordring
######*/

bool GossipHello_npc_tirion_fordring(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->IsQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetQuestStatus(5742) == QUEST_STATUS_INCOMPLETE && pPlayer->GetStandState() == PLAYER_STATE_SIT )
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I am ready to hear your tale, Tirion.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    SEND_PREPARED_GOSSIP_MENU(pPlayer, pCreature);

    return true;
}

bool GossipSelect_npc_tirion_fordring(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Thank you, Tirion.  What of your identity?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU_TEXTID(4493, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "That is terrible.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU_TEXTID(4494, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I will, Tirion.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            pPlayer->SEND_GOSSIP_MENU_TEXTID(4495, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(5742);
            break;
    }
    return true;
}

/*######
##  npc_anchorite_truuen
######*/

enum eTruuen
{
    SAY_WP_0                    = -1000709,
    SAY_WP_1                    = -1000710,
    SAY_WP_2                    = -1000711,
    SAY_WP_3                    = -1000712,
    SAY_WP_4                    = -1000713,
    SAY_WP_5                    = -1000714,
    SAY_WP_6                    = -1000715,

    NPC_GHOST_UTHER             = 17233,
    NPC_THEL_DANIS              = 1854,
    NPC_GHOUL                   = 1791,

    QUEST_TOMB_LIGHTBRINGER     = 9446
};

struct npc_anchorite_truuenAI : public npc_escortAI
{    
    npc_anchorite_truuenAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    uint32 EventTimer;
    uint64 UterGUID;
    uint32 uiPhase;

    void Reset()
    override {        
        EventTimer = 5000;
        UterGUID = 0;
        uiPhase = 0;
    }
    
    void EnterCombat(Unit *pWho) override {}
    
    void WaypointReached(uint32 uiPointId)
    override {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (uiPointId)
        {
        case 8:
            DoScriptText(SAY_WP_0, me);
            for (int i = 0; i < 2; i++)
                me->SummonCreature(NPC_GHOUL, 1035.43,-1572.97,61.5412, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 90000);
            break;
        case 9:
            DoScriptText(SAY_WP_1, me);
            break;
        case 14:
            for (int i = 0; i < 4; i++)
                me->SummonCreature(NPC_GHOUL, 1159.77,-1762.64,60.5699, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
            break;
        case 15:
            DoScriptText(SAY_WP_2, me);
            SetRun(false);
            break;
        case 22:
            if (Creature* pTheldanis = me->FindNearestCreature(NPC_THEL_DANIS, 50))
                DoScriptText(SAY_WP_3, pTheldanis);
            break;
        case 23:
            if (Creature* pUter = me->SummonCreature(NPC_GHOST_UTHER, 971.86,-1825.42 ,81.99 , 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 10000))
            {
                pUter->AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
                DoScriptText(SAY_WP_4, pUter, me);
                UterGUID = pUter->GetGUID();
                uiPhase = 1;
                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                SetEscortPaused(true);
            }
            pPlayer->GroupEventHappens(QUEST_TOMB_LIGHTBRINGER, me);
            break;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    override {
        npc_escortAI::UpdateAI(uiDiff);

        if (!UpdateVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED) && uiPhase)
            {
                if (EventTimer <= uiDiff)
                {
                    EventTimer = 5000;

                    if (Creature* pUter = ObjectAccessor::GetCreature(*me, UterGUID))
                    {
                        switch(uiPhase)
                        {
                        case 1:
                            DoScriptText(SAY_WP_5, pUter, me);
                            me->SetStandState(UNIT_STAND_STATE_STAND);
                            ++uiPhase;
                            break;
                        case 2:
                            DoScriptText(SAY_WP_6, pUter, me);
                            ++uiPhase = 0;
                            break;
                        case 3:
                            me->ForcedDespawn();
                            pUter->ForcedDespawn();
                            break;
                        }
                    }
                }
                else
                    EventTimer -= uiDiff;
            }
            return;
        }

        DoMeleeAttackIfReady();
    }

    void JustSummoned(Creature* pSummoned)
    override {
        if (pSummoned->GetEntry() == NPC_GHOUL)
            pSummoned->AI()->AttackStart(me);
    }
};

CreatureAI* GetAI_npc_anchorite_truuen(Creature* pCreature)
{
    return new npc_anchorite_truuenAI(pCreature);
}

bool QuestAccept_npc_anchorite_truuen(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_TOMB_LIGHTBRINGER)
    {
        if (npc_anchorite_truuenAI* pEscortAI = CAST_AI(npc_anchorite_truuenAI, pCreature->AI()))
        {
            pEscortAI->Start(true, true, false, pPlayer->GetGUID(), pCreature->GetEntry());
            pEscortAI->SetDespawnAtEnd(false);
            return true;
        }
    }
    return false;
}

void AddSC_eastern_plaguelands()
{
    OLDScript* newscript;

    newscript = new OLDScript;
    newscript->Name="mobs_ghoul_flayer";
    newscript->GetAI = &GetAI_mobs_ghoul_flayer;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="npc_augustus_the_touched";
    newscript->OnGossipHello = &GossipHello_npc_augustus_the_touched;
    newscript->OnGossipSelect = &GossipSelect_npc_augustus_the_touched;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="npc_darrowshire_spirit";
    newscript->GetAI = &GetAI_npc_darrowshire_spirit;
    newscript->OnGossipHello = &GossipHello_npc_darrowshire_spirit;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="npc_tirion_fordring";
    newscript->OnGossipHello =  &GossipHello_npc_tirion_fordring;
    newscript->OnGossipSelect = &GossipSelect_npc_tirion_fordring;
    sScriptMgr->RegisterOLDScript(newscript);
    
    newscript = new OLDScript;
    newscript->Name = "npc_anchorite_truuen";
    newscript->GetAI = &GetAI_npc_anchorite_truuen;
    newscript->OnQuestAccept =  &QuestAccept_npc_anchorite_truuen;
    sScriptMgr->RegisterOLDScript(newscript);
}

