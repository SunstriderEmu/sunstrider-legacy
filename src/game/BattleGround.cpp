/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
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

#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundMgr.h"
#include "Creature.h"
#include "MapManager.h"
#include "Language.h"
#include "Chat.h"
#include "SpellAuras.h"
#include "ArenaTeam.h"
#include "World.h"
#include "Util.h"
#include "SpellMgr.h"

Battleground::Battleground()
{
    m_TypeID            = 0;
    m_InstanceID        = 0;
    m_Status            = 0;
    m_RemovalTime       = 0;
    m_StartTime         = 0;
    m_LastResurrectTime = 0;
    m_Queue_type        = MAX_BATTLEGROUND_QUEUE_RANGES;
    m_InvitedAlliance   = 0;
    m_InvitedHorde      = 0;
    m_ArenaType         = 0;
    m_IsArena           = false;
    m_Winner            = 2;
    m_ElaspedTime       = 0;
    m_Events            = 0;
    m_IsRated           = false;
    m_BuffChange        = false;
    m_Name              = "";
    m_LevelMin          = 0;
    m_LevelMax          = 0;
    m_InBGFreeSlotQueue = false;
    m_SetDeleteThis     = false;

    m_MaxPlayersPerTeam = 0;
    m_MaxPlayers        = 0;
    m_MinPlayersPerTeam = 0;
    m_MinPlayers        = 0;

    m_MapId             = 0;

    m_TeamStartLocX[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocX[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocY[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocY[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocZ[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocZ[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocO[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocO[BG_TEAM_HORDE]      = 0;

    m_ArenaTeamIds[BG_TEAM_ALLIANCE]   = 0;
    m_ArenaTeamIds[BG_TEAM_HORDE]      = 0;

    m_ArenaTeamRatingChanges[BG_TEAM_ALLIANCE]   = 0;
    m_ArenaTeamRatingChanges[BG_TEAM_HORDE]      = 0;

    m_BgRaids[BG_TEAM_ALLIANCE]         = nullptr;
    m_BgRaids[BG_TEAM_HORDE]            = nullptr;

    m_PlayersCount[BG_TEAM_ALLIANCE]    = 0;
    m_PlayersCount[BG_TEAM_HORDE]       = 0;

    m_PrematureCountDown = false;
    m_PrematureCountDown = 0;
    m_timeLimit = 0;

    m_HonorMode = BG_NORMAL;
    
    m_team1LogInfo.clear();
    m_team2LogInfo.clear();

    m_StartTimestamp = 0;
    m_StartDelayTime = 0;
    m_PrematureCountDownTimer = 0;

    m_Spectators.clear();
}

Battleground::~Battleground()
{
    // remove objects and creatures
    // (this is done automatically in mapmanager update, when the instance is reset after the reset time)
    int size = m_BgCreatures.size();
    for(int i = 0; i < size; ++i)
    {
        DelCreature(i);
    }
    size = m_BgObjects.size();
    for(int i = 0; i < size; ++i)
    {
        DelObject(i);
    }

    // delete creature and go respawn times
    WorldDatabase.PExecute("DELETE FROM creature_respawn WHERE instance = '%u'",GetInstanceID());
    WorldDatabase.PExecute("DELETE FROM gameobject_respawn WHERE instance = '%u'",GetInstanceID());
    // delete instance from db
    CharacterDatabase.PExecute("DELETE FROM instance WHERE id = '%u'",GetInstanceID());
    // remove from battlegrounds
    sBattlegroundMgr->RemoveBattleground(GetInstanceID());
    // unload map
    if(Map * map = sMapMgr->FindMap(GetMapId(), GetInstanceID()))
        if(map->IsBattlegroundOrArena())
        {
            ((BattlegroundMap*)map)->SetUnload();
            ((BattlegroundMap*)map)->SetBG(NULL);
        }
    // remove from bg free slot queue
    this->RemoveFromBGFreeSlotQueue();
    
    for (std::map<uint64, PlayerLogInfo*>::iterator itr = m_team1LogInfo.begin(); itr != m_team1LogInfo.end(); itr++)
        delete itr->second;
    for (std::map<uint64, PlayerLogInfo*>::iterator itr = m_team2LogInfo.begin(); itr != m_team2LogInfo.end(); itr++)
        delete itr->second;
}

void Battleground::Update(time_t diff)
{
    if(!GetPlayersSize() && !GetRemovedPlayersSize() && !GetReviveQueueSize() && !m_Spectators.size())
        //BG is empty
        return;

    if(GetStatus() == STATUS_IN_PROGRESS)
        m_ElaspedTime += diff;

    // WorldPacket data;

    if(GetRemovedPlayersSize())
    {
        for(std::map<uint64, uint8>::iterator itr = m_RemovedPlayers.begin(); itr != m_RemovedPlayers.end(); ++itr)
        {
            Player *plr = sObjectMgr->GetPlayer(itr->first);
            switch(itr->second)
            {
                //following code is handled by event:
                /*case 0:
                    sBattlegroundMgr->m_BattlegroundQueues[GetTypeID()].RemovePlayer(itr->first);
                    //RemovePlayerFromQueue(itr->first);
                    if(plr)
                    {
                        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, plr->GetTeam(), plr->GetBattlegroundQueueIndex(m_TypeID), STATUS_NONE, 0, 0);
                        plr->GetSession()->SendPacket(&data);
                    }
                    break;*/
                case 1:                                     // currently in bg and was removed from bg
                    if(plr)
                        RemovePlayerAtLeave(itr->first, true, true);
                    else
                        RemovePlayerAtLeave(itr->first, false, false);
                    break;
                case 2:                                     // revive queue
                    RemovePlayerFromResurrectQueue(itr->first);
                    break;
                default:
                    TC_LOG_ERROR("FIXME","Battleground: Unknown remove player case!");
            }
        }
        m_RemovedPlayers.clear();
    }

    // remove offline players from bg after MAX_OFFLINE_TIME
    if(GetPlayersSize())
    {
        for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player *plr = sObjectMgr->GetPlayer(itr->first);

            // Don't reset player last online time, he is allowed to be disconnected for 2 minutes in total during the whole battleground
            /*if(plr)
                itr->second.LastOnlineTime = 0;                 // update last online time*/
            if (!plr) {
                itr->second.ElapsedTimeDisconnected += diff;
                
                if(itr->second.ElapsedTimeDisconnected >= MAX_OFFLINE_TIME) {
                    CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '8' WHERE guid = %u", GUID_LOPART(itr->first)); // AT_LOGIN_SET_DESERTER
                    m_RemovedPlayers[itr->first] = 1;           // add to remove list (BG)
                }
            }
        }
    }

    m_LastResurrectTime += diff;
    if (m_LastResurrectTime >= RESURRECTION_INTERVAL)
    {
        if(GetReviveQueueSize())
        {
            for(std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
            {
                Creature *sh = NULL;
                for(std::vector<uint64>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
                {
                    Player *plr = sObjectMgr->GetPlayer(*itr2);
                    if(!plr)
                        continue;

                    if (!sh)
                    {
                        sh = ObjectAccessor::GetCreature(*plr, itr->first);
                        // only for visual effect
                        if (sh)
                            sh->CastSpell(sh, SPELL_SPIRIT_HEAL, true);   // Spirit Heal, effect 117
                    }

                    plr->CastSpell(plr, SPELL_RESURRECTION_VISUAL, true);   // Resurrection visual
                    m_ResurrectQueue.push_back(*itr2);
                }
                (itr->second).clear();
            }

            m_ReviveQueue.clear();
            m_LastResurrectTime = 0;
        }
        else
            // queue is clear and time passed, just update last resurrection time
            m_LastResurrectTime = 0;
    }
    else if (m_LastResurrectTime > 500)    // Resurrect players only half a second later, to see spirit heal effect on NPC
    {
        for(std::vector<uint64>::iterator itr = m_ResurrectQueue.begin(); itr != m_ResurrectQueue.end(); ++itr)
        {
            Player *plr = sObjectMgr->GetPlayer(*itr);
            if(!plr)
                continue;
            plr->ResurrectPlayer(1.0f);
            plr->CastSpell(plr, SPELL_SPIRIT_HEAL_MANA, true);
            sObjectAccessor->ConvertCorpseForPlayer(*itr);
        }
        m_ResurrectQueue.clear();
    }

    // if less then minimum players are in on one side, then start premature finish timer
    if(GetStatus() == STATUS_IN_PROGRESS && !IsArena() && sBattlegroundMgr->GetPrematureFinishTime() && (GetPlayersCountByTeam(TEAM_ALLIANCE) < GetMinPlayersPerTeam() || GetPlayersCountByTeam(TEAM_HORDE) < GetMinPlayersPerTeam()))
    {
        if(!m_PrematureCountDown)
        {
            m_PrematureCountDown = true;
            m_PrematureCountDownTimer = sBattlegroundMgr->GetPrematureFinishTime();
            SendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING);
        }
        else if(m_PrematureCountDownTimer < diff)
        {
            // time's up!
            EndBattleground(0); // noone wins
            m_PrematureCountDown = false;
        }
        else
        {
            uint32 newtime = m_PrematureCountDownTimer - diff;
            // announce every minute
            if(m_PrematureCountDownTimer != sBattlegroundMgr->GetPrematureFinishTime() && newtime / 60000 != m_PrematureCountDownTimer / 60000)
                SendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING);
            m_PrematureCountDownTimer = newtime;
        }
    }
    else if (m_PrematureCountDown)
        m_PrematureCountDown = false;

    if(GetStatus() == STATUS_WAIT_LEAVE)
    {
        // remove all players from battleground after 2 minutes
        m_RemovalTime += diff;
        if(m_RemovalTime >= TIME_TO_AUTOREMOVE)                 // 2 minutes
        {
            for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            {
                m_RemovedPlayers[itr->first] = 1;           // add to remove list (BG)
            }

            for (SpectatorList::iterator itr = m_Spectators.begin(); itr != m_Spectators.end(); ++itr)
            {
                m_RemovedPlayers[*itr] = 1;
            }
            // do not change any battleground's private variables
        }
    }

    //check time limit if any
    if(GetStatus() == STATUS_IN_PROGRESS && GetTimeLimit() && GetElapsedTime() > GetTimeLimit())
        EndBattleground(0);
}

void Battleground::SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O)
{
    uint8 idx = GetTeamIndexByTeamId(TeamID);
    m_TeamStartLocX[idx] = X;
    m_TeamStartLocY[idx] = Y;
    m_TeamStartLocZ[idx] = Z;
    m_TeamStartLocO[idx] = O;
}

void Battleground::SendPacketToAll(WorldPacket *packet)
{
    for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr->GetPlayer(itr->first);
        if(plr)
            plr->GetSession()->SendPacket(packet);
    }
}

void Battleground::SendPacketToTeam(uint32 TeamID, WorldPacket *packet, Player *sender, bool self)
{
    for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr->GetPlayer(itr->first);

        if (!plr)
            continue;

        if(!self && sender == plr)
            continue;

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            plr->GetSession()->SendPacket(packet);
    }
}

void Battleground::PlaySoundToAll(uint32 SoundID)
{
    WorldPacket data;
    sBattlegroundMgr->BuildPlaySoundPacket(&data, SoundID);
    SendPacketToAll(&data);
}

void Battleground::PlaySoundToTeam(uint32 SoundID, uint32 TeamID)
{
    WorldPacket data;

    for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr->GetPlayer(itr->first);

        if (!plr)
            continue;

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
        {
            sBattlegroundMgr->BuildPlaySoundPacket(&data, SoundID);
            plr->GetSession()->SendPacket(&data);
        }
    }
}

void Battleground::CastSpellOnTeam(uint32 SpellID, uint32 TeamID)
{
    for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr->GetPlayer(itr->first);

        if (!plr)
            continue;

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            plr->CastSpell(plr, SpellID, true);
    }
}

void Battleground::YellToAll(Creature* creature, const char* text, Language language)
{
    for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        WorldPacket data(SMSG_MESSAGECHAT, 200);
        Player *plr = sObjectMgr->GetPlayer(itr->first);
        if(!plr)
            continue;
        creature->BuildMonsterChat(&data,CHAT_MSG_MONSTER_YELL,text,language,creature->GetName().c_str(),itr->first);
        plr->GetSession()->SendPacket(&data);
    }
}


void Battleground::RewardHonorToTeam(uint32 Honor, uint32 TeamID)
{
    for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr->GetPlayer(itr->first);

        if(!plr)
            continue;

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            UpdatePlayerScore(plr, SCORE_BONUS_HONOR, Honor);
    }
}

void Battleground::RewardReputationToTeam(uint32 faction_id, uint32 Reputation, uint32 TeamID)
{
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if(!factionEntry)
        return;

    for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr->GetPlayer(itr->first);

        if(!plr)
            continue;

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            plr->ModifyFactionReputation(factionEntry, Reputation);
    }
}

void Battleground::UpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data;
    sBattlegroundMgr->BuildUpdateWorldStatePacket(&data, Field, Value);
    SendPacketToAll(&data);
}

void Battleground::UpdateWorldStateForPlayer(uint32 Field, uint32 Value, Player *Source)
{
    WorldPacket data;
    sBattlegroundMgr->BuildUpdateWorldStatePacket(&data, Field, Value);
    Source->GetSession()->SendPacket(&data);
}

void Battleground::EndBattleground(uint32 winner)
{
    this->RemoveFromBGFreeSlotQueue();
    uint32 almost_winning_team = TEAM_HORDE;
    ArenaTeam * winner_arena_team = NULL;
    ArenaTeam * loser_arena_team = NULL;
    uint32 loser_rating = 0;
    uint32 winner_rating = 0;
    uint32 final_loser_rating = 0;
    uint32 final_winner_rating = 0;
    WorldPacket data;
    Player *Source = NULL;
    const char *winmsg = "";
    
    // Stats
    uint32 finalScoreAlliance = m_score[GetTeamIndexByTeamId(TEAM_ALLIANCE)];
    uint32 finalScoreHorde = m_score[GetTeamIndexByTeamId(TEAM_HORDE)];
    
    if (finalScoreAlliance == 0x7FFFFF)
        finalScoreAlliance = 0;
    else if (finalScoreAlliance >= 2000 && (GetMapId() == 529 || GetMapId() == 566))
        finalScoreAlliance = 2000;
        
    if (finalScoreHorde == 0x7FFFFF)
        finalScoreHorde = 0;
    else if (finalScoreHorde >= 2000 && (GetMapId() == 529 || GetMapId() == 566))
        finalScoreHorde = 2000;

    if(winner == TEAM_ALLIANCE)
    {
        if(isBattleground()) {
            winmsg = GetTrinityString(LANG_BG_A_WINS);
            LogsDatabase.PExecute("INSERT INTO bg_stats (mapid, start_time, end_time, winner, score_alliance, score_horde) VALUES (%u, %u, %u, 0, %u, %u)", GetMapId(), GetStartTimestamp(), time(NULL), finalScoreAlliance, finalScoreHorde);
        }
        else
            winmsg = GetTrinityString(LANG_ARENA_GOLD_WINS);

        PlaySoundToAll(SOUND_ALLIANCE_WINS);                // alliance wins sound

        SetWinner(WINNER_ALLIANCE);
    }
    else if(winner == TEAM_HORDE)
    {
        if(isBattleground()) {
            winmsg = GetTrinityString(LANG_BG_H_WINS);
            LogsDatabase.PExecute("INSERT INTO bg_stats (mapid, start_time, end_time, winner, score_alliance, score_horde) VALUES (%u, %u, %u, 1, %u, %u)", GetMapId(), GetStartTimestamp(), time(NULL), finalScoreAlliance, finalScoreHorde);
        }
        else
            winmsg = GetTrinityString(LANG_ARENA_GREEN_WINS);

        PlaySoundToAll(SOUND_HORDE_WINS);                   // horde wins sound

        SetWinner(WINNER_HORDE);
    }
    else
    {
        SetWinner(3);
    }

    SetStatus(STATUS_WAIT_LEAVE);
    m_RemovalTime = 0;

    // arena rating calculation
    if(IsArena() && isRated())
    {
        if(winner == TEAM_ALLIANCE)
        {
            winner_arena_team = sObjectMgr->GetArenaTeamById(GetArenaTeamIdForTeam(TEAM_ALLIANCE));
            loser_arena_team = sObjectMgr->GetArenaTeamById(GetArenaTeamIdForTeam(TEAM_HORDE));
        }
        else if(winner == TEAM_HORDE)
        {
            winner_arena_team = sObjectMgr->GetArenaTeamById(GetArenaTeamIdForTeam(TEAM_HORDE));
            loser_arena_team = sObjectMgr->GetArenaTeamById(GetArenaTeamIdForTeam(TEAM_ALLIANCE));
        }
        if(winner_arena_team && loser_arena_team)
        {
            loser_rating = loser_arena_team->GetStats().rating;
            winner_rating = winner_arena_team->GetStats().rating;
            int32 winner_change = winner_arena_team->WonAgainst(loser_rating);
            int32 loser_change = loser_arena_team->LostAgainst(winner_rating);
            if(winner == TEAM_ALLIANCE)
            {
                SetArenaTeamRatingChangeForTeam(TEAM_ALLIANCE, winner_change);
                SetArenaTeamRatingChangeForTeam(TEAM_HORDE, loser_change);
            }
            else
            {
                SetArenaTeamRatingChangeForTeam(TEAM_HORDE, winner_change);
                SetArenaTeamRatingChangeForTeam(TEAM_ALLIANCE, loser_change);
            }
            
            final_loser_rating = loser_arena_team->GetStats().rating;
            final_winner_rating = winner_arena_team->GetStats().rating;


            if (sWorld->getConfig(CONFIG_ARMORY_ENABLE))
            {
                uint32 maxChartID;
                QueryResult result = CharacterDatabase.PQuery("SELECT MAX(gameid) FROM armory_game_chart");
                if(!result)
                    maxChartID = 0;
                else
                {
                    Field* fields = result->Fetch();
                    maxChartID = fields[0].GetUInt32();
                }
                uint32 gameID = maxChartID+1;
                for (BattlegroundScoreMap::const_iterator itr = GetPlayerScoresBegin();itr !=GetPlayerScoresEnd(); ++itr)
                {
                    Player* player = ObjectAccessor::FindPlayer(itr->first);
                    if (!player)
                        continue;
                    uint32 plTeamID = player->GetArenaTeamId(winner_arena_team->GetSlot());
                    int changeType;
                    uint32 resultRating;
                    uint32 resultTeamID;
                    int32 ratingChange;
                    if (plTeamID == winner_arena_team->GetId())
                    {
                        changeType = 1; //win
                        resultRating = final_winner_rating;
                        resultTeamID = plTeamID;
                        ratingChange = winner_change;
                    }
                    else
                    {
                        changeType = 2; //lose
                        resultRating = final_loser_rating;
                        resultTeamID = loser_arena_team->GetId();
                        ratingChange = loser_change;
                    }
                    std::ostringstream sql_query;
                    //                                                        gameid,              teamid,                     guid,                    changeType,             ratingChange,               teamRating,                  damageDone,                          deaths,                          healingDone,                           damageTaken,,                           healingTaken,                         killingBlows,                      mapId,                 start,                       end
                    sql_query << "INSERT INTO armory_game_chart VALUES ('" << gameID << "', '" << resultTeamID << "', '" << player->GetGUID() << "', '" << changeType << "', '" << ratingChange  << "', '" << resultRating << "', '" << itr->second->DamageDone << "', '" << itr->second->Deaths << "', '" << itr->second->HealingDone << "', '" << itr->second->DamageTaken << "', '" << itr->second->HealingTaken << "', '" << itr->second->KillingBlows << "', '" << m_MapId << "', '" << m_StartTime << "', '" << TIME_TO_AUTOREMOVE << "')";
                    CharacterDatabase.Execute(sql_query.str().c_str());
                }
            }

            TC_LOG_DEBUG("arena","Arena match Type: %u for Team1Id: %u - Team2Id: %u ended. WinnerTeamId: %u. Winner rating: %u, Loser rating: %u. RatingChange: %i.", m_ArenaType, m_ArenaTeamIds[BG_TEAM_ALLIANCE], m_ArenaTeamIds[BG_TEAM_HORDE], winner_arena_team->GetId(), final_winner_rating, final_loser_rating, winner_change);
            for (BattlegroundScoreMap::const_iterator itr = GetPlayerScoresBegin();itr !=GetPlayerScoresEnd(); ++itr) {
                if (Player* player = sObjectMgr->GetPlayer(itr->first)) {
                    TC_LOG_DEBUG("arena","Statistics for %s (GUID: " UI64FMTD ", Team Id: %d, IP: %s): %u damage, %u healing, %u killing blows", player->GetName().c_str(), itr->first, player->GetArenaTeamId(m_ArenaType == 5 ? 2 : m_ArenaType == 3), player->GetSession()->GetRemoteAddress().c_str(), itr->second->DamageDone, itr->second->HealingDone, itr->second->KillingBlows);
                    //LogsDatabase.PExecute("INSERT INTO arena_match_player (match_id, player_guid, player_name, team, ip, heal, damage, killing_blows) VALUES (%u, " UI64FMTD ", '%s', %u, '%s', %u, %u, %u)", matchId, itr->first, player->GetName(), player->GetArenaTeamId(m_ArenaType == 5 ? 2 : m_ArenaType == 3), player->GetSession()->GetRemoteAddress().c_str(), itr->second->DamageDone, itr->second->HealingDone, itr->second->KillingBlows);
                    uint32 team = GetPlayerTeam(itr->first);
                    if (team == TEAM_ALLIANCE) {
                        std::map<uint64, PlayerLogInfo*>::iterator itr2 = m_team1LogInfo.find(itr->first);
                        if (itr2 != m_team1LogInfo.end()) {
                            itr2->second->damage = itr->second->DamageDone;
                            itr2->second->heal = itr->second->HealingDone;
                            itr2->second->kills = itr->second->KillingBlows;
                        }
                    }
                    else {
                        std::map<uint64, PlayerLogInfo*>::iterator itr2 = m_team2LogInfo.find(itr->first);
                        if (itr2 != m_team2LogInfo.end()) {
                            itr2->second->damage = itr->second->DamageDone;
                            itr2->second->heal = itr->second->HealingDone;
                            itr2->second->kills = itr->second->KillingBlows;
                        }
                    }
                }
            }
            std::ostringstream ss;
            ss << "INSERT INTO arena_match (`type`, team1, team1_member1, team1_member1_ip, team1_member1_heal, team1_member1_damage, team1_member1_kills, ";
            ss << "team1_member2, team1_member2_ip, team1_member2_heal, team1_member2_damage, team1_member2_kills, ";
            ss << "team1_member3, team1_member3_ip, team1_member3_heal, team1_member3_damage, team1_member3_kills, ";
            ss << "team1_member4, team1_member4_ip, team1_member4_heal, team1_member4_damage, team1_member4_kills, ";
            ss << "team1_member5, team1_member5_ip, team1_member5_heal, team1_member5_damage, team1_member5_kills, ";
            ss << "team2, ";
            ss << "team2_member1, team2_member1_ip, team2_member1_heal, team2_member1_damage, team2_member1_kills, ";
            ss << "team2_member2, team2_member2_ip, team2_member2_heal, team2_member2_damage, team2_member2_kills, ";
            ss << "team2_member3, team2_member3_ip, team2_member3_heal, team2_member3_damage, team2_member3_kills, ";
            ss << "team2_member4, team2_member4_ip, team2_member4_heal, team2_member4_damage, team2_member4_kills, ";
            ss << "team2_member5, team2_member5_ip, team2_member5_heal, team2_member5_damage, team2_member5_kills, ";
            ss << "start_time, end_time, winner, rating_change, winner_rating, loser_rating, team1_name, team2_name) VALUES (";
            ss << uint32(m_ArenaType) << ", " << m_ArenaTeamIds[BG_TEAM_ALLIANCE] << ", ";
            for (std::map<uint64, PlayerLogInfo*>::iterator itr = m_team1LogInfo.begin(); itr != m_team1LogInfo.end(); itr++)
                ss << itr->second->guid << ", '" << itr->second->ip.c_str() << "', " << itr->second->heal << ", " << itr->second->damage << ", " << uint32(itr->second->kills) << ", ";
            for (uint8 i = 0; i < (5 - m_team1LogInfo.size()); i++)
                ss << "0, '', 0, 0, 0, ";
            ss << m_ArenaTeamIds[BG_TEAM_HORDE] << ", ";
            for (std::map<uint64, PlayerLogInfo*>::iterator itr = m_team2LogInfo.begin(); itr != m_team2LogInfo.end(); itr++)
                ss << itr->second->guid << ", '" << itr->second->ip.c_str() << "', " << itr->second->heal << ", " << itr->second->damage << ", " << uint32(itr->second->kills) << ", ";
            for (uint8 i = 0; i < (5 - m_team2LogInfo.size()); i++)
                ss << "0, '', 0, 0, 0, ";
            ss << GetStartTimestamp() << ", " << time(NULL) << ", " << winner_arena_team->GetId() << ", " << winner_change << ", ";
            ss << final_winner_rating << ", " << final_loser_rating;
            if (winner == TEAM_ALLIANCE)
                ss << ", '" << winner_arena_team->GetName() << "', '" << loser_arena_team->GetName() << "')";
            else if (winner == TEAM_HORDE)
                ss << ", '" << loser_arena_team->GetName() << "', '" << winner_arena_team->GetName() << "')";
            LogsDatabase.Execute(ss.str().c_str());
            //LogsDatabase.PExecute("INSERT INTO arena_match (type, team1, team2, team1_members, team2_members, start_time, end_time, winner, rating_change) VALUES (%u, %u, %u, \"%s\", \"%s\", %u, %u, %u, %u)", m_ArenaType, m_ArenaTeamIds[BG_TEAM_ALLIANCE], m_ArenaTeamIds[BG_TEAM_HORDE], oss_team1Members.str().c_str(), oss_team2Members.str().c_str(), GetStartTimestamp(), time(NULL), winner_arena_team->GetId(), winner_change);
        }
        else
        {
            SetArenaTeamRatingChangeForTeam(TEAM_ALLIANCE, 0);
            SetArenaTeamRatingChangeForTeam(TEAM_HORDE, 0);
        }
    }

    if (!IsArena()) {
        if(m_score[GetTeamIndexByTeamId(TEAM_ALLIANCE)] == m_score[GetTeamIndexByTeamId(TEAM_HORDE)])
            almost_winning_team = 0;         //no real winner
        if(m_score[GetTeamIndexByTeamId(TEAM_ALLIANCE)] > m_score[GetTeamIndexByTeamId(TEAM_HORDE)])
            almost_winning_team = TEAM_ALLIANCE;
    }

    for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr->GetPlayer(itr->first);
        if(!plr)
            continue;

        // should remove spirit of redemption
        if(plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            plr->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

        if(!plr->IsAlive())
        {
            plr->ResurrectPlayer(1.0f);
            plr->SpawnCorpseBones();
        }

        uint32 team = itr->second.Team;
        if(!team) team = plr->GetTeam();

        // per player calculation
        if(IsArena() && isRated() && winner_arena_team && loser_arena_team)
        {
            if(team == winner)
                winner_arena_team->MemberWon(plr,loser_rating);
            else
                loser_arena_team->MemberLost(plr,winner_rating);
        }

        if (team == winner) {
            if(!Source)
                Source = plr;
            RewardMark(plr,ITEM_WINNER_COUNT);
            UpdatePlayerScore(plr, SCORE_BONUS_HONOR, 20);
            RewardQuest(plr);
        }
        else if (winner !=0)
            RewardMark(plr,ITEM_LOSER_COUNT);
    else if(winner == 0)
    {
        if(sWorld->getConfig(CONFIG_PREMATURE_BG_REWARD))    // We're feeling generous, giving rewards to people who not earned them ;)
        {    //nested ifs for the win! its boring writing that, forgive me my unfunniness
            if (GetTypeID() == BATTLEGROUND_AV)             // Only 1 mark for alterac
                RewardMark(plr,ITEM_LOSER_COUNT);
            else if(almost_winning_team == team)                    //player's team had more points
                RewardMark(plr,ITEM_WINNER_COUNT);
            else
                RewardMark(plr,ITEM_LOSER_COUNT);            // if scores were the same, each team gets 1 mark.
        }
    }

        plr->SetHealth(plr->GetMaxHealth());
        plr->SetPower(POWER_MANA, plr->GetMaxPower(POWER_MANA));
        plr->CombatStopWithPets(true);

        BlockMovement(plr);

        sBattlegroundMgr->BuildPvpLogDataPacket(&data, this);
        plr->GetSession()->SendPacket(&data);

        uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(GetTypeID(), GetArenaType());
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, plr->GetTeam(), plr->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetElapsedTime());
        plr->GetSession()->SendPacket(&data);
    }

    for (SpectatorList::iterator itr = m_Spectators.begin(); itr != m_Spectators.end(); ++itr)
    {
        Player *plr = sObjectMgr->GetPlayer(*itr);
        if(!plr)
            continue;

        BlockMovement(plr);

        sBattlegroundMgr->BuildPvpLogDataPacket(&data, this);
        plr->GetSession()->SendPacket(&data);

        uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(GetTypeID(), GetArenaType());
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, plr->GetTeam(), 0, STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetElapsedTime());
        plr->GetSession()->SendPacket(&data);
    }
    
    if(IsArena() && isRated() && winner_arena_team && loser_arena_team)
    {
        // update arena points only after increasing the player's match count!
        //obsolete: winner_arena_team->UpdateArenaPointsHelper();
        //obsolete: loser_arena_team->UpdateArenaPointsHelper();
        // save the stat changes
        winner_arena_team->SaveToDB();
        loser_arena_team->SaveToDB();
        // send updated arena team stats to players
        // this way all arena team members will get notified, not only the ones who participated in this match
        winner_arena_team->NotifyStatsChanged();
        loser_arena_team->NotifyStatsChanged();
    }

    // inform invited players about the removal
    sBattlegroundMgr->m_BattlegroundQueues[sBattlegroundMgr->BGQueueTypeId(GetTypeID(), GetArenaType())].BGEndedRemoveInvites(this);

    if(Source)
    {
        ChatHandler::BuildChatPacket(data, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, Source->GetGUID(), 0, winmsg, 0);
        SendPacketToAll(&data);
    }
}

uint32 Battleground::GetBattlemasterEntry() const
{
    switch(GetTypeID())
    {
        case BATTLEGROUND_AV: return 15972;
        case BATTLEGROUND_WS: return 14623;
        case BATTLEGROUND_AB: return 14879;
        case BATTLEGROUND_EY: return 22516;
        case BATTLEGROUND_NA: return 20200;
        default:              return 0;
    }
}

void Battleground::SetStatus(uint32 Status)       
{ 
    m_Status = Status; 

    if (m_Status == STATUS_WAIT_JOIN && !IsArena() && sWorld->getConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_ENABLE))
    {
        if (sWorld->getConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_WORLDONLY))
        {
            std::ostringstream msg;
            msg << "|cffffff00[Annonceur BG] " << GetName() << "|r [" << GetMaxLevel() << "] " << "a commencé.";
            //ugly hack to get a player just to get to the ChatHandler
            HashMapHolder<Player>::MapType& m = sObjectAccessor->GetPlayers();
            auto itr = m.begin();
            if(itr->second)
                ChatHandler(itr->second).SendMessageWithoutAuthor("world", msg.str().c_str());
        } else
            sWorld->SendWorldText(LANG_BG_STARTED_ANNOUNCE_WORLD, GetName().c_str(), /*GetMinLevel(),*/ GetMaxLevel()); //Min level system is wrong and not complete
    }
}

void Battleground::RewardMark(Player *plr,uint32 count)
{
    // 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    if(plr->GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
        return;

    if(!plr || !count)
        return;
   
    // Give less marks if the player has been disconnected during the battleground
    if (count == 3) { // Winner
        std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.find(plr->GetGUIDLow());
        if (itr != m_Players.end()) {
            float ratio = itr->second.ElapsedTimeDisconnected / (float) MAX_OFFLINE_TIME * 100.f;
            if (ratio <= 33.3f)
                count = 3;
            else if (ratio <= 66.66f)
                count = 2;
            else
                count = 1;
        }
    } else if (count == 1) { // Loser
        std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.find(plr->GetGUIDLow());
        if (itr != m_Players.end()) {
            float ratio = itr->second.ElapsedTimeDisconnected / (float) MAX_OFFLINE_TIME * 100.f;
            if (ratio <= 50.0f)
                count = 1;
            else
                count = 0;
        }
    }

    BattlegroundMarks mark;    
    switch(GetTypeID())
    {
        case BATTLEGROUND_AV:
            mark = ITEM_AV_MARK_OF_HONOR;
            break;
        case BATTLEGROUND_WS:
            mark = ITEM_WS_MARK_OF_HONOR;
            break;
        case BATTLEGROUND_AB:
            mark = ITEM_AB_MARK_OF_HONOR;
            break;
        case BATTLEGROUND_EY:            
            mark = ITEM_EY_MARK_OF_HONOR;
            break;
        default:
            return;
    }

    if ( ItemTemplate const *pProto = sObjectMgr->GetItemTemplate( mark ) )
    {
        ItemPosCountVec dest;
        uint32 no_space_count = 0;
        uint8 msg = plr->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, mark, count, &no_space_count, pProto );
        if( msg != EQUIP_ERR_OK )                       // convert to possible store amount
            count -= no_space_count;

        if(!dest.empty())                // can add some
            if(Item* item = plr->StoreNewItem( dest, mark, true, 0, pProto))
                plr->SendNewItem(item,count,false,true);

        if(no_space_count > 0)
            SendRewardMarkByMail(plr,mark,no_space_count);
    }
}

void Battleground::SendRewardMarkByMail(Player *plr,uint32 mark, uint32 count)
{
    uint32 bmEntry = GetBattlemasterEntry();
    if(!bmEntry)
        return;

    ItemTemplate const* markProto = sObjectMgr->GetItemTemplate(mark);
    if(!markProto)
        return;

    if(Item* markItem = Item::CreateItem(mark,count,plr,markProto))
    {
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        // save new item before send
        markItem->SaveToDB(trans);                               // save for prevent lost at next mail load, if send fail then item will deleted
        CharacterDatabase.CommitTransaction(trans);

        // item
        MailItemsInfo mi;
        mi.AddItem(markItem->GetGUIDLow(), markItem->GetEntry(), markItem);

        // subject: item name
        std::string subject = plr->GetSession()->GetLocalizedItemName(markProto);
        if(subject.empty())
            subject = "-";

        // text
        std::string textFormat = plr->GetSession()->GetTrinityString(LANG_BG_MARK_BY_MAIL);
        char textBuf[300];
        snprintf(textBuf,300,textFormat.c_str(),GetName().c_str(),GetName().c_str());
        uint32 itemTextId = sObjectMgr->CreateItemText( textBuf );

        WorldSession::SendMailTo(plr, MAIL_CREATURE, MAIL_STATIONERY_NORMAL, bmEntry, plr->GetGUIDLow(), subject, itemTextId , &mi, 0, 0, MAIL_CHECK_MASK_NONE);
    }
}

void Battleground::RewardQuest(Player *plr)
{
    // 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    if(plr->GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
        return;

    uint32 quest;
    switch(GetTypeID())
    {
        case BATTLEGROUND_AV:
            quest = SPELL_AV_QUEST_REWARD;
            break;
        case BATTLEGROUND_WS:
            quest = SPELL_WS_QUEST_REWARD;
            break;
        case BATTLEGROUND_AB:
            quest = SPELL_AB_QUEST_REWARD;
            break;
        case BATTLEGROUND_EY:
            quest = SPELL_EY_QUEST_REWARD;
            break;
        default:
            return;
    }

    plr->CastSpell(plr, quest, true);
}

void Battleground::BlockMovement(Player *plr)
{
    plr->SetClientControl(plr, 0);                          // movement disabled NOTE: the effect will be automatically removed by client when the player is teleported from the battleground, so no need to send with uint8(1) in RemovePlayerAtLeave()
}

void Battleground::RemovePlayerAtLeave(uint64 guid, bool Transport, bool SendPacket)
{
    if (isSpectator(guid))
    {
        if (Player* player = ObjectAccessor::FindPlayer(guid))
        {
            player->CancelSpectate();

            uint32 map = player->GetBattlegroundEntryPointMap();
            float positionX = player->GetBattlegroundEntryPointX();
            float positionY = player->GetBattlegroundEntryPointY();
            float positionZ = player->GetBattlegroundEntryPointZ();
            float positionO = player->GetBattlegroundEntryPointO();
            if (player->TeleportTo(map, positionX, positionY, positionZ, positionO))
            {
                player->SetSpectate(false);
                RemoveSpectator(player->GetGUID());
            }
        }
        return;
    }
    uint32 team = GetPlayerTeam(guid);
    bool participant = false;
    // Remove from lists/maps
    std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.find(guid);
    if(itr != m_Players.end())
    {
        UpdatePlayersCountByTeam(team, true);   // -1 player
        m_Players.erase(itr);
        // check if the player was a participant of the match, or only entered through gm command (goname)
        participant = true;
    }

    std::map<uint64, BattlegroundScore*>::iterator itr2 = m_PlayerScores.find(guid);
    if(itr2 != m_PlayerScores.end())
    {
        if (team == TEAM_ALLIANCE) {
            std::map<uint64, PlayerLogInfo*>::iterator itr3 = m_team1LogInfo.find(itr2->first);
            if (itr3 != m_team1LogInfo.end()) {
                itr3->second->damage = itr2->second->DamageDone;
                itr3->second->heal = itr2->second->HealingDone;
                itr3->second->kills = itr2->second->KillingBlows;
            }
        }
        else {
            std::map<uint64, PlayerLogInfo*>::iterator itr3 = m_team2LogInfo.find(itr2->first);
            if (itr3 != m_team2LogInfo.end()) {
                itr3->second->damage = itr2->second->DamageDone;
                itr3->second->heal = itr2->second->HealingDone;
                itr3->second->kills = itr2->second->KillingBlows;
            }
        }

        delete itr2->second;                                // delete player's score
        m_PlayerScores.erase(itr2);
    }

    RemovePlayerFromResurrectQueue(guid);

    Player *plr = sObjectMgr->GetPlayer(guid);

    // should remove spirit of redemption
    if(plr && plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        plr->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

    if(plr && !plr->IsAlive())                              // resurrect on exit
    {
        plr->ResurrectPlayer(1.0f);
        plr->SpawnCorpseBones();
    }

    RemovePlayer(plr, guid);                                // BG subclass specific code
    DecreaseInvitedCount(team);

    if(plr)
    {
        plr->ClearAfkReports();

        if(participant) // if the player was a match participant, remove auras, calc rating, update queue
        {
            if(!team) team = plr->GetTeam();

            uint32 bgTypeId = GetTypeID();
            uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(GetTypeID(), GetArenaType());
            // if arena, remove the specific arena auras
            if(IsArena())
            {
                plr->RemoveArenaAuras(true);    // removes debuffs / dots etc., we don't want the player to die after porting out
                bgTypeId=BATTLEGROUND_AA;       // set the bg type to all arenas (it will be used for queue refreshing)

                // summon old pet if there was one and there isn't a current pet
                if(!plr->GetPet() && plr->GetTemporaryUnsummonedPetNumber())
                {
                    Pet* NewPet = new Pet;
                    if(!NewPet->LoadPetFromDB(plr, 0, plr->GetTemporaryUnsummonedPetNumber(), true))
                        delete NewPet;

                    (plr)->SetTemporaryUnsummonedPetNumber(0);
                }

                if(isRated() && GetStatus() == STATUS_IN_PROGRESS)
                {
                    //left a rated match while the encounter was in progress, consider as loser
                    ArenaTeam * winner_arena_team = 0;
                    ArenaTeam * loser_arena_team = 0;
                    if(team == TEAM_HORDE)
                    {
                        winner_arena_team = sObjectMgr->GetArenaTeamById(GetArenaTeamIdForTeam(TEAM_ALLIANCE));
                        loser_arena_team = sObjectMgr->GetArenaTeamById(GetArenaTeamIdForTeam(TEAM_HORDE));
                    }
                    else
                    {
                        winner_arena_team = sObjectMgr->GetArenaTeamById(GetArenaTeamIdForTeam(TEAM_HORDE));
                        loser_arena_team = sObjectMgr->GetArenaTeamById(GetArenaTeamIdForTeam(TEAM_ALLIANCE));
                    }
                    if(winner_arena_team && loser_arena_team)
                    {
                        loser_arena_team->MemberLost(plr,winner_arena_team->GetRating());
                    }
                }
            }
            else        // BG: simply remove debuffs
                plr->RemoveArenaAuras(true);
            

            WorldPacket data;
            if(SendPacket)
            {
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, team, plr->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_NONE, 0, 0);
                plr->GetSession()->SendPacket(&data);
            }

            // this call is important, because player, when joins to battleground, this method is not called, so it must be called when leaving bg
            plr->RemoveBattlegroundQueueId(bgQueueTypeId);

            //we should update battleground queue, but only if bg isn't ending
            if (GetQueueType() < MAX_BATTLEGROUND_QUEUE_RANGES)
                sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].Update(bgTypeId, GetQueueType());

            Group * group = plr->GetGroup();
            // remove from raid group if exist
            if(group && group == GetBgRaid(team))
            {
                if(!group->RemoveMember(guid, 0))               // group was disbanded
                {
                    SetBgRaid(team, NULL);
                    delete group;
                }
            }

            // Let others know
            sBattlegroundMgr->BuildPlayerLeftBattlegroundPacket(&data, plr);
            SendPacketToTeam(team, &data, plr, false);
        }

        // Do next only if found in battleground
        plr->SetBattlegroundId(0);                          // We're not in BG.
        // reset destination bg team
        plr->SetBGTeam(0);

        if(Transport)
        {
            if (!plr->TeleportTo(plr->GetBattlegroundEntryPointMap(), plr->GetBattlegroundEntryPointX(), plr->GetBattlegroundEntryPointY(), plr->GetBattlegroundEntryPointZ(), plr->GetBattlegroundEntryPointO()))
                plr->TeleportTo(plr->m_homebindMapId, plr->m_homebindX, plr->m_homebindY, plr->m_homebindZ, plr->GetOrientation());
        }

        // Log
        TC_LOG_DEBUG("battleground","BATTLEGROUND: Removed player %s from Battleground.", plr->GetName().c_str());
    }

    if(!GetPlayersSize() && !GetInvitedCount(TEAM_HORDE) && !GetInvitedCount(TEAM_ALLIANCE) && !m_Spectators.size())
    {
        // if no players left AND no invitees left AND no spectators left, set this bg to delete in next update
        // direct deletion could cause crashes
        m_SetDeleteThis = true;

        // return to prevent addition to freeslotqueue
        return;
    }

    // a player exited the battleground, so there are free slots. add to queue
    this->AddToBGFreeSlotQueue();
}

// this method is called when no players remains in battleground
void Battleground::Reset()
{
    SetQueueType(MAX_BATTLEGROUND_QUEUE_RANGES);
    SetWinner(WINNER_NONE);
    SetStatus(STATUS_WAIT_QUEUE);
    SetRemovalTimer(0);
    SetLastResurrectTime(0);
    SetArenaType(0);
    SetRated(false);

    m_Events = 0;

    if (m_InvitedAlliance > 0 || m_InvitedHorde > 0)
        TC_LOG_ERROR("battleground","Battleground system ERROR: bad counter, m_InvitedAlliance: %d, m_InvitedHorde: %d", m_InvitedAlliance, m_InvitedHorde);

    m_InvitedAlliance = 0;
    m_InvitedHorde = 0;
    m_InBGFreeSlotQueue = false;

    m_Players.clear();
    m_Spectators.clear();
    m_PlayerScores.clear();

    // reset BGSubclass
    ResetBGSubclass();
}

void Battleground::StartBattleground()
{
    SetLastResurrectTime(0);
    SetStartTimestamp(time(NULL));
    if(m_IsRated) 
        TC_LOG_DEBUG("arena","Arena match type: %u for Team1Id: %u - Team2Id: %u started.", m_ArenaType, m_ArenaTeamIds[BG_TEAM_ALLIANCE], m_ArenaTeamIds[BG_TEAM_HORDE]);
}

void Battleground::onAddSpectator(Player *spectator)
{
}

void Battleground::AddPlayer(Player *plr)
{
    if (isSpectator(plr->GetGUID()))
        return;

    // remove afk from player
    if (plr->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK))
        plr->ToggleAFK();
        
    // score struct must be created in inherited class

    uint64 guid = plr->GetGUID();
    uint32 team = plr->GetBGTeam();

    BattlegroundPlayer bp;
    bp.ElapsedTimeDisconnected = 0;
    bp.Team = team;

    // Add to list/maps
    m_Players[guid] = bp;

    UpdatePlayersCountByTeam(team, false);                  // +1 player

    WorldPacket data;
    sBattlegroundMgr->BuildPlayerJoinedBattlegroundPacket(&data, plr);
    SendPacketToTeam(team, &data, plr, false);
    
    //remove custom morphs to prevent abuses
    plr->RestoreDisplayId();

    // add arena specific auras
    if(IsArena())
    {
        plr->RemoveArenaSpellCooldowns();
        plr->RemoveArenaAuras();
        plr->RemoveAllEnchantments(TEMP_ENCHANTMENT_SLOT, true);
        if(team == TEAM_ALLIANCE)                                // gold
        {
            if(plr->GetTeam() == TEAM_HORDE)
                plr->CastSpell(plr, SPELL_HORDE_GOLD_FLAG,true);
            else
                plr->CastSpell(plr, SPELL_ALLIANCE_GOLD_FLAG,true);
        }
        else                                                // green
        {
            if(plr->GetTeam() == TEAM_HORDE)
                plr->CastSpell(plr, SPELL_HORDE_GREEN_FLAG,true);
            else
                plr->CastSpell(plr, SPELL_ALLIANCE_GREEN_FLAG,true);
        }

        plr->DestroyConjuredItems(true);
        plr->UnsummonPetTemporaryIfAny();

        if(GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
        {
            plr->CastSpell(plr, SPELL_ARENA_PREPARATION, true);

            plr->SetHealth(plr->GetMaxHealth());
            plr->SetPower(POWER_MANA, plr->GetMaxPower(POWER_MANA));
        }
    }
    else
    {
        if(GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
            plr->CastSpell(plr, SPELL_PREPARATION, true);   // reduces all mana cost of spells.
    }

    // Log
    TC_LOG_DEBUG("battleground","BATTLEGROUND: Player %s joined the battle.", plr->GetName().c_str());
}

/* This method should be called only once ... it adds pointer to queue */
void Battleground::AddToBGFreeSlotQueue()
{
    // make sure to add only once
    if(!m_InBGFreeSlotQueue)
    {
        sBattlegroundMgr->BGFreeSlotQueue[m_TypeID].push_front(this);
        m_InBGFreeSlotQueue = true;
    }
}

/* This method removes this battleground from free queue - it must be called when deleting battleground - not used now*/
void Battleground::RemoveFromBGFreeSlotQueue()
{
    // set to be able to re-add if needed
    m_InBGFreeSlotQueue = false;
    // uncomment this code when battlegrounds will work like instances
    for (std::deque<Battleground*>::iterator itr = sBattlegroundMgr->BGFreeSlotQueue[m_TypeID].begin(); itr != sBattlegroundMgr->BGFreeSlotQueue[m_TypeID].end(); ++itr)
    {
        if ((*itr)->GetInstanceID() == m_InstanceID)
        {
            sBattlegroundMgr->BGFreeSlotQueue[m_TypeID].erase(itr);
            return;
        }
    }
}

// get the number of free slots for team
// works in similar way that HasFreeSlotsForTeam did, but this is needed for join as group
uint32 Battleground::GetFreeSlotsForTeam(uint32 Team) const
{
    //if BG is starting ... invite anyone
    /*if (GetStatus() == STATUS_WAIT_JOIN)
        return (GetInvitedCount(Team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) : 0;*/
    // If BG is starting, invite same amount of players of each side... Just like STATUS_IN_PROGRESS
    //if BG is already started .. do not allow to join too much players of one faction
    uint32 otherTeam;
    uint32 otherIn;
    if (Team == TEAM_ALLIANCE)
    {
        otherTeam = GetInvitedCount(TEAM_HORDE);
        otherIn = GetPlayersCountByTeam(TEAM_HORDE);
    }
    else
    {
        otherTeam = GetInvitedCount(TEAM_ALLIANCE);
        otherIn = GetPlayersCountByTeam(TEAM_ALLIANCE);
    }
    if (GetStatus() == STATUS_IN_PROGRESS || GetStatus() == STATUS_WAIT_JOIN)
    {
        // difference based on ppl invited (not necessarily entered battle)
        // default: allow 0
        uint32 diff = 0;
        // allow join one person if the sides are equal (to fill up bg to minplayersperteam)
        if (otherTeam == GetInvitedCount(Team))
            diff = 3;
        // allow join more ppl if the other side has more players
        else if(otherTeam > GetInvitedCount(Team))
            diff = otherTeam - GetInvitedCount(Team);

        // difference based on max players per team (don't allow inviting more)
        uint32 diff2 = (GetInvitedCount(Team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) : 0;

        // difference based on players who already entered
        // default: allow 0
        uint32 diff3 = 0;
        // allow join one person if the sides are equal (to fill up bg minplayersperteam)
        if (otherIn == GetPlayersCountByTeam(Team))
            diff3 = 3;
        // allow join more ppl if the other side has more players
        else if (otherIn > GetPlayersCountByTeam(Team))
            diff3 = otherIn - GetPlayersCountByTeam(Team);
        // or other side has less than minPlayersPerTeam
        else if (GetInvitedCount(Team) <= GetMinPlayersPerTeam())
            diff3 = GetMinPlayersPerTeam() - GetInvitedCount(Team) + 1;

        // return the minimum of the 3 differences

        // min of diff and diff 2
        diff = diff < diff2 ? diff : diff2;

        // min of diff, diff2 and diff3
        return diff < diff3 ? diff : diff3 ;
    }

    return 0;
}

bool Battleground::HasFreeSlots() const
{
    return GetPlayersSize() < GetMaxPlayers();
}

void Battleground::UpdatePlayerScore(Player *Source, uint32 type, uint32 value)
{
    //this procedure is called from virtual function implemented in bg subclass
    std::map<uint64, BattlegroundScore*>::iterator itr = m_PlayerScores.find(Source->GetGUID());

    if(itr == m_PlayerScores.end())                         // player not found...
        return;

    switch(type)
    {
        case SCORE_KILLING_BLOWS:                           // Killing blows
            itr->second->KillingBlows += value;
            break;
        case SCORE_DEATHS:                                  // Deaths
            itr->second->Deaths += value;
            break;
        case SCORE_HONORABLE_KILLS:                         // Honorable kills
            itr->second->HonorableKills += value;
            break;
        case SCORE_BONUS_HONOR:                             // Honor bonus
            // do not add honor in arenas
            if(isBattleground())
            {
                // reward honor instantly
                if(Source->RewardHonor(NULL, 1, value))
                    itr->second->BonusHonor += value;
            }
            break;
            //used only in EY, but in MSG_PVP_LOG_DATA opcode
        case SCORE_DAMAGE_DONE:                             // Damage Done
            itr->second->DamageDone += value;
            break;
        case SCORE_HEALING_DONE:                            // Healing Done
            itr->second->HealingDone += value;
            break;
        case SCORE_DAMAGE_TAKEN:
            itr->second->DamageTaken += value;              // Damage Taken
            break;
        case SCORE_HEALING_TAKEN:
            itr->second->HealingTaken += value;             // Healing Taken
            break;
        default:
            TC_LOG_ERROR("battleground","Battleground: Unknown player score type %u", type);
            break;
    }
}

void Battleground::AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid)
{
    m_ReviveQueue[npc_guid].push_back(player_guid);

    Player *plr = sObjectMgr->GetPlayer(player_guid);
    if(!plr)
        return;

    plr->CastSpell(plr, SPELL_WAITING_FOR_RESURRECT, true);
    SpellEntry const *spellInfo = sSpellMgr->GetSpellInfo( SPELL_WAITING_FOR_RESURRECT );
    if(spellInfo)
    {
        Aura *Aur = CreateAura(spellInfo, 0, NULL, plr);
        plr->AddAura(Aur);
    }
}

void Battleground::RemovePlayerFromResurrectQueue(uint64 player_guid)
{
    for(std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
    {
        for(std::vector<uint64>::iterator itr2 =(itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
        {
            if(*itr2 == player_guid)
            {
                (itr->second).erase(itr2);

                Player *plr = sObjectMgr->GetPlayer(player_guid);
                if(!plr)
                    return;

                plr->RemoveAurasDueToSpell(SPELL_WAITING_FOR_RESURRECT);

                return;
            }
        }
    }
}

bool Battleground::AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime, bool inactive)
{
    Map * map = sMapMgr->FindMap(GetMapId(),GetInstanceID());
    if(!map)
        return false;

    // must be created this way, adding to godatamap would add it to the base map of the instance
    // and when loading it (in go::LoadFromDB()), a new guid would be assigned to the object, and a new object would be created
    // so we must create it specific for this instance
    GameObject * go = new GameObject;
    if(!go->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_GAMEOBJECT,true),entry, map,x,y,z,o,rotation0,rotation1,rotation2,rotation3,100,1))
    {
        TC_LOG_ERROR("battleground","Gameobject template %u not found in database! Battleground not created!", entry);
        TC_LOG_ERROR("battleground","Cannot create gameobject template %u! Battleground not created!", entry);
        delete go;
        return false;
    }

    if(inactive)
        go->SetInactive(true);

    // add to world, so it can be later looked up from HashMapHolder
    map->Add(go);
    m_BgObjects[type] = go->GetGUID();
    return true;
}

//some doors aren't despawned so we cannot handle their closing in gameobject::update()
//it would be nice to correctly implement GO_ACTIVATED state and open/close doors in gameobject code
void Battleground::DoorClose(uint32 type)
{
    GameObject *obj = HashMapHolder<GameObject>::Find(m_BgObjects[type]);
    if(obj)
    {
        //if doors are open, close it
        if( obj->getLootState() == GO_ACTIVATED && !obj->GetGoState() )
        {
            //change state to allow door to be closed
            obj->SetLootState(GO_READY);
            obj->UseDoorOrButton(RESPAWN_ONE_DAY);
        }
    }
    else
    {
        TC_LOG_ERROR("FIXME","Battleground: Door object not found (cannot close doors)");
    }
}

void Battleground::DoorOpen(uint32 type)
{
    GameObject *obj = HashMapHolder<GameObject>::Find(m_BgObjects[type]);
    if(obj)
    {
        //change state to be sure they will be opened
        obj->SetLootState(GO_READY);
        obj->UseDoorOrButton(RESPAWN_ONE_DAY);
    }
    else
    {
        TC_LOG_ERROR("battleground","Battleground: Door object not found! - doors will be closed.");
    }
}

GameObject* Battleground::GetBGObject(uint32 type)
{
    GameObject *obj = HashMapHolder<GameObject>::Find(m_BgObjects[type]);
    if(!obj)
        TC_LOG_ERROR("battleground","couldn't get gameobject %i",type);
    return obj;
}

Creature* Battleground::GetBGCreature(uint32 type)
{
    Creature *creature = HashMapHolder<Creature>::Find(m_BgCreatures[type]);
    if(!creature)
        TC_LOG_ERROR("battleground","couldn't get creature %i",type);
    return creature;
}

void Battleground::SpawnBGObject(uint32 type, uint32 respawntime)
{
    Map * map = sMapMgr->FindMap(GetMapId(),GetInstanceID());
    if(!map)
        return;
    if( respawntime == 0 )
    {
        GameObject *obj = HashMapHolder<GameObject>::Find(m_BgObjects[type]);
        if(obj)
        {
            //we need to change state from GO_JUST_DEACTIVATED to GO_READY in case battleground is starting again
            if( obj->getLootState() == GO_JUST_DEACTIVATED)
                obj->SetLootState(GO_READY);
            if( obj->IsInactive() )
                obj->SetInactive(false);
            obj->SetRespawnTime(0);
            map->Add(obj);
        }
    }
    else
    {
        GameObject *obj = HashMapHolder<GameObject>::Find(m_BgObjects[type]);
        if(obj)
        {
            map->Add(obj);
            obj->SetRespawnTime(respawntime);
            obj->SetLootState(GO_JUST_DEACTIVATED);
        }
    }
}

Creature* Battleground::AddCreature(uint32 entry, uint32 type, uint32 teamval, float x, float y, float z, float o, uint32 respawntime)
{
    Map * map = sMapMgr->FindMap(GetMapId(),GetInstanceID());
    if(!map)
        return NULL;

    Creature* pCreature = new Creature;
    if (!pCreature->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT,true), map, entry, teamval))
    {
        TC_LOG_ERROR("battleground","Can't create creature entry: %u",entry);
        delete pCreature;
        return NULL;
    }

    pCreature->Relocate(x, y, z, o);

    if(!pCreature->IsPositionValid())
    {
        TC_LOG_ERROR("battleground","ERROR: Creature (guidlow %d, entry %d) not added to battleground. Suggested coordinates isn't valid (X: %f Y: %f)",pCreature->GetGUIDLow(),pCreature->GetEntry(),pCreature->GetPositionX(),pCreature->GetPositionY());
        delete pCreature;
        return NULL;
    }

    pCreature->SetHomePosition(x, y, z, o);
    pCreature->AIM_Initialize();

    //pCreature->SetDungeonDifficulty(0);

    map->Add(pCreature);
    m_BgCreatures[type] = pCreature->GetGUID();

    return  pCreature;
}
/*
void Battleground::SpawnBGCreature(uint32 type, uint32 respawntime)
{
    Map * map = sMapMgr->FindMap(GetMapId(),GetInstanceId());
    if(!map)
        return false;

    if(respawntime == 0)
    {
        Creature *obj = HashMapHolder<Creature>::Find(m_BgCreatures[type]);
        if(obj)
        {
            //obj->Respawn();                               // bugged
            obj->SetRespawnTime(0);
            sObjectMgr->SaveCreatureRespawnTime(obj->GetGUIDLow(), GetInstanceID(), 0);
            map->Add(obj);
        }
    }
    else
    {
        Creature *obj = HashMapHolder<Creature>::Find(m_BgCreatures[type]);
        if(obj)
        {
            obj->SetDeathState(DEAD);
            obj->SetRespawnTime(respawntime);
            map->Add(obj);
        }
    }
}
*/
bool Battleground::DelCreature(uint32 type)
{
    if(!m_BgCreatures[type])
        return true;

    Creature *cr = HashMapHolder<Creature>::Find(m_BgCreatures[type]);
    if(!cr)
    {
        TC_LOG_ERROR("battleground","Can't find creature guid: %u",GUID_LOPART(m_BgCreatures[type]));
        return false;
    }

    cr->AI()->EnterEvadeMode();
    cr->AddObjectToRemoveList();
    m_BgCreatures[type] = 0;
    return true;
}

bool Battleground::DelObject(uint32 type)
{
    if(!m_BgObjects[type])
        return true;

    GameObject *obj = HashMapHolder<GameObject>::Find(m_BgObjects[type]);
    if(!obj)
    {
        TC_LOG_ERROR("battleground","Can't find gobject guid: %u",GUID_LOPART(m_BgObjects[type]));
        return false;
    }
    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete();
    m_BgObjects[type] = 0;
    return true;
}

bool Battleground::AddSpiritGuide(uint32 type, float x, float y, float z, float o, uint32 team)
{
    uint32 entry = 0;

    if(team == TEAM_ALLIANCE)
        entry = 13116;
    else
        entry = 13117;

    Creature* pCreature = AddCreature(entry,type,team,x,y,z,o);
    if(!pCreature)
    {
        TC_LOG_ERROR("FIXME","Can't create Spirit guide. Battleground not created!");
        EndNow();
        return false;
    }

    pCreature->SetDeathState(DEAD);

    pCreature->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, pCreature->GetGUID());
    // aura
    pCreature->SetUInt32Value(UNIT_FIELD_AURA, SPELL_SPIRIT_HEAL_CHANNEL);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAFLAGS, 0x00000009);
    pCreature->SetUInt32Value(UNIT_FIELD_AURALEVELS, 0x0000003C);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS, 0x000000FF);
    // casting visual effect
    pCreature->SetUInt32Value(UNIT_CHANNEL_SPELL, SPELL_SPIRIT_HEAL_CHANNEL);
    // correct cast speed
    pCreature->SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    //pCreature->CastSpell(pCreature, SPELL_SPIRIT_HEAL_CHANNEL, true);

    return true;
}

void Battleground::SendMessageToAll(char const* text)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, nullptr, nullptr, text);
    SendPacketToAll(&data);
}

void Battleground::SendMessageToAll(int32 entry)
{
    char const* text = GetTrinityString(entry);
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, nullptr, nullptr, text);
    SendPacketToAll(&data);
}

void Battleground::EndNow()
{
    RemoveFromBGFreeSlotQueue();
    SetStatus(STATUS_WAIT_LEAVE);
    SetRemovalTimer(TIME_TO_AUTOREMOVE);
    // inform invited players about the removal
    sBattlegroundMgr->m_BattlegroundQueues[sBattlegroundMgr->BGQueueTypeId(GetTypeID(), GetArenaType())].BGEndedRemoveInvites(this);
}

// Battleground messages are localized using the dbc lang, they are not client language dependent
const char *Battleground::GetTrinityString(int32 entry)
{
    // FIXME: now we have different DBC locales and need localized message for each target client
    return sObjectMgr->GetTrinityStringForDBCLocale(entry);
}

/*
important notice:
buffs aren't spawned/despawned when players captures anything
buffs are in their positions when battleground starts
*/
void Battleground::HandleTriggerBuff(uint64 const& go_guid)
{
    GameObject *obj = HashMapHolder<GameObject>::Find(go_guid);
    if(!obj || obj->GetGoType() != GAMEOBJECT_TYPE_TRAP || !obj->isSpawned())
        return;

    //change buff type, when buff is used:
    int32 index = m_BgObjects.size() - 1;
    while (index >= 0 && m_BgObjects[index] != go_guid)
        index--;
    if (index < 0)
    {
        TC_LOG_ERROR("battleground","Battleground (Type: %u) has buff gameobject (Guid: %u Entry: %u Type:%u) but it hasn't that object in its internal data",GetTypeID(),GUID_LOPART(go_guid),obj->GetEntry(),obj->GetGoType());
        return;
    }

    //randomly select new buff
    uint8 buff = urand(0, 2);
    uint32 entry = obj->GetEntry();
    if( m_BuffChange && entry != Buff_Entries[buff] )
    {
        //despawn current buff
        SpawnBGObject(index, RESPAWN_ONE_DAY);
        //set index for new one
        for (uint8 currBuffTypeIndex = 0; currBuffTypeIndex < 3; ++currBuffTypeIndex)
            if( entry == Buff_Entries[currBuffTypeIndex] )
            {
                index -= currBuffTypeIndex;
                index += buff;
            }
    }

    SpawnBGObject(index, BUFF_RESPAWN_TIME);
}

void Battleground::HandleKillPlayer( Player *player, Player *killer )
{
    //keep in mind that for arena this will have to be changed a bit

    // add +1 deaths
    UpdatePlayerScore(player, SCORE_DEATHS, 1);

    // add +1 kills to group and +1 killing_blows to killer
    if( killer )
    {
        UpdatePlayerScore(killer, SCORE_HONORABLE_KILLS, 1);
        UpdatePlayerScore(killer, SCORE_KILLING_BLOWS, 1);

        for(std::map<uint64, BattlegroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player *plr = sObjectMgr->GetPlayer(itr->first);

            if(!plr || plr == killer)
                continue;

            if( plr->GetTeam() == killer->GetTeam() && plr->IsAtGroupRewardDistance(player) )
                UpdatePlayerScore(plr, SCORE_HONORABLE_KILLS, 1);
        }
    }

    // to be able to remove insignia -- ONLY IN Battlegrounds
    if( !IsArena() )
        player->SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE );
}

// return the player's team based on battlegroundplayer info
// used in same faction arena matches mainly
uint32 Battleground::GetPlayerTeam(uint64 guid)
{
    std::map<uint64, BattlegroundPlayer>::const_iterator itr = m_Players.find(guid);
    if(itr != m_Players.end())
        return itr->second.Team;
    return 0;
}

bool Battleground::IsPlayerInBattleground(uint64 guid)
{
    std::map<uint64, BattlegroundPlayer>::const_iterator itr = m_Players.find(guid);
    if(itr!=m_Players.end())
        return true;
    return false;
}

void Battleground::PlayerRelogin(uint64 guid)
{
    if(GetStatus() != STATUS_WAIT_LEAVE)
        return;

    Player *plr = sObjectMgr->GetPlayer(guid);
    if(!plr)
        return;

    WorldPacket data;
    uint32 bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());

    BlockMovement(plr);

    sBattlegroundMgr->BuildPvpLogDataPacket(&data, this);
    plr->GetSession()->SendPacket(&data);

    sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, plr->GetTeam(), plr->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetElapsedTime());
    plr->GetSession()->SendPacket(&data);
}

uint32 Battleground::GetAlivePlayersCountByTeam(uint32 Team) const
{
    int count = 0;
    for(std::map<uint64, BattlegroundPlayer>::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if(itr->second.Team == Team)
        {
            Player * pl = sObjectMgr->GetPlayer(itr->first);
            if(pl && pl->IsAlive())
                ++count;
        }
    }
    return count;
}

void Battleground::SetHoliday(bool is_holiday)
{
    if(is_holiday)
        m_HonorMode = BG_HOLIDAY;
    else
        m_HonorMode = BG_NORMAL;
}

int32 Battleground::GetObjectType(uint64 guid)
{
    for(uint32 i = 0;i < m_BgObjects.size(); i++)
        if(m_BgObjects[i] == guid)
            return i;
    TC_LOG_ERROR("FIXME","Battleground: cheating? a player used a gameobject which isnt supposed to be a usable object!");
    return -1;
}

void Battleground::HandleKillUnit(Creature *creature, Player *killer)
{
}

// This method should be called when player logs out from running battleground
void Battleground::EventPlayerLoggedOut(Player* player)
{
    if( GetStatus() == STATUS_IN_PROGRESS )
    {
        if (!isSpectator(player->GetGUID()))
        {
            if( isBattleground() )
                EventPlayerDroppedFlag(player);
            else if( IsArena() )
                player->LeaveBattleground();
        }
    }

    if (isSpectator(player->GetGUID()))
    {
        player->CancelSpectate();

        uint32 map = player->GetBattlegroundEntryPointMap();
        float positionX = player->GetBattlegroundEntryPointX();
        float positionY = player->GetBattlegroundEntryPointY();
        float positionZ = player->GetBattlegroundEntryPointZ();
        float positionO = player->GetBattlegroundEntryPointO();

        if (player->TeleportTo(map, positionX, positionY, positionZ, positionO))
        {
            player->SetSpectate(false);
            RemoveSpectator(player->GetGUID());
        }
    }
}

void Battleground::PlayerInvitedInRatedArena(Player* player, uint32 team)
{
    PlayerLogInfo* logInfo = new PlayerLogInfo;
    logInfo->guid = player->GetGUIDLow();
    logInfo->ip = player->GetSession()->GetRemoteAddress();
    logInfo->heal = 0;
    logInfo->damage = 0;
    logInfo->kills = 0;

    if (team == TEAM_ALLIANCE)
        m_team1LogInfo[player->GetGUID()] = logInfo;
    else
        m_team2LogInfo[player->GetGUID()] = logInfo;
}

void Battleground::SendSpectateAddonsMsg(SpectatorAddonMsg msg)
{
    if (!HaveSpectators())
        return;

    for (SpectatorList::iterator itr = m_Spectators.begin(); itr != m_Spectators.end(); ++itr)
        msg.SendPacket(*itr);
}

bool Battleground::isSpectator(uint64 guid)
{
    for(std::set<uint64>::iterator itr = m_Spectators.begin(); itr != m_Spectators.end(); ++itr)
    {
        if (guid == *itr)
            return true;
    }

    return false;
}

bool Battleground::canEnterSpectator(Player *spectator)
{
    if (isSpectator(spectator->GetGUID()))
        return false;

    if (m_Spectators.size() < sWorld->getConfig(CONFIG_ARENA_SPECTATOR_MAX))
        return true;

    return false;
}
