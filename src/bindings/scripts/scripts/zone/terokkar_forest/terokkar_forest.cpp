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
SDName: Terokkar_Forest
SD%Complete: 80
SDComment: Quest support: 9889, 10009, 10873, 10896, 11096, 10052, 10051, 11093. Skettis->Ogri'la Flight
SDCategory: Terokkar Forest
EndScriptData */

/* ContentData
mob_unkor_the_ruthless
mob_infested_root_walker
mob_rotting_forest_rager
mob_netherweb_victim
npc_floon
npc_skyguard_handler_deesak
npc_isla_starmane
npc_hungry_nether_ray
npc_kaliri_trigger
EndContentData */

#include "precompiled.h"
#include "../../npc/npc_escortAI.h"

/*######
## mob_unkor_the_ruthless
######*/

#define SAY_SUBMIT                      -1000351

#define FACTION_HOSTILE                 45
#define FACTION_FRIENDLY                35
#define QUEST_DONTKILLTHEFATONE         9889

#define SPELL_PULVERIZE                 2676
//#define SPELL_QUID9889                32174

struct mob_unkor_the_ruthlessAI : public ScriptedAI
{
    mob_unkor_the_ruthlessAI(Creature* c) : ScriptedAI(c) {}

    bool CanDoQuest;
    uint32 UnkorUnfriendly_Timer;
    uint32 Pulverize_Timer;

    void Reset()
    {
        CanDoQuest = false;
        UnkorUnfriendly_Timer = 0;
        Pulverize_Timer = 3000;
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, PLAYER_STATE_NONE);
        m_creature->setFaction(FACTION_HOSTILE);
    }

    void Aggro(Unit *who) {}

    void DoNice()
    {
        DoScriptText(SAY_SUBMIT, m_creature);
        m_creature->setFaction(FACTION_FRIENDLY);
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, PLAYER_STATE_SIT);
        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
        UnkorUnfriendly_Timer = 60000;
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if( done_by->GetTypeId() == TYPEID_PLAYER )
            if( (m_creature->GetHealth()-damage)*100 / m_creature->GetMaxHealth() < 30 )
        {
            if( Group* pGroup = (done_by->ToPlayer())->GetGroup() )
            {
                for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    Player *pGroupie = itr->getSource();
                    if( pGroupie &&
                        pGroupie->GetQuestStatus(QUEST_DONTKILLTHEFATONE) == QUEST_STATUS_INCOMPLETE &&
                        pGroupie->GetReqKillOrCastCurrentCount(QUEST_DONTKILLTHEFATONE, 18260) == 10 )
                    {
                        pGroupie->AreaExploredOrEventHappens(QUEST_DONTKILLTHEFATONE);
                        if( !CanDoQuest )
                            CanDoQuest = true;
                    }
                }
            } else
            if( (done_by->ToPlayer())->GetQuestStatus(QUEST_DONTKILLTHEFATONE) == QUEST_STATUS_INCOMPLETE &&
                (done_by->ToPlayer())->GetReqKillOrCastCurrentCount(QUEST_DONTKILLTHEFATONE, 18260) == 10 )
            {
                (done_by->ToPlayer())->AreaExploredOrEventHappens(QUEST_DONTKILLTHEFATONE);
                CanDoQuest = true;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if( CanDoQuest )
        {
            if( !UnkorUnfriendly_Timer )
            {
                //DoCast(m_creature,SPELL_QUID9889);        //not using spell for now
                DoNice();
            }
            else
            {
                if( UnkorUnfriendly_Timer < diff )
                {
                    EnterEvadeMode();
                    return;
                }else UnkorUnfriendly_Timer -= diff;
            }
        }

        if(!UpdateVictim())
            return;

        if( Pulverize_Timer < diff )
        {
            DoCast(m_creature,SPELL_PULVERIZE);
            Pulverize_Timer = 9000;
        }else Pulverize_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_unkor_the_ruthless(Creature *_Creature)
{
    return new mob_unkor_the_ruthlessAI (_Creature);
}

/*######
## mob_infested_root_walker
######*/

struct mob_infested_root_walkerAI : public ScriptedAI
{
    mob_infested_root_walkerAI(Creature *c) : ScriptedAI(c) {}

    void Reset() { }
    void Aggro(Unit *who) { }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by && done_by->GetTypeId() == TYPEID_PLAYER)
            if (m_creature->GetHealth() <= damage)
                if (rand()%100 < 75)
                    //Summon Wood Mites
                    m_creature->CastSpell(m_creature,39130,true);
    }
};
CreatureAI* GetAI_mob_infested_root_walker(Creature *_Creature)
{
    return new mob_infested_root_walkerAI (_Creature);
}

/*######
## mob_rotting_forest_rager
######*/

struct mob_rotting_forest_ragerAI : public ScriptedAI
{
    mob_rotting_forest_ragerAI(Creature *c) : ScriptedAI(c) {}

    void Reset() { }
    void Aggro(Unit *who) { }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by->GetTypeId() == TYPEID_PLAYER)
            if (m_creature->GetHealth() <= damage)
                if (rand()%100 < 75)
                    //Summon Lots of Wood Mights
                    m_creature->CastSpell(m_creature,39134,true);
    }
};
CreatureAI* GetAI_mob_rotting_forest_rager(Creature *_Creature)
{
    return new mob_rotting_forest_ragerAI (_Creature);
}

/*######
## mob_netherweb_victim
######*/

#define QUEST_TARGET        22459
//#define SPELL_FREE_WEBBED   38950

const uint32 netherwebVictims[6] =
{
    18470, 16805, 21242, 18452, 22482, 21285
};
struct mob_netherweb_victimAI : public ScriptedAI
{
    mob_netherweb_victimAI(Creature *c) : ScriptedAI(c) {}

    void Reset() { }
    void Aggro(Unit *who) { }
    void MoveInLineOfSight(Unit *who) { }

    void JustDied(Unit* Killer)
    {
        if( Killer->GetTypeId() == TYPEID_PLAYER )
        {
            if( (Killer->ToPlayer())->GetQuestStatus(10873) == QUEST_STATUS_INCOMPLETE )
            {
                if( rand()%100 < 25 )
                {
                    DoSpawnCreature(QUEST_TARGET,0,0,0,0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,60000);
                    (Killer->ToPlayer())->KilledMonster(QUEST_TARGET, m_creature->GetGUID());
                }else
                DoSpawnCreature(netherwebVictims[rand()%6],0,0,0,0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,60000);

                if( rand()%100 < 75 )
                    DoSpawnCreature(netherwebVictims[rand()%6],0,0,0,0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,60000);
                DoSpawnCreature(netherwebVictims[rand()%6],0,0,0,0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,60000);
            }
        }
    }
};
CreatureAI* GetAI_mob_netherweb_victim(Creature *_Creature)
{
    return new mob_netherweb_victimAI (_Creature);
}

/*######
## npc_floon
######*/

#define GOSSIP_FLOON1           "You owe Sim'salabim money. Hand them over or die!"
#define GOSSIP_FLOON2           "Hand over the money or die...again!"

#define SAY_FLOON_ATTACK        -1000352

#define FACTION_HOSTILE_FL      1738
#define FACTION_FRIENDLY_FL     35

#define SPELL_SILENCE           6726
#define SPELL_FROSTBOLT         9672
#define SPELL_FROST_NOVA        11831

struct npc_floonAI : public ScriptedAI
{
    npc_floonAI(Creature* c) : ScriptedAI(c) {}

    uint32 Silence_Timer;
    uint32 Frostbolt_Timer;
    uint32 FrostNova_Timer;

    void Reset()
    {
        Silence_Timer = 2000;
        Frostbolt_Timer = 4000;
        FrostNova_Timer = 9000;
        m_creature->setFaction(FACTION_FRIENDLY_FL);
    }

    void Aggro(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if( Silence_Timer < diff )
        {
            DoCast(m_creature->getVictim(),SPELL_SILENCE);
            Silence_Timer = 30000;
        }else Silence_Timer -= diff;

        if( FrostNova_Timer < diff )
        {
            DoCast(m_creature,SPELL_FROST_NOVA);
            FrostNova_Timer = 20000;
        }else FrostNova_Timer -= diff;

        if( Frostbolt_Timer < diff )
        {
            DoCast(m_creature->getVictim(),SPELL_FROSTBOLT);
            Frostbolt_Timer = 5000;
        }else Frostbolt_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_floon(Creature *_Creature)
{
    return new npc_floonAI (_Creature);
}

bool GossipHello_npc_floon(Player *player, Creature *_Creature )
{
    if( player->GetQuestStatus(10009) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM(1, GOSSIP_FLOON1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(9442, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_floon(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF )
    {
        player->ADD_GOSSIP_ITEM(1, GOSSIP_FLOON2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(9443, _Creature->GetGUID());
    }
    if( action == GOSSIP_ACTION_INFO_DEF+1 )
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->setFaction(FACTION_HOSTILE_FL);
        DoScriptText(SAY_FLOON_ATTACK, _Creature, player);
        ((npc_floonAI*)_Creature->AI())->AttackStart(player);
    }
    return true;
}

/*######
## npc_skyguard_handler_deesak
######*/

#define GOSSIP_SKYGUARD "Fly me to Ogri'la please"

bool GossipHello_npc_skyguard_handler_deesak(Player *player, Creature *_Creature )
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetReputationRank(1031) >= REP_HONORED)
        player->ADD_GOSSIP_ITEM( 2, GOSSIP_SKYGUARD, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_skyguard_handler_deesak(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,41279,true);               //TaxiPath 705 (Taxi - Skettis to Skyguard Outpost)
    }
    return true;
}

/*######
## npc_isla_starmane
######*/

#define SAY_PROGRESS_1  -1000353
#define SAY_PROGRESS_2  -1000354
#define SAY_PROGRESS_3  -1000355
#define SAY_PROGRESS_4  -1000356

#define QUEST_EFTW_H    10052
#define QUEST_EFTW_A    10051
#define GO_CAGE         182794
#define SPELL_CAT       32447

struct npc_isla_starmaneAI : public npc_escortAI
{
    npc_isla_starmaneAI(Creature* c) : npc_escortAI(c) {}

    bool Completed;

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if(!player)
            return;

        switch(i)
        {
        case 0:
            {
            GameObject* Cage = FindGameObject(GO_CAGE, 10, m_creature);
            if(Cage)
                Cage->SetGoState(0);
            }break;
        case 2: DoScriptText(SAY_PROGRESS_1, m_creature, player); break;
        case 5: DoScriptText(SAY_PROGRESS_2, m_creature, player); break;
        case 6: DoScriptText(SAY_PROGRESS_3, m_creature, player); break;
        case 29:DoScriptText(SAY_PROGRESS_4, m_creature, player);
            if (player)
            {
                if( player->GetTeam() == ALLIANCE)
                    player->GroupEventHappens(QUEST_EFTW_A, m_creature);
                else if(player->GetTeam() == HORDE)
                    player->GroupEventHappens(QUEST_EFTW_H, m_creature);
            } Completed = true;
            m_creature->SetInFront(player); break;
        case 30: m_creature->HandleEmoteCommand(EMOTE_ONESHOT_WAVE); break;
        case 31: DoCast(m_creature, SPELL_CAT);
            m_creature->RemoveUnitMovementFlag(MOVEMENTFLAG_WALK_MODE); break;
        }
    }

    void Reset()
    {
        Completed = false;
        m_creature->setFaction(1660);
    }

    void Aggro(Unit* who){}

    void JustDied(Unit* killer)
    {
        if (PlayerGUID)
        {
            Player* player = GetPlayerForEscort();
            if (player && !Completed)
            {
                if(player->GetTeam() == ALLIANCE)
                    player->FailQuest(QUEST_EFTW_A);
                else if(player->GetTeam() == HORDE)
                    player->FailQuest(QUEST_EFTW_H);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
    }
};

bool QuestAccept_npc_isla_starmane(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_EFTW_H || quest->GetQuestId() == QUEST_EFTW_A)
    {
        ((npc_escortAI*)(creature->AI()))->Start(true, true, false, player->GetGUID(), creature->GetEntry());
        creature->setFaction(113);
    }
    return true;
}

CreatureAI* GetAI_npc_isla_starmaneAI(Creature *pCreature)
{
    return new npc_isla_starmaneAI(pCreature);
}

/*######
## go_skull_pile
######*/
#define GOSSIP_S_DARKSCREECHER_AKKARAI      "Summon Darkscreecher Akkarai"
#define GOSSIP_S_KARROG                     "Summon Karrog"
#define GOSSIP_S_GEZZARAK_THE_HUNTRESS      "Summon Gezzarak the Huntress"
#define GOSSIP_S_VAKKIZ_THE_WINDRAGER       "Summon Vakkiz the Windrager"

bool GossipHello_go_skull_pile(Player *player, GameObject* _GO)
{
    if ((player->GetQuestStatus(11885) == QUEST_STATUS_INCOMPLETE) || player->GetQuestRewardStatus(11885))
    {
        player->ADD_GOSSIP_ITEM(0, GOSSIP_S_DARKSCREECHER_AKKARAI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(0, GOSSIP_S_KARROG, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->ADD_GOSSIP_ITEM(0, GOSSIP_S_GEZZARAK_THE_HUNTRESS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->ADD_GOSSIP_ITEM(0, GOSSIP_S_VAKKIZ_THE_WINDRAGER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
    }

    player->SEND_GOSSIP_MENU(_GO->GetGOInfo()->questgiver.gossipID, _GO->GetGUID());
    return true;
}

void SendActionMenu_go_skull_pile(Player *player, GameObject* _GO, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
              player->CastSpell(player,40642,false);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
              player->CastSpell(player,40640,false);
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
              player->CastSpell(player,40632,false);
            break;
        case GOSSIP_ACTION_INFO_DEF + 4:
              player->CastSpell(player,40644,false);
            break;
    }
}

bool GossipSelect_go_skull_pile(Player *player, GameObject* _GO, uint32 sender, uint32 action )
{
    switch(sender)
    {
        case GOSSIP_SENDER_MAIN:    SendActionMenu_go_skull_pile(player, _GO, action); break;
    }
    return true;
}

/*######
## go_ancient_skull_pile
######*/

enum eAncientSkullPile
{
    QUEST_TEROKK_DOWNFALL       = 11073,

    ENTRY_TEROKK                = 21838
};

bool GossipHello_go_ancient_skull_pile(Player* pPlayer, GameObject* pGo)
{
    if (pPlayer->GetQuestStatus(QUEST_TEROKK_DOWNFALL) == QUEST_STATUS_INCOMPLETE)
        pPlayer->SummonCreature(ENTRY_TEROKK, -3793.01, 3503.55, 287.01, 0.9485, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
        
    return false;
}

/*######
## npc_hungry_nether_ray
######*/

#define SPELL_SUMMON_RAY    41423

#define RAY_FEED_CREDIT  23438

struct npc_hungry_nether_rayAI : public ScriptedAI
{
    npc_hungry_nether_rayAI(Creature* c) : ScriptedAI(c) {}
    
    uint32 checkTimer;
    uint64 lastCredit;
    
    void Reset()
    {
        m_creature->GetMotionMaster()->MoveFollow(m_creature->GetOwner(), PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        checkTimer = 5000; //check for creature every 5 sec
    }
    
    void Aggro(Unit* who) {}
    
    Creature* SelectCreatureInGrid(uint32 entry, float range)
    {
        Creature* pCreature = NULL;

        CellPair pair(Trinity::ComputeCellPair(m_creature->GetPositionX(), m_creature->GetPositionY()));
        Cell cell(pair);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*m_creature, entry, true, range); //true, as it should check only for alive creatures
        Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(pCreature, creature_check);

        TypeContainerVisitor<Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck>, GridTypeMapContainer> creature_searcher(searcher);

        CellLock<GridReadGuard> cell_lock(cell, pair);
        cell_lock->Visit(cell_lock, creature_searcher,*(m_creature->GetMap()));
        
        return pCreature;
    }
    
    void UpdateAI(const uint32 diff)
    {
        if (checkTimer < diff)
        {
            Creature* feedCredit = SelectCreatureInGrid(RAY_FEED_CREDIT, 10);
            if (feedCredit && feedCredit->GetGUID() != lastCredit)
            {
                lastCredit = feedCredit->GetGUID();
                (m_creature->GetOwner()->ToPlayer())->KilledMonster(RAY_FEED_CREDIT, feedCredit->GetGUID());
                feedCredit->DealDamage(feedCredit, feedCredit->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                feedCredit->RemoveCorpse();
                checkTimer = 5000;
            }
        }else checkTimer -= diff;
    }
};

CreatureAI* GetAI_npc_hungry_nether_ray(Creature *pCreature)
{
    return new npc_hungry_nether_rayAI(pCreature);
}

/*######
## npc_kaliri_egg_trigger
######*/

struct npc_kaliri_egg_triggerAI : public ScriptedAI
{
    npc_kaliri_egg_triggerAI(Creature* c) : ScriptedAI(c) {}

    void Reset()
    {
        m_creature->setFaction(14);
        m_creature->SetVisibility(VISIBILITY_OFF);
        GameObject* eggGO = NULL;
        CellPair pair(Trinity::ComputeCellPair(m_creature->GetPositionX(), m_creature->GetPositionY()));
        Cell cell(pair);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        Trinity::NearestGameObjectEntryInObjectRangeCheck go_check(*m_creature, 185549, 1);
        Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher(eggGO, go_check);

        TypeContainerVisitor<Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck>, GridTypeMapContainer> go_searcher(searcher);

        CellLock<GridReadGuard> cell_lock(cell, pair);
        cell_lock->Visit(cell_lock, go_searcher,*(m_creature->GetMap()));

        if(eggGO)
            eggGO->SetGoState(1);
    }

    void Aggro(Unit *who) {}
    void AttackStart(Unit* who) {}
    void MoveInLineOfSight(Unit* who) {}
    void UpdateAI(const uint32 diff) {}
    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (spell->Id == 39844){
            GameObject* eggGO = NULL;
            CellPair pair(Trinity::ComputeCellPair(m_creature->GetPositionX(), m_creature->GetPositionY()));
            Cell cell(pair);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();

            Trinity::NearestGameObjectEntryInObjectRangeCheck go_check(*m_creature, 185549, 1);
            Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher(eggGO, go_check);

            TypeContainerVisitor<Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck>, GridTypeMapContainer> go_searcher(searcher);

            CellLock<GridReadGuard> cell_lock(cell, pair);
            cell_lock->Visit(cell_lock, go_searcher,*(m_creature->GetMap()));

            if(eggGO)
                eggGO->SetGoState(0);
                
            caster->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        }
    }
};

CreatureAI* GetAI_npc_kaliri_egg_trigger(Creature *_Creature)
{
    return new npc_kaliri_egg_triggerAI (_Creature);
}

/*######
## AddSC
######*/

void AddSC_terokkar_forest()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name="mob_unkor_the_ruthless";
    newscript->GetAI = &GetAI_mob_unkor_the_ruthless;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_infested_root_walker";
    newscript->GetAI = &GetAI_mob_infested_root_walker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_rotting_forest_rager";
    newscript->GetAI = &GetAI_mob_rotting_forest_rager;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_netherweb_victim";
    newscript->GetAI = &GetAI_mob_netherweb_victim;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_floon";
    newscript->pGossipHello =  &GossipHello_npc_floon;
    newscript->pGossipSelect = &GossipSelect_npc_floon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_skyguard_handler_deesak";
    newscript->pGossipHello =  &GossipHello_npc_skyguard_handler_deesak;
    newscript->pGossipSelect = &GossipSelect_npc_skyguard_handler_deesak;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name= "npc_isla_starmane";
    newscript->GetAI = &GetAI_npc_isla_starmaneAI;
    newscript->pQuestAccept = &QuestAccept_npc_isla_starmane;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_skull_pile";
    newscript->pGOHello  = &GossipHello_go_skull_pile;
    newscript->pGOSelect = &GossipSelect_go_skull_pile;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_hungry_nether_ray";
    newscript->GetAI = &GetAI_npc_hungry_nether_ray;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="go_ancient_skull_pile";
    newscript->pGOHello = &GossipHello_go_ancient_skull_pile;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_kaliri_egg_trigger";
    newscript->GetAI =  &GetAI_npc_kaliri_egg_trigger;
    newscript->RegisterSelf();
}

