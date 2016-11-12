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
SDName: Silverpine_Forest
SD%Complete: 100
SDComment: Quest support: 1886, 435
SDCategory: Silverpine Forest
EndScriptData */

/* ContentData
npc_astor_hadren
npc_deathstalker_erland
EndContentData */


#include "EscortAI.h"

/*######
## npc_astor_hadren
######*/

#define GOSSIP_HAH "You're Astor Hadren, right?"
#define GOSSIP_SAH "You've got something I need, Astor. And I'll be taking it now."
struct npc_astor_hadrenAI : public ScriptedAI
{
    npc_astor_hadrenAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    override {
        me->SetFaction(68);
    }

    void EnterCombat(Unit* who)
    override {
    }

    void JustDied(Unit *who)
    override {
        me->SetFaction(68);
    }
};

CreatureAI* GetAI_npc_astor_hadren(Creature *_creature)
{
    return new npc_astor_hadrenAI(_creature);
}

bool GossipHello_npc_astor_hadren(Player *player, Creature *_Creature)
{
    if (player->GetQuestStatus(1886) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, GOSSIP_HAH, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU_TEXTID(623, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_astor_hadren(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, GOSSIP_SAH, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU_TEXTID(624, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->CLOSE_GOSSIP_MENU();
            _Creature->SetFaction(21);
            if(player)
                ((npc_astor_hadrenAI*)_Creature->AI())->AttackStart(player);
            break;
    }
    return true;
}

/*######
## npc_deathstalker_erland
######*/

#define SAY_QUESTACCEPT -1000335
#define SAY_START       -1000336
#define SAY_AGGRO_1     -1000337
#define SAY_AGGRO_2     -1000338
#define SAY_LAST        -1000339

#define SAY_THANKS      -1000340
#define SAY_RANE        -1000341
#define SAY_ANSWER      -1000342
#define SAY_MOVE_QUINN  -1000343

#define SAY_GREETINGS   -1000344
#define SAY_QUINN       -1000345
#define SAY_ON_BYE      -1000346

#define QUEST_ESCORTING 435
#define NPC_RANE        1950
#define NPC_QUINN       1951

struct npc_deathstalker_erlandAI : public npc_escortAI
{
    npc_deathstalker_erlandAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    override {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch(i)
        {
        case 1: DoScriptText(SAY_START, me, player);break;
        case 13:
            DoScriptText(SAY_LAST, me, player);
            player->GroupEventHappens(QUEST_ESCORTING, me);break;
        case 14: DoScriptText(SAY_THANKS, me, player);break;
        case 15: {
                Unit* Rane = FindCreature(NPC_RANE, 20, me);
                if(Rane)
                    DoScriptText(SAY_RANE, Rane);
                break;}
        case 16: DoScriptText(SAY_ANSWER, me);break;
        case 17: DoScriptText(SAY_MOVE_QUINN, me); break;
        case 24: DoScriptText(SAY_GREETINGS, me);break;
        case 25: {
                Unit* Quinn = FindCreature(NPC_QUINN, 20, me);
                if(Quinn)
                    DoScriptText(SAY_QUINN, Quinn);
                break;}
        case 26: DoScriptText(SAY_ON_BYE, me, nullptr);break;

        }
    }

    void Reset() override {}

    void EnterCombat(Unit* who)
    override {
        switch(rand()%2)
        {
        case 0: DoScriptText(SAY_AGGRO_1, me, who);break;
        case 1: DoScriptText(SAY_AGGRO_2, me, who);break;
        }
    }

    void UpdateAI(const uint32 diff)
    override {
        npc_escortAI::UpdateAI(diff);
    }
};

bool QuestAccept_npc_deathstalker_erland(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_ESCORTING)
    {
        DoScriptText(SAY_QUESTACCEPT, creature, player);
        ((npc_escortAI*)(creature->AI()))->Start(true, true, false, player->GetGUID(), creature->GetEntry());
    }

    return true;
}

CreatureAI* GetAI_npc_deathstalker_erlandAI(Creature *pCreature)
{
    return new npc_deathstalker_erlandAI(pCreature);
}

/* ScriptData
SDName: pyrewood_ambush
SD%Complete: 100
SDComment: Quest pyrewood ambush (id 452), you have to kill 4 waves of people: 1, 2, 3, 3.
           The quest giver is here for help you.
SDCategory: OLDScript Quests

EndScriptData */

#define QUEST_PYREWOOD_AMBUSH 452

#define NPCSAY_INIT "Get ready, they'll be arriving any minute..." //no blizzlike
#define NPCSAY_END "Thanks for your help!" //no blizzlike

static float SpawnPoints[3][4] =
{
    //pos_x   pos_y     pos_z    orien
    //door
    {-396.17, 1505.86, 19.77, 0},
    {-396.91, 1505.77, 19.77, 0},
    {-397.94, 1504.74, 19.77, 0},
};

#define WAIT_SECS 6000

struct pyrewood_ambushAI : public ScriptedAI
{

    pyrewood_ambushAI(Creature *c) : ScriptedAI(c), Summons(me)
    {
       QuestInProgress = false;
    }


    uint32 Phase;
    int KillCount;
    uint32 WaitTimer;
    uint64 PlayerGUID;
    SummonList Summons;

    bool QuestInProgress;

    void Reset() override
    {
        WaitTimer = WAIT_SECS;

        if(!QuestInProgress) //fix reset values (see UpdateVictim)
        {
            Phase = 0;
            KillCount = 0;
            PlayerGUID = 0;
            Summons.DespawnAll();
        }
    }

    void EnterCombat(Unit *who) override  
    {}

    void JustSummoned(Creature *pSummoned) override
    {
        Summons.Summon(pSummoned);
        ++KillCount;
    }

    void SummonedCreatureDespawn(Creature* pSummoned)
    override {
        Summons.Despawn(pSummoned);
        --KillCount;
    }
    
    void JustRespawned() override
    {
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void SummonCreatureWithRandomTarget(uint32 creatureId, int position)
    {
        Creature *pSummoned = me->SummonCreature(creatureId, SpawnPoints[position][0], SpawnPoints[position][1], SpawnPoints[position][2], SpawnPoints[position][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
        if(pSummoned)
        {
            Player* pPlayer = nullptr;
            Unit* pTarget = nullptr;
            if (PlayerGUID)
            {
                pPlayer = ObjectAccessor::GetPlayer(*me, PlayerGUID);
                switch(rand()%2)
                {
                    case 0: pTarget = me; break;
                    case 1: pTarget = pPlayer; break;
                }
            }else
                pTarget = me;

            pSummoned->SetFaction(168);
            pSummoned->AddThreat(pTarget, 32.0f);
            (pSummoned->ToCreature())->AI()->AttackStart(pTarget);
        }
    }

    void JustDied(Unit* pKiller) override
    {
        if (PlayerGUID)
        {
            Player* pPlayer = ObjectAccessor::GetPlayer(*me, PlayerGUID);
            if (pPlayer && (pPlayer->ToPlayer())->GetQuestStatus(QUEST_PYREWOOD_AMBUSH) == QUEST_STATUS_INCOMPLETE)
                (pPlayer->ToPlayer())->FailQuest(QUEST_PYREWOOD_AMBUSH);
        }

    }

    void UpdateAI(const uint32 diff) override
    {

        //TC_LOG_INFO("DEBUG: p(%i) k(%i) d(%u) W(%i)", Phase, KillCount, diff, WaitTimer);

        if(!QuestInProgress)
            return;
            
        if (PlayerGUID) {
            Player* pPlayer = ObjectAccessor::GetPlayer(*me, PlayerGUID);
            if (!pPlayer)
                return;
            
            if (pPlayer->IsDead()) {
                pPlayer->FailQuest(QUEST_PYREWOOD_AMBUSH);
                Summons.DespawnAll();
                QuestInProgress = false;
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            }
        }

        if(KillCount && (Phase < 6))
        {
            if(!UpdateVictim() ) //reset() on target Despawn...
                return;

            DoMeleeAttackIfReady();
            return;
        }


        switch(Phase){
            case 0:
                if(WaitTimer == WAIT_SECS)
                    me->Say(NPCSAY_INIT, LANG_UNIVERSAL, nullptr); //no blizzlike

                if(WaitTimer <= diff)
                {
                    WaitTimer -= diff;
                    return;
                }
                break;
            case 1:
                SummonCreatureWithRandomTarget(2060, 1);
                break;
            case 2:
                SummonCreatureWithRandomTarget(2061, 2);
                SummonCreatureWithRandomTarget(2062, 0);
                break;
            case 3:
                SummonCreatureWithRandomTarget(2063, 1);
                SummonCreatureWithRandomTarget(2064, 2);
                SummonCreatureWithRandomTarget(2065, 0);
                break;
            case 4:
                SummonCreatureWithRandomTarget(2066, 1);
                SummonCreatureWithRandomTarget(2067, 0);
                SummonCreatureWithRandomTarget(2068, 2);
                break;
            case 5: //end
                if (PlayerGUID)
                {
                    Unit* player = ObjectAccessor::GetUnit((*me), PlayerGUID);
                    if( player && player->GetTypeId() == TYPEID_PLAYER )
                    {
                        me->Say(NPCSAY_END, LANG_UNIVERSAL, nullptr); //no blizzlike
                        (player->ToPlayer())->GroupEventHappens(QUEST_PYREWOOD_AMBUSH, me);
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    }
                }
                QuestInProgress = false;
                Reset();
                break;
         }

         Phase++; //prepare next phase

    }
};

CreatureAI* GetAI_pyrewood_ambush(Creature *pCreature)
{
    return new pyrewood_ambushAI (pCreature);
}

bool QuestAccept_pyrewood_ambush(Player *pPlayer, Creature *pCreature, const Quest *pQuest )
{
    if ((pQuest->GetQuestId() == QUEST_PYREWOOD_AMBUSH) && (!((pyrewood_ambushAI*)(pCreature->AI()))->QuestInProgress))
    {
        ((pyrewood_ambushAI*)(pCreature->AI()))->QuestInProgress = true;
        ((pyrewood_ambushAI*)(pCreature->AI()))->Phase = 0;
        ((pyrewood_ambushAI*)(pCreature->AI()))->KillCount = 0;
        ((pyrewood_ambushAI*)(pCreature->AI()))->PlayerGUID = pPlayer->GetGUID();
        pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    return true;
}

/*######
## AddSC
######*/

void AddSC_silverpine_forest()
{
    OLDScript *newscript;

    newscript = new OLDScript;
    newscript->Name="npc_astor_hadren";
    newscript->OnGossipHello =  &GossipHello_npc_astor_hadren;
    newscript->OnGossipSelect = &GossipSelect_npc_astor_hadren;
    newscript->GetAI = &GetAI_npc_astor_hadren;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="npc_deathstalker_erland";
    newscript->GetAI = &GetAI_npc_deathstalker_erlandAI;
    newscript->OnQuestAccept = &QuestAccept_npc_deathstalker_erland;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name = "pyrewood_ambush";
    newscript->GetAI = &GetAI_pyrewood_ambush;
    newscript->OnQuestAccept = &QuestAccept_pyrewood_ambush;
    sScriptMgr->RegisterOLDScript(newscript);
}

