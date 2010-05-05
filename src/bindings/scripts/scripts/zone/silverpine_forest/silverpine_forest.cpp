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

#include "precompiled.h"
#include "../../npc/npc_escortAI.h"

/*######
## npc_astor_hadren
######*/

#define GOSSIP_HAH "You're Astor Hadren, right?"
#define GOSSIP_SAH "You've got something I need, Astor. And I'll be taking it now."
struct TRINITY_DLL_DECL npc_astor_hadrenAI : public ScriptedAI
{
    npc_astor_hadrenAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        m_creature->setFaction(68);
    }

    void Aggro(Unit* who)
    {
    }

    void JustDied(Unit *who)
    {
        m_creature->setFaction(68);
    }
};

CreatureAI* GetAI_npc_astor_hadren(Creature *_creature)
{
    return new npc_astor_hadrenAI(_creature);
}

bool GossipHello_npc_astor_hadren(Player *player, Creature *_Creature)
{
    if (player->GetQuestStatus(1886) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_HAH, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(623, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_astor_hadren(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SAH, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(624, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->CLOSE_GOSSIP_MENU();
            _Creature->setFaction(21);
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

struct TRINITY_DLL_DECL npc_deathstalker_erlandAI : public npc_escortAI
{
    npc_deathstalker_erlandAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch(i)
        {
        case 1: DoScriptText(SAY_START, m_creature, player);break;
        case 13:
            DoScriptText(SAY_LAST, m_creature, player);
            player->GroupEventHappens(QUEST_ESCORTING, m_creature);break;
        case 14: DoScriptText(SAY_THANKS, m_creature, player);break;
        case 15: {
                Unit* Rane = FindCreature(NPC_RANE, 20, m_creature);
                if(Rane)
                    DoScriptText(SAY_RANE, Rane);
                break;}
        case 16: DoScriptText(SAY_ANSWER, m_creature);break;
        case 17: DoScriptText(SAY_MOVE_QUINN, m_creature); break;
        case 24: DoScriptText(SAY_GREETINGS, m_creature);break;
        case 25: {
                Unit* Quinn = FindCreature(NPC_QUINN, 20, m_creature);
                if(Quinn)
                    DoScriptText(SAY_QUINN, Quinn);
                break;}
        case 26: DoScriptText(SAY_ON_BYE, m_creature, NULL);break;

        }
    }

    void Reset() {}

    void Aggro(Unit* who)
    {
        switch(rand()%2)
        {
        case 0: DoScriptText(SAY_AGGRO_1, m_creature, who);break;
        case 1: DoScriptText(SAY_AGGRO_2, m_creature, who);break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
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
SDCategory: Script Quests

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

struct TRINITY_DLL_DECL pyrewood_ambushAI : public ScriptedAI
{

    pyrewood_ambushAI(Creature *c) : ScriptedAI(c), Summons(m_creature)
    {
       QuestInProgress = false;
    }


    uint32 Phase;
    int KillCount;
    uint32 WaitTimer;
    uint64 PlayerGUID;
    SummonList Summons;

    bool QuestInProgress;

    void Reset()
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

    void Aggro(Unit *who){}

    void JustSummoned(Creature *pSummoned)
    {
        Summons.Summon(pSummoned);
        ++KillCount;
    }

    void SummonedCreatureDespawn(Creature* pSummoned)
    {
        Summons.Despawn(pSummoned);
        --KillCount;
    }

    void SummonCreatureWithRandomTarget(uint32 creatureId, int position)
    {
        Creature *pSummoned = m_creature->SummonCreature(creatureId, SpawnPoints[position][0], SpawnPoints[position][1], SpawnPoints[position][2], SpawnPoints[position][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
        if(pSummoned)
        {
            Player* pPlayer = NULL;
            Unit* pTarget = NULL;
            if (PlayerGUID)
            {
                pPlayer = Unit::GetPlayer(PlayerGUID);
                switch(rand()%2)
                {
                    case 0: pTarget = m_creature; break;
                    case 1: pTarget = pPlayer; break;
                }
            }else
                pTarget = m_creature;

            pSummoned->setFaction(168);
            pSummoned->AddThreat(pTarget, 32.0f);
            ((Creature*)pSummoned)->AI()->AttackStart(pTarget);
        }
    }

    void JustDied(Unit* pKiller)
    {
        if (PlayerGUID)
        {
            Player* pPlayer = Unit::GetPlayer(PlayerGUID);
            if (pPlayer && ((Player*)pPlayer)->GetQuestStatus(QUEST_PYREWOOD_AMBUSH) == QUEST_STATUS_INCOMPLETE)
                ((Player*)pPlayer)->FailQuest(QUEST_PYREWOOD_AMBUSH);
        }

    }

    void UpdateAI(const uint32 diff)
    {

        //sLog.outString("DEBUG: p(%i) k(%i) d(%u) W(%i)", Phase, KillCount, diff, WaitTimer);

        if(!QuestInProgress)
            return;
            
        if (PlayerGUID) {
            Player* pPlayer = Unit::GetPlayer(PlayerGUID);
            if (!pPlayer)
                return;
            
            if (pPlayer->isDead()) {
                pPlayer->FailQuest(QUEST_PYREWOOD_AMBUSH);
                Summons.DespawnAll();
                QuestInProgress = false;
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
                    m_creature->Say(NPCSAY_INIT, LANG_UNIVERSAL, 0); //no blizzlike

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
                    Unit* player = Unit::GetUnit((*m_creature), PlayerGUID);
                    if( player && player->GetTypeId() == TYPEID_PLAYER )
                    {
                        m_creature->Say(NPCSAY_END, LANG_UNIVERSAL, 0); //no blizzlike
                        ((Player*)player)->GroupEventHappens(QUEST_PYREWOOD_AMBUSH, m_creature);
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
    }

    return true;
}

/*######
## AddSC
######*/

void AddSC_silverpine_forest()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_astor_hadren";
    newscript->pGossipHello =  &GossipHello_npc_astor_hadren;
    newscript->pGossipSelect = &GossipSelect_npc_astor_hadren;
    newscript->GetAI = &GetAI_npc_astor_hadren;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_deathstalker_erland";
    newscript->GetAI = &GetAI_npc_deathstalker_erlandAI;
    newscript->pQuestAccept = &QuestAccept_npc_deathstalker_erland;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "pyrewood_ambush";
    newscript->GetAI = &GetAI_pyrewood_ambush;
    newscript->pQuestAccept = &QuestAccept_pyrewood_ambush;
    newscript->RegisterSelf();
}

