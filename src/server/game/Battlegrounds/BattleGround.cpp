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
#include "ArenaTeamMgr.h"
#include "World.h"
#include "Util.h"
#include "SpellMgr.h"
#include "LogsDatabaseAccessor.h"
#include "Mail.h"
#include "Transport.h"
#include "GameTime.h"
#include "BattleGroundAV.h"
#include "BattleGroundWS.h"
#include "BattleGroundEY.h"
#include "BattleGroundAB.h"
#include "ReputationMgr.h"

namespace Trinity
{
    class BattlegroundChatBuilder
    {
    public:
        BattlegroundChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, va_list* args = nullptr)
            : _msgtype(msgtype), _textId(textId), _source(source), _args(args) { }

        void operator()(WorldPacket& data, LocaleConstant loc_idx)
        {
            char const* text = sObjectMgr->GetTrinityString(_textId, loc_idx);
            if (_args)
            {
                // we need copy va_list before use or original va_list will corrupted
                va_list ap;
                va_copy(ap, *_args);

                char str[2048];
                vsnprintf(str, 2048, text, ap);
                va_end(ap);

                do_helper(data, &str[0]);
            }
            else
                do_helper(data, text);
        }

    private:
        void do_helper(WorldPacket& data, char const* text)
        {
            ChatHandler::BuildChatPacket(data, _msgtype, LANG_UNIVERSAL, _source, _source, text);
        }

        ChatMsg _msgtype;
        int32 _textId;
        Player const* _source;
        va_list* _args;
    };

    class Battleground2ChatBuilder
    {
    public:
        Battleground2ChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, int32 arg1, int32 arg2)
            : _msgtype(msgtype), _textId(textId), _source(source), _arg1(arg1), _arg2(arg2) {}

        void operator()(WorldPacket& data, LocaleConstant loc_idx)
        {
            char const* text = sObjectMgr->GetTrinityString(_textId, loc_idx);
            char const* arg1str = _arg1 ? sObjectMgr->GetTrinityString(_arg1, loc_idx) : "";
            char const* arg2str = _arg2 ? sObjectMgr->GetTrinityString(_arg2, loc_idx) : "";

            char str[2048];
            snprintf(str, 2048, text, arg1str, arg2str);

            ChatHandler::BuildChatPacket(data, _msgtype, LANG_UNIVERSAL, _source, _source, str);
        }

    private:
        ChatMsg _msgtype;
        int32 _textId;
        Player const* _source;
        int32 _arg1;
        int32 _arg2;
    };
}                                                           // namespace Trinity

template<class Do>
void Battleground::BroadcastWorker(Do& _do)
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player* p = ObjectAccessor::FindConnectedPlayer(itr->first);
        if (p)
            _do(p);
    }
}

Battleground::Battleground()
{
    m_TypeID            = BATTLEGROUND_TYPE_NONE;
    m_InstanceID        = 0;
    m_Status            = 0;
    m_ClientInstanceID  = 0;
    m_StartTime         = 0;
    m_EndTime           = 0;
    m_ValidStartPositionTimer = 0;
    m_LastResurrectTime = 0;
    m_BracketId         = BG_BRACKET_ID_FIRST;
    m_InvitedAlliance   = 0;
    m_InvitedHorde      = 0;
    m_ArenaType         = 0;
    m_IsArena           = false;
    m_Winner            = 2;
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
    m_Map               = nullptr;
    m_StartMaxDist      = 0.0f;
    ScriptId            = 0;

    _arenaTeamIds[TEAM_ALLIANCE]   = 0;
    _arenaTeamIds[TEAM_HORDE]      = 0;

    m_ArenaTeamMMR[TEAM_ALLIANCE] = 0;
    m_ArenaTeamMMR[TEAM_HORDE]    = 0;

    m_ArenaTeamRatingChanges[TEAM_ALLIANCE]   = 0;
    m_ArenaTeamRatingChanges[TEAM_HORDE]      = 0;

    m_BgRaids[TEAM_ALLIANCE]         = nullptr;
    m_BgRaids[TEAM_HORDE]            = nullptr;

    m_PlayersCount[TEAM_ALLIANCE]    = 0;
    m_PlayersCount[TEAM_HORDE]       = 0;

    m_score[TEAM_ALLIANCE] = 0;
    m_score[TEAM_HORDE]    = 0;

    m_PrematureCountDown = false;
    m_PrematureCountDown = 0;
    m_timeLimit = 0;

    m_HonorMode = BG_NORMAL;
    
    m_team1LogInfo.clear();
    m_team2LogInfo.clear();

    m_StartDelayTime = 0;
    m_PrematureCountDownTimer = 0;

    m_Spectators.clear();
}

Battleground::~Battleground()
{
    // remove objects and creatures
    // (this is done automatically in mapmanager update, when the instance is reset after the reset time)
    uint32 size = BgCreatures.size();
    for(int i = 0; i < size; ++i)
        DelCreature(i);

    size = BgObjects.size();
    for(int i = 0; i < size; ++i)
        DelObject(i);


    // delete instance from db
    CharacterDatabase.PExecute("DELETE FROM instance WHERE id = '%u'", GetInstanceID());

    sBattlegroundMgr->RemoveBattleground(GetTypeID(), GetInstanceID());
    // unload map
    if (m_Map)
    {
        m_Map->DeleteRespawnTimes();
        m_Map->SetUnload();
        //unlink to prevent crash, always unlink all pointer reference before destruction
        m_Map->SetBG(nullptr);
        m_Map = nullptr;
    }
    // remove from bg free slot queue
    RemoveFromBGFreeSlotQueue();
    
    for (auto & itr : m_team1LogInfo)
        delete itr.second;
    for (auto & itr : m_team2LogInfo)
        delete itr.second;

    for (auto itr = PlayerScores.begin(); itr != PlayerScores.end(); ++itr)
        delete itr->second;
}

void Battleground::Update(time_t diff)
{
    if(!GetPlayersSize() && !GetReviveQueueSize() && !m_Spectators.size())
        return; //BG is empty

    m_StartTime += diff;

    switch (GetStatus())
    {
        case STATUS_WAIT_JOIN:
            if (GetPlayersSize())
            {
                _CheckSafePositions(diff);
            }
            break;
        case STATUS_IN_PROGRESS:
            _ProcessOfflineQueue(diff);
            // after 45 minutes without one team losing, the arena closes with no winner and no rating change
            if (IsArena())
            {
                if (GetStartTime() >= 45 * MINUTE*IN_MILLISECONDS)
                {
                    EndBattleground(0);
                    return;
                }
            }
            else {
                //_ProcessResurrect(diff);
            }
            break;
        case STATUS_WAIT_LEAVE:
            _ProcessLeave(diff);
            break;
        default:
            break;
    }

    m_LastResurrectTime += diff;
    if (m_LastResurrectTime >= RESURRECTION_INTERVAL)
    {
        if(GetReviveQueueSize())
        {
            for(auto & itr : m_ReviveQueue)
            {
                Creature *sh = nullptr;
                for(auto itr2 = (itr.second).begin(); itr2 != (itr.second).end(); ++itr2)
                {
                    Player *plr = ObjectAccessor::FindPlayer(*itr2);
                    if(!plr)
                        continue;

                    if (!sh)
                    {
                        sh = ObjectAccessor::GetCreature(*plr, itr.first);
                        // only for visual effect
                        if (sh)
                            sh->CastSpell(sh, SPELL_SPIRIT_HEAL, true);   // Spirit Heal, effect 117
                    }

                    plr->CastSpell(plr, SPELL_RESURRECTION_VISUAL, true);   // Resurrection visual
                    m_ResurrectQueue.push_back(*itr2);
                }
                (itr.second).clear();
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
        for(ObjectGuid& itr : m_ResurrectQueue)
        {
            Player *plr = ObjectAccessor::FindPlayer(itr);
            if(!plr)
                continue;
            plr->ResurrectPlayer(1.0f);
            plr->CastSpell(plr, SPELL_SPIRIT_HEAL_MANA, true);
            plr->GetMap()->ConvertCorpseToBones(itr);
        }
        m_ResurrectQueue.clear();
    }

    // if less then minimum players are in on one side, then start premature finish timer
    if(GetStatus() == STATUS_IN_PROGRESS && !IsArena() 
        && sBattlegroundMgr->GetPrematureFinishTime() 
        && (GetPlayersCountByTeam(ALLIANCE) < GetMinPlayersPerTeam() || GetPlayersCountByTeam(HORDE) < GetMinPlayersPerTeam())
        && !(IsArena() && sBattlegroundMgr->IsArenaTesting())
        && !(isBattleground() && sBattlegroundMgr->IsBattleGroundTesting())
        )
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

    //check time limit if any
    if(GetStatus() == STATUS_IN_PROGRESS && GetTimeLimit() && GetStartTime() > GetTimeLimit())
        EndBattleground(0);

}

inline void Battleground::_ProcessLeave(uint32 diff)
{
    // *********************************************************
    // ***           BATTLEGROUND ENDING SYSTEM              ***
    // *********************************************************
    // remove all players from battleground after 2 minutes
    m_EndTime -= diff;
    if (m_EndTime <= 0)
    {
        m_EndTime = 0;
        BattlegroundPlayerMap::iterator itr, next;
        for (itr = m_Players.begin(); itr != m_Players.end(); itr = next)
        {
            next = itr;
            ++next;
            //itr is erased here!
            RemovePlayerAtLeave(itr->first, true, true);// remove player from BG
                                                        // do not change any battleground's private variables
        }

        for (ObjectGuid const& m_Spectator : m_Spectators)
            RemovePlayerAtLeave(m_Spectator, true, true);
    }


}

void Battleground::SetTeamStartPosition(TeamId teamID, Position const& pos)
{
    ASSERT(teamID < TEAM_NEUTRAL);
    StartPosition[teamID] = pos;
}

Position const* Battleground::GetTeamStartPosition(TeamId teamId) const
{
    ASSERT(teamId < TEAM_NEUTRAL);
    return &StartPosition[teamId];
}

void Battleground::SendPacketToAll(WorldPacket *packet)
{
    for(auto & m_Player : m_Players)
    {
        Player *plr = ObjectAccessor::FindPlayer(m_Player.first);
        if(plr)
            plr->SendDirectMessage(packet);
    }
}

void Battleground::SendPacketToTeam(uint32 TeamID, WorldPacket *packet, Player *sender, bool self)
{
    for(auto & m_Player : m_Players)
    {
        Player *plr = ObjectAccessor::FindPlayer(m_Player.first);

        if (!plr)
            continue;

        if(!self && sender == plr)
            continue;

        uint32 team = m_Player.second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            plr->SendDirectMessage(packet);
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

    for(auto & m_Player : m_Players)
    {
        Player *plr = ObjectAccessor::FindPlayer(m_Player.first);

        if (!plr)
            continue;

        uint32 team = m_Player.second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
        {
            sBattlegroundMgr->BuildPlaySoundPacket(&data, SoundID);
            plr->SendDirectMessage(&data);
        }
    }
}

void Battleground::CastSpellOnTeam(uint32 SpellID, uint32 TeamID)
{
    for(auto & m_Player : m_Players)
    {
        Player *plr = ObjectAccessor::FindPlayer(m_Player.first);

        if (!plr)
            continue;

        uint32 team = m_Player.second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            plr->CastSpell(plr, SpellID, true);
    }
}

void Battleground::RewardHonorToTeam(uint32 Honor, uint32 TeamID)
{
    for(auto & m_Player : m_Players)
    {
        Player *plr = ObjectAccessor::FindPlayer(m_Player.first);

        if(!plr)
            continue;

        uint32 team = m_Player.second.Team;//GetPlayerTeam(plr->GetGUID());
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

    for(auto & m_Player : m_Players)
    {
        Player *plr = ObjectAccessor::FindPlayer(m_Player.first);

        if(!plr)
            continue;

        uint32 team = m_Player.second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            plr->GetReputationMgr().ModifyReputation(factionEntry, Reputation);

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
    Source->SendDirectMessage(&data);
}

void Battleground::EndBattleground(uint32 winner)
{
    RemoveFromBGFreeSlotQueue();
    uint32 almost_winning_team = HORDE;
    ArenaTeam * winner_arena_team = nullptr;
    ArenaTeam * loser_arena_team = nullptr;
    uint32 loser_rating = 0;
    uint32 winner_rating = 0;
    uint32 final_loser_rating = 0;
    uint32 final_winner_rating = 0;
    Player *Source = nullptr;
    const char *winmsg = "";
    
    // Stats
    uint32 finalScoreAlliance = m_score[GetTeamIndexByTeamId(ALLIANCE)];
    uint32 finalScoreHorde = m_score[GetTeamIndexByTeamId(HORDE)];
    
    if (finalScoreAlliance == 0x7FFFFF)
        finalScoreAlliance = 0;
    else if (finalScoreAlliance >= 2000 && (GetMapId() == 529 || GetMapId() == 566))
        finalScoreAlliance = 2000;
        
    if (finalScoreHorde == 0x7FFFFF)
        finalScoreHorde = 0;
    else if (finalScoreHorde >= 2000 && (GetMapId() == 529 || GetMapId() == 566))
        finalScoreHorde = 2000;

    std::string winnerLog;
    if(winner == ALLIANCE)
    {
        if(isBattleground())
            winmsg = GetTrinityString(LANG_BG_A_WINS);
        else
            winmsg = GetTrinityString(LANG_ARENA_GOLD_WINS);

        PlaySoundToAll(SOUND_ALLIANCE_WINS);                // alliance wins sound

        SetWinner(WINNER_ALLIANCE);
    }
    else if(winner == HORDE)
    {
        if(isBattleground())
            winmsg = GetTrinityString(LANG_BG_H_WINS);
        else
            winmsg = GetTrinityString(LANG_ARENA_GREEN_WINS);

        PlaySoundToAll(SOUND_HORDE_WINS);                   // horde wins sound

        SetWinner(WINNER_HORDE);
    }
    else
    {
        SetWinner(3);
    }

    if (isBattleground())
        LogsDatabaseAccessor::BattlegroundStats(GetMapId(), GetStartTimestamp(), time(nullptr), Team(winner), finalScoreAlliance, finalScoreHorde);

    SetStatus(STATUS_WAIT_LEAVE);
    //we must set it this way, because end time is sent in packet!
    m_EndTime = TIME_TO_AUTOREMOVE;

    // arena rating calculation
    if(IsArena() && isRated())
    {
        if(winner == ALLIANCE)
        {
            winner_arena_team = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(ALLIANCE));
            loser_arena_team = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(HORDE));
        }
        else if(winner == HORDE)
        {
            winner_arena_team = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(HORDE));
            loser_arena_team = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(ALLIANCE));
        }
        if(winner_arena_team && loser_arena_team)
        {
            loser_rating = loser_arena_team->GetStats().Rating;
            winner_rating = winner_arena_team->GetStats().Rating;
            int32 winner_change = winner_arena_team->WonAgainst(loser_rating);
            int32 loser_change = loser_arena_team->LostAgainst(winner_rating);
            if(winner == ALLIANCE)
            {
                SetArenaTeamRatingChangeForTeam(ALLIANCE, winner_change);
                SetArenaTeamRatingChangeForTeam(HORDE, loser_change);
            }
            else
            {
                SetArenaTeamRatingChangeForTeam(HORDE, winner_change);
                SetArenaTeamRatingChangeForTeam(ALLIANCE, loser_change);
            }
            
            final_loser_rating = loser_arena_team->GetStats().Rating;
            final_winner_rating = winner_arena_team->GetStats().Rating;

            TC_LOG_DEBUG("arena","Arena match Type: %u for Team1Id: %u - Team2Id: %u ended. WinnerTeamId: %u. Winner rating: %u, Loser rating: %u. RatingChange: %i.", m_ArenaType, _arenaTeamIds[TEAM_ALLIANCE], _arenaTeamIds[TEAM_HORDE], winner_arena_team->GetId(), final_winner_rating, final_loser_rating, winner_change);
            for (auto itr = GetPlayerScoresBegin();itr !=GetPlayerScoresEnd(); ++itr) {
                if (Player* player = ObjectAccessor::FindPlayer(itr->first)) {
                    TC_LOG_DEBUG("arena","Statistics for %s (GUID: %u, Team Id: %d, IP: %s): %u damage, %u healing, %u killing blows", player->GetName().c_str(), itr->first.GetCounter(), player->GetArenaTeamId(m_ArenaType == 5 ? 2 : m_ArenaType == 3), player->GetSession()->GetRemoteAddress().c_str(), itr->second->DamageDone, itr->second->HealingDone, itr->second->KillingBlows);
                    //LogsDatabase.PExecute("INSERT INTO arena_match_player (match_id, player_guid, player_name, team, ip, heal, damage, killing_blows) VALUES (%u, " UI64FMTD ", '%s', %u, '%s', %u, %u, %u)", matchId, itr->first, player->GetName(), player->GetArenaTeamId(m_ArenaType == 5 ? 2 : m_ArenaType == 3), player->GetSession()->GetRemoteAddress().c_str(), itr->second->DamageDone, itr->second->HealingDone, itr->second->KillingBlows);
                    uint32 team = GetPlayerTeam(itr->first);
                    if (team == ALLIANCE) {
                        auto itr2 = m_team1LogInfo.find(itr->first);
                        if (itr2 != m_team1LogInfo.end()) {
                            itr2->second->damage = itr->second->DamageDone;
                            itr2->second->heal = itr->second->HealingDone;
                            itr2->second->kills = itr->second->KillingBlows;
                        }
                    }
                    else {
                        auto itr2 = m_team2LogInfo.find(itr->first);
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
            ss << uint32(m_ArenaType) << ", " << _arenaTeamIds[TEAM_ALLIANCE] << ", ";
            for (auto & itr : m_team1LogInfo)
                ss << itr.second->guid << ", '" << itr.second->ip.c_str() << "', " << itr.second->heal << ", " << itr.second->damage << ", " << uint32(itr.second->kills) << ", ";
            for (uint8 i = 0; i < (5 - m_team1LogInfo.size()); i++)
                ss << "0, '', 0, 0, 0, ";
            ss << _arenaTeamIds[TEAM_HORDE] << ", ";
            for (auto & itr : m_team2LogInfo)
                ss << itr.second->guid << ", '" << itr.second->ip.c_str() << "', " << itr.second->heal << ", " << itr.second->damage << ", " << uint32(itr.second->kills) << ", ";
            for (uint8 i = 0; i < (5 - m_team2LogInfo.size()); i++)
                ss << "0, '', 0, 0, 0, ";
            ss << GetStartTimestamp() << ", " << time(nullptr) << ", " << winner_arena_team->GetId() << ", " << winner_change << ", ";
            ss << final_winner_rating << ", " << final_loser_rating;
            if (winner == ALLIANCE)
                ss << ", '" << winner_arena_team->GetName() << "', '" << loser_arena_team->GetName() << "')";
            else if (winner == HORDE)
                ss << ", '" << loser_arena_team->GetName() << "', '" << winner_arena_team->GetName() << "')";
            LogsDatabase.Execute(ss.str().c_str());
            //LogsDatabase.PExecute("INSERT INTO arena_match (type, team1, team2, team1_members, team2_members, start_time, end_time, winner, rating_change) VALUES (%u, %u, %u, \"%s\", \"%s\", %u, %u, %u, %u)", m_ArenaType, _arenaTeamIds[TEAM_ALLIANCE], _arenaTeamIds[TEAM_HORDE], oss_team1Members.str().c_str(), oss_team2Members.str().c_str(), GetStartTimestamp(), time(NULL), winner_arena_team->GetId(), winner_change);
        }
        else
        {
            SetArenaTeamRatingChangeForTeam(ALLIANCE, 0);
            SetArenaTeamRatingChangeForTeam(HORDE, 0);
        }
    }

    if (!IsArena()) {
        if(m_score[GetTeamIndexByTeamId(ALLIANCE)] == m_score[GetTeamIndexByTeamId(HORDE)])
            almost_winning_team = 0;         //no real winner
        if(m_score[GetTeamIndexByTeamId(ALLIANCE)] > m_score[GetTeamIndexByTeamId(HORDE)])
            almost_winning_team = ALLIANCE;
    }

    WorldPacket pvpLogData;
    BuildPvPLogDataPacket(pvpLogData);

    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());

    for(auto & m_Player : m_Players)
    {
        Player *plr = ObjectAccessor::FindPlayer(m_Player.first);
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
        else
            //needed cause else in av some creatures will kill the players at the end
            plr->CombatStop();

        uint32 team = m_Player.second.Team;
        if(!team) 
            team = plr->GetTeam();

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
            RewardMark(plr, ITEM_WINNER_COUNT);
            UpdatePlayerScore(plr, SCORE_BONUS_HONOR, 20);
            RewardQuest(plr);
        }
        else if (winner !=0)
            RewardMark(plr, ITEM_LOSER_COUNT);
        else if(winner == 0)
        {
            if(sWorld->getConfig(CONFIG_PREMATURE_BG_REWARD))    // We're feeling generous, giving rewards to people who not earned them ;)
            {    //nested ifs for the win! its boring writing that, forgive me my unfunniness
                if (GetTypeID() == BATTLEGROUND_AV)             // Only 1 mark for alterac
                    RewardMark(plr, ITEM_LOSER_COUNT);
                else if(almost_winning_team == team)                    //player's team had more points
                    RewardMark(plr, ITEM_WINNER_COUNT);
                else
                    RewardMark(plr, ITEM_LOSER_COUNT);            // if scores were the same, each team gets 1 mark.
            }
        }

        plr->ResetAllPowers();
        plr->CombatStopWithPets(true);

        BlockMovement(plr);

        plr->SendDirectMessage(&pvpLogData);

        WorldPacket data;
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, plr->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetStartTime(), GetArenaType(), plr->GetBGTeam());
        plr->SendDirectMessage(&data);
    }

    for (ObjectGuid m_Spectator : m_Spectators)
    {
        Player *plr = ObjectAccessor::FindPlayer(m_Spectator);
        if(!plr)
            continue;

        BlockMovement(plr);

        plr->SendDirectMessage(&pvpLogData);

        WorldPacket data;
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, plr->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetStartTime(), GetArenaType(), plr->GetBGTeam());
        plr->SendDirectMessage(&data);
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

    if(Source)
    {
        WorldPacket data;
        ChatHandler::BuildChatPacket(data, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, Source->GetGUID(), ObjectGuid::Empty, winmsg, 0);
        SendPacketToAll(&data);
    }
}


void Battleground::SendMessageToAll(int32 entry, ChatMsg type, Player const* source)
{
    if (!entry)
        return;

    Trinity::BattlegroundChatBuilder bg_builder(type, entry, source);
    Trinity::LocalizedPacketDo<Trinity::BattlegroundChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

void Battleground::PSendMessageToAll(int32 entry, ChatMsg type, Player const* source, ...)
{
    if (!entry)
        return;

    va_list ap;
    va_start(ap, source);

    Trinity::BattlegroundChatBuilder bg_builder(type, entry, source, &ap);
    Trinity::LocalizedPacketDo<Trinity::BattlegroundChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);

    va_end(ap);
}

void Battleground::SendWarningToAll(int32 entry, ...)
{
    if (!entry)
        return;

    std::map<uint32, WorldPacket> localizedPackets;
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player* p = ObjectAccessor::FindConnectedPlayer(itr->first);
        if (!p)
            continue;

        if (localizedPackets.find(p->GetSession()->GetSessionDbLocaleIndex()) == localizedPackets.end())
        {
            char const* format = sObjectMgr->GetTrinityString(entry, p->GetSession()->GetSessionDbLocaleIndex());

            char str[1024];
            va_list ap;
            va_start(ap, entry);
            vsnprintf(str, 1024, format, ap);
            va_end(ap);

            ChatHandler::BuildChatPacket(localizedPackets[p->GetSession()->GetSessionDbLocaleIndex()], CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, nullptr, nullptr, str);
        }

        p->SendDirectMessage(&localizedPackets[p->GetSession()->GetSessionDbLocaleIndex()]);
    }
}

void Battleground::SendMessage2ToAll(int32 entry, ChatMsg type, Player const* source, int32 arg1, int32 arg2)
{
    Trinity::Battleground2ChatBuilder bg_builder(type, entry, source, arg1, arg2);
    Trinity::LocalizedPacketDo<Trinity::Battleground2ChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
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

void Battleground::SetStatus(BattlegroundStatus Status)
{ 
    m_Status = Status; 

    if (m_Status == STATUS_WAIT_JOIN && !IsArena() && sWorld->getConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_ENABLE))
    {
        if (sWorld->getConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_WORLDONLY))
        {
            std::ostringstream msg;
            msg << "|cffffff00[BG Announcer] " << GetName() << "|r [" << GetMaxLevel() << "] " << "started.";
            ChatHandler::SendMessageWithoutAuthor("world", msg.str().c_str());
        } else
            sWorld->SendWorldText(LANG_BG_STARTED_ANNOUNCE_WORLD, GetName().c_str(), /*GetMinLevel(),*/ GetMaxLevel()); //Min level system is wrong and not complete
    }
}

void Battleground::RewardMark(Player* plr, uint32 count)
{
    // 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    if(plr->GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
        return;

    if(!plr || !count)
        return;
   
    // Give less marks if the player has been disconnected during the battleground
    auto itr = m_Players.find(plr->GetGUID());
    if (itr != m_Players.end())
    {
        float offlineRatio = itr->second.TotalOfflineTime / float(m_StartTime);
        if (count == 3) { // Winner
            if (offlineRatio <= 0.33f)
                count = 3;
            else if (offlineRatio <= 0.66f)
                count = 2;
            else
                count = 1;
        }
        else if (count == 1) { // Loser
            if (offlineRatio <= 0.5f)
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
            if(Item* item = plr->StoreNewItem( dest, mark, true, 0))
                plr->SendNewItem(item,count,false,true);

        if(no_space_count > 0)
            SendRewardMarkByMail(plr,mark,no_space_count);
    }
}

void Battleground::SendRewardMarkByMail(Player* plr, uint32 mark, uint32 count)
{
    uint32 bmEntry = GetBattlemasterEntry();
    if(!bmEntry)
        return;

    ItemTemplate const* markProto = sObjectMgr->GetItemTemplate(mark);
    if(!markProto)
        return;

    if(Item* markItem = Item::CreateItem(mark,count,plr))
    {
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        // save new item before send
        markItem->SaveToDB(trans);                               // save for prevent lost at next mail load, if send fail then item will deleted

        // subject: item name
        std::string subject = plr->GetSession()->GetLocalizedItemName(markProto);
        if(subject.empty())
            subject = "-";

        // text
        std::string textFormat = plr->GetSession()->GetTrinityString(LANG_BG_MARK_BY_MAIL);
        char textBuf[300];
        snprintf(textBuf,300,textFormat.c_str(),GetName().c_str(),GetName().c_str());

        MailDraft(subject, textBuf)
            .AddItem(markItem)
            .SendMailTo(trans, MailReceiver(plr, plr->GetGUID().GetCounter()), bmEntry, MAIL_CHECK_MASK_HAS_BODY);

        CharacterDatabase.CommitTransaction(trans);
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

void Battleground::RemovePlayerAtLeave(ObjectGuid guid, bool Transport, bool SendPacket)
{
    if (isSpectator(guid))
    {
        if (Player* player = ObjectAccessor::FindPlayer(guid))
        {
            player->CancelSpectate();

            if (player->TeleportToBGEntryPoint())
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
    auto itr = m_Players.find(guid);
    if(itr != m_Players.end())
    {
        UpdatePlayersCountByTeam(team, true);   // -1 player
        m_Players.erase(itr);
        // check if the player was a participant of the match, or only entered through gm command (goname)
        participant = true;
    }

    auto itr2 = PlayerScores.find(guid);
    if(itr2 != PlayerScores.end())
    {
        if (team == ALLIANCE) {
            auto itr3 = m_team1LogInfo.find(itr2->first);
            if (itr3 != m_team1LogInfo.end()) {
                itr3->second->damage = itr2->second->DamageDone;
                itr3->second->heal = itr2->second->HealingDone;
                itr3->second->kills = itr2->second->KillingBlows;
            }
        }
        else {
            auto itr3 = m_team2LogInfo.find(itr2->first);
            if (itr3 != m_team2LogInfo.end()) {
                itr3->second->damage = itr2->second->DamageDone;
                itr3->second->heal = itr2->second->HealingDone;
                itr3->second->kills = itr2->second->KillingBlows;
            }
        }

        delete itr2->second;                                // delete player's score
        PlayerScores.erase(itr2);
    }

    RemovePlayerFromResurrectQueue(guid);

    Player *plr = ObjectAccessor::FindPlayer(guid);
    if (plr)
    {
        // should remove spirit of redemption
        if (plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            plr->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

        plr->RemoveAurasByType(SPELL_AURA_MOUNTED);

        if (!plr->IsAlive())                              // resurrect on exit
        {
            plr->ResurrectPlayer(1.0f);
            plr->SpawnCorpseBones();
        }
    }
    else
    {
        SQLTransaction trans(nullptr);
        Player::OfflineResurrect(guid, trans);
    }

    RemovePlayer(plr, guid);                                // BG subclass specific code

    if (participant) // if the player was a match participant, remove auras, calc rating, update queue
    {
        BattlegroundTypeId bgTypeId = GetTypeID();
        BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());
        if (plr)
        {
            plr->ClearAfkReports();

            if(!team) 
                team = plr->GetTeam();

            // if arena, remove the specific arena auras
            if(IsArena())
            {
                plr->RemoveArenaAuras(true);    // removes debuffs / dots etc., we don't want the player to die after porting out
                bgTypeId = BATTLEGROUND_AA;       // set the bg type to all arenas (it will be used for queue refreshing)

                // summon old pet if there was one and there isn't a current pet
                if(!plr->GetPet() && plr->GetTemporaryUnsummonedPetNumber())
                {
                    auto NewPet = new Pet(plr);
                    if(!NewPet->LoadPetFromDB(plr, 0, plr->GetTemporaryUnsummonedPetNumber(), true))
                        delete NewPet;

                    (plr)->SetTemporaryUnsummonedPetNumber(0);
                }

                if(isRated() && GetStatus() == STATUS_IN_PROGRESS)
                {
                    //left a rated match while the encounter was in progress, consider as loser
                    ArenaTeam * winner_arena_team = nullptr;
                    ArenaTeam * loser_arena_team = nullptr;
                    if(team == HORDE)
                    {
                        winner_arena_team = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(ALLIANCE));
                        loser_arena_team = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(HORDE));
                    }
                    else
                    {
                        winner_arena_team = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(HORDE));
                        loser_arena_team = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(ALLIANCE));
                    }
                    if(winner_arena_team && loser_arena_team)
                    {
                        loser_arena_team->MemberLost(plr,winner_arena_team->GetRating());
                    }
                }
            }
            else        // BG: simply remove debuffs
                plr->RemoveArenaAuras(true);
            

            if(SendPacket)
            {
                WorldPacket data;
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, plr->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_NONE, 0, 0, 0, 0);
                plr->SendDirectMessage(&data);
            }

            // this call is important, because player, when joins to battleground, this method is not called, so it must be called when leaving bg
            plr->RemoveBattlegroundQueueId(bgQueueTypeId);
        }

        // remove from raid group if player is member
        if (Group* group = GetBgRaid(team))
        {
            if (!group->RemoveMember(guid))               // group was disbanded
                SetBgRaid(team, nullptr);
        }
        DecreaseInvitedCount(team);

        //we should update battleground queue, but only if bg isn't ending
        if (isBattleground() && GetStatus() < STATUS_WAIT_LEAVE)
        {
            // a player has left the battleground, so there are free slots -> add to queue
            AddToBGFreeSlotQueue();
            sBattlegroundMgr->ScheduleQueueUpdate(0, 0, bgQueueTypeId, bgTypeId, GetBracketId());
        }
        // Let others know
        WorldPacket data;
        sBattlegroundMgr->BuildPlayerLeftBattlegroundPacket(&data, guid);
        SendPacketToTeam(team, &data, plr, false);

        TC_LOG_DEBUG("battleground","BATTLEGROUND: Removed player %s from Battleground.", plr->GetName().c_str());
    }

    if(!GetPlayersSize() && !GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE) && !m_Spectators.size())
    {
        // if no players left AND no invitees left AND no spectators left, set this bg to delete in next update
        // direct deletion could cause crashes
        m_SetDeleteThis = true;

        // return to prevent addition to freeslotqueue
        return;
    }

    if (plr)
    {
        // Do next only if found in battleground
        plr->SetBattlegroundId(0, BATTLEGROUND_TYPE_NONE);  // We're not in BG.
        // reset destination bg team
        plr->SetBGTeam(0);

        // remove all criterias on bg leave
        //LK plr->ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);

        if (Transport)
            plr->TeleportToBGEntryPoint();

        TC_LOG_DEBUG("bg.battleground", "Removed player %s from Battleground.", plr->GetName().c_str());
    }
}

// this method is called when no players remains in battleground
void Battleground::Reset()
{
    SetStartTime(0);
    SetWinner(WINNER_NONE);
    SetStatus(STATUS_WAIT_QUEUE);
    SetEndTime(0);
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
    PlayerScores.clear();

    // reset BGSubclass
    ResetBGSubclass();
}

void Battleground::StartBattleground()
{
    SetStartTime(0);
    SetLastResurrectTime(0);
    // add BG to free slot queue
    AddToBGFreeSlotQueue();

    // add bg to update list
    // This must be done here, because we need to have already invited some players when first BG::Update() method is executed
    // and it doesn't matter if we call StartBattleground() more times, because m_Battlegrounds is a map and instance id never changes
    sBattlegroundMgr->AddBattleground(this);

    if(m_IsRated) 
        TC_LOG_DEBUG("arena","Arena match type: %u for Team1Id: %u - Team2Id: %u started.", m_ArenaType, _arenaTeamIds[TEAM_ALLIANCE], _arenaTeamIds[TEAM_HORDE]);
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

    ObjectGuid guid = plr->GetGUID();
    uint32 team = plr->GetBGTeam();

    BattlegroundPlayer bp;
    bp.OfflineRemoveTime = 0;
    bp.TotalOfflineTime = 0;
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
        plr->RemoveArenaAuras(false);
        plr->RemoveAllEnchantments(TEMP_ENCHANTMENT_SLOT, true);
        if(team == ALLIANCE)                                // gold
        {
            if(plr->GetTeam() == HORDE)
                plr->CastSpell(plr, SPELL_HORDE_GOLD_FLAG, true);
            else
                plr->CastSpell(plr, SPELL_ALLIANCE_GOLD_FLAG, true);
        }
        else                                                // green
        {
            if(plr->GetTeam() == HORDE)
                plr->CastSpell(plr, SPELL_HORDE_GREEN_FLAG, true);
            else
                plr->CastSpell(plr, SPELL_ALLIANCE_GREEN_FLAG, true);
        }

        plr->DestroyConjuredItems(true);
        plr->UnsummonPetTemporaryIfAny();

        if(GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
        {
            plr->CastSpell(plr, SPELL_ARENA_PREPARATION, TRIGGERED_FULL_MASK);
            plr->ResetAllPowers();
        }
    }
    else
    {
        if(GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
            plr->CastSpell(plr, SPELL_PREPARATION, true);   // reduces all mana cost of spells.
    }

    // setup BG group membership
    PlayerAddedToBGCheckIfBGIsRunning(plr);
    AddOrSetPlayerToCorrectBgGroup(plr, team);

    // Log
    TC_LOG_DEBUG("battleground","BATTLEGROUND: Player %s joined the battle.", plr->GetName().c_str());
}

void Battleground::SetStartDelayTime(int Time)
{
    if (   (IsArena() && sBattlegroundMgr->IsArenaTesting())
        || (isBattleground() && sBattlegroundMgr->IsBattleGroundTesting()) )
        m_StartDelayTime = 1000;
    else
        m_StartDelayTime = Time;
}

/* This method should be called only once ... it adds pointer to queue */
void Battleground::AddToBGFreeSlotQueue()
{
    // make sure to add only once
    if (!m_InBGFreeSlotQueue && isBattleground())
    {
        sBattlegroundMgr->AddToBGFreeSlotQueue(m_TypeID, this);
        m_InBGFreeSlotQueue = true;
    }
}

/* This method removes this battleground from free queue - it must be called when deleting battleground - not used now*/
void Battleground::RemoveFromBGFreeSlotQueue()
{
    if (m_InBGFreeSlotQueue)
    {
        sBattlegroundMgr->RemoveFromBGFreeSlotQueue(m_TypeID, m_InstanceID);
        m_InBGFreeSlotQueue = false;
    }
}


// get the number of free slots for team
// returns the number how many players can join battleground to MaxPlayersPerTeam
uint32 Battleground::GetFreeSlotsForTeam(uint32 Team) const
{
    // if BG is starting and CONFIG_BATTLEGROUND_INVITATION_TYPE == BG_QUEUE_INVITATION_TYPE_NO_BALANCE, invite anyone
    if (GetStatus() == STATUS_WAIT_JOIN && sWorld->getIntConfig(CONFIG_BATTLEGROUND_INVITATION_TYPE) == BG_QUEUE_INVITATION_TYPE_NO_BALANCE)
        return (GetInvitedCount(Team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) : 0;

    // if BG is already started or CONFIG_BATTLEGROUND_INVITATION_TYPE != BG_QUEUE_INVITATION_TYPE_NO_BALANCE, do not allow to join too much players of one faction
    uint32 otherTeamInvitedCount;
    uint32 thisTeamInvitedCount;
    uint32 otherTeamPlayersCount;
    uint32 thisTeamPlayersCount;

    if (Team == ALLIANCE)
    {
        thisTeamInvitedCount = GetInvitedCount(ALLIANCE);
        otherTeamInvitedCount = GetInvitedCount(HORDE);
        thisTeamPlayersCount = GetPlayersCountByTeam(ALLIANCE);
        otherTeamPlayersCount = GetPlayersCountByTeam(HORDE);
    }
    else
    {
        thisTeamInvitedCount = GetInvitedCount(HORDE);
        otherTeamInvitedCount = GetInvitedCount(ALLIANCE);
        thisTeamPlayersCount = GetPlayersCountByTeam(HORDE);
        otherTeamPlayersCount = GetPlayersCountByTeam(ALLIANCE);
    }
    if (GetStatus() == STATUS_IN_PROGRESS || GetStatus() == STATUS_WAIT_JOIN)
    {
        // difference based on ppl invited (not necessarily entered battle)
        // default: allow 0
        uint32 diff = 0;

        // allow join one person if the sides are equal (to fill up bg to minPlayerPerTeam)
        if (otherTeamInvitedCount == thisTeamInvitedCount)
            diff = 1;
        // allow join more ppl if the other side has more players
        else if (otherTeamInvitedCount > thisTeamInvitedCount)
            diff = otherTeamInvitedCount - thisTeamInvitedCount;

        // difference based on max players per team (don't allow inviting more)
        uint32 diff2 = (thisTeamInvitedCount < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - thisTeamInvitedCount : 0;

        // difference based on players who already entered
        // default: allow 0
        uint32 diff3 = 0;
        // allow join one person if the sides are equal (to fill up bg minPlayerPerTeam)
        if (otherTeamPlayersCount == thisTeamPlayersCount)
            diff3 = 1;
        // allow join more ppl if the other side has more players
        else if (otherTeamPlayersCount > thisTeamPlayersCount)
            diff3 = otherTeamPlayersCount - thisTeamPlayersCount;
        // or other side has less than minPlayersPerTeam
        else if (thisTeamInvitedCount <= GetMinPlayersPerTeam())
            diff3 = GetMinPlayersPerTeam() - thisTeamInvitedCount + 1;

        // return the minimum of the 3 differences

        // min of diff and diff 2
        diff = std::min(diff, diff2);
        // min of diff, diff2 and diff3
        return std::min(diff, diff3);
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
    auto itr = PlayerScores.find(Source->GetGUID());

    if(itr == PlayerScores.end())                         // player not found...
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
                if(Source->RewardHonor(nullptr, 1, value))
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

void Battleground::AddPlayerToResurrectQueue(ObjectGuid npc_guid, ObjectGuid player_guid)
{
    m_ReviveQueue[npc_guid].push_back(player_guid);

    Player *plr = ObjectAccessor::FindPlayer(player_guid);
    if(!plr)
        return;

    plr->CastSpell(plr, SPELL_WAITING_FOR_RESURRECT, true);
}

void Battleground::RemovePlayerFromResurrectQueue(ObjectGuid player_guid)
{
    for(auto & itr : m_ReviveQueue)
    {
        for(auto itr2 =(itr.second).begin(); itr2 != (itr.second).end(); ++itr2)
        {
            if(*itr2 == player_guid)
            {
                (itr.second).erase(itr2);

                Player *plr = ObjectAccessor::FindPlayer(player_guid);
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
    Map* map = FindBgMap();
    if (!map)
        return false;

    // must be created this way, adding to godatamap would add it to the base map of the instance
    // and when loading it (in go::LoadFromDB()), a new guid would be assigned to the object, and a new object would be created
    // so we must create it specific for this instance
    GameObject* go = sObjectMgr->CreateGameObject(entry);
    if(!go->Create(GetBgMap()->GenerateLowGuid<HighGuid::GameObject>(), entry, GetBgMap(), PHASEMASK_NORMAL, Position(x,y,z,o), G3D::Quat(rotation0,rotation1,rotation2,rotation3), 255, GO_STATE_READY))
    {
        TC_LOG_ERROR("battleground","Gameobject template %u not found in database! Battleground not created!", entry);
        TC_LOG_ERROR("battleground","Cannot create gameobject template %u! Battleground not created!", entry);
        delete go;
        return false;
    }

    if(inactive)
        go->SetInactive(true);

    map->AddToMap(go); // add to world, so it can be later looked up from HashMapHolder in SpawnBGObject

    BgObjects[type] = go->GetGUID();
    return true;
}

//some doors aren't despawned so we cannot handle their closing in gameobject::update()
//it would be nice to correctly implement GO_ACTIVATED state and open/close doors in gameobject code
void Battleground::DoorClose(uint32 type)
{
    if(GameObject *obj = GetBgMap()->GetGameObject(BgObjects[type]))
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
        TC_LOG_ERROR("bg.battleground", "Battleground::DoorClose: door gameobject (type: %u, %s) not found for BG (map: %u, instance id: %u)!",
            type, ObjectGuid(BgObjects[type]).ToString().c_str(), m_MapId, m_InstanceID);
}

void Battleground::DoorOpen(uint32 type)
{
    if(GameObject* obj = GetBgMap()->GetGameObject(BgObjects[type]))
    {
        //change state to be sure they will be opened
        obj->SetLootState(GO_READY);
        obj->UseDoorOrButton(RESPAWN_ONE_DAY);
    }
    else
        TC_LOG_ERROR("bg.battleground", "Battleground::DoorOpen: door gameobject (type: %u, %s) not found for BG (map: %u, instance id: %u)!",
            type, ObjectGuid(BgObjects[type]).ToString().c_str(), m_MapId, m_InstanceID);
}

GameObject* Battleground::GetBGObject(uint32 type, bool logError)
{
    GameObject* obj = GetBgMap()->GetGameObject(BgObjects[type]);
    if (!obj)
    {
        if (logError)
            TC_LOG_ERROR("bg.battleground", "Battleground::GetBGObject: gameobject (type: %u, %s) not found for BG (map: %u, instance id: %u)!",
                type, ObjectGuid(BgObjects[type]).ToString().c_str(), m_MapId, m_InstanceID);
        else
            TC_LOG_INFO("bg.battleground", "Battleground::GetBGObject: gameobject (type: %u, %s) not found for BG (map: %u, instance id: %u)!",
                type, ObjectGuid(BgObjects[type]).ToString().c_str(), m_MapId, m_InstanceID);
    }
    return obj;
}

Creature* Battleground::GetBGCreature(uint32 type, bool logError)
{
    Creature* creature = GetBgMap()->GetCreature(BgCreatures[type]);
    if (!creature)
    {
        if (logError)
            TC_LOG_ERROR("bg.battleground", "Battleground::GetBGCreature: creature (type: %u, %s) not found for BG (map: %u, instance id: %u)!",
                type, ObjectGuid(BgCreatures[type]).ToString().c_str(), m_MapId, m_InstanceID);
        else
            TC_LOG_INFO("bg.battleground", "Battleground::GetBGCreature: creature (type: %u, %s) not found for BG (map: %u, instance id: %u)!",
                type, ObjectGuid(BgCreatures[type]).ToString().c_str(), m_MapId, m_InstanceID);
    }
    return creature;
}

void Battleground::SpawnBGObject(uint32 type, uint32 respawntime)
{
    if (Map* map = FindBgMap())
        if (GameObject* obj = map->GetGameObject(BgObjects[type]))
        {
            if (respawntime)
                obj->SetLootState(GO_JUST_DEACTIVATED);
            else
            {
                if (obj->getLootState() == GO_JUST_DEACTIVATED)
                    // Change state from GO_JUST_DEACTIVATED to GO_READY in case battleground is starting again
                    obj->SetLootState(GO_READY);
                if (obj->IsInactive())
                    obj->SetInactive(false);
            }
            obj->SetRespawnTime(respawntime);
            
            if (!obj->IsInWorld()) //sunstrider: TC code tries to add it to map a second time in some cases
                map->AddToMap(obj);
            else if(!respawntime) //Immediately recreate object at client (else player would have to move for the object to appear)
                obj->UpdateObjectVisibility(true);
        }
}

Creature* Battleground::AddCreature(uint32 entry, uint32 type, float x, float y, float z, float o, uint32 respawntime)
{
    // If the assert is called, means that BgCreatures must be resized!
    ASSERT(type < BgCreatures.size());

    Map* map = FindBgMap();
    if (!map)
        return nullptr;

    auto  pCreature = new Creature();
    if (!pCreature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, PHASEMASK_NORMAL, entry, { x, y, z, o }))
    {
        TC_LOG_ERROR("battleground","Can't create creature entry: %u",entry);
        delete pCreature;
        return nullptr;
    }

    pCreature->Relocate(x, y, z, o);

    if(!pCreature->IsPositionValid())
    {
        TC_LOG_ERROR("battleground","ERROR: Creature (guidlow %d, entry %d) not added to battleground. Suggested coordinates isn't valid (X: %f Y: %f)",pCreature->GetGUID().GetCounter(),pCreature->GetEntry(),pCreature->GetPositionX(),pCreature->GetPositionY());
        delete pCreature;
        return nullptr;
    }

    pCreature->SetHomePosition(x, y, z, o);
    pCreature->AIM_Initialize();

    if (!map->AddToMap(pCreature))
    {
        delete pCreature;
        return nullptr;
    }

    BgCreatures[type] = pCreature->GetGUID();

    if (respawntime)
        pCreature->SetRespawnDelay(respawntime);

    return pCreature;
}

bool Battleground::DelCreature(uint32 type)
{
    if(!BgCreatures[type])
        return true;

    Creature *cr = GetBgMap()->GetCreature(BgCreatures[type]);
    if(!cr)
    {
        TC_LOG_ERROR("bg.battleground", "Battleground::DelCreature: creature (type: %u, %s) not found for BG (map: %u, instance id: %u)!",
            type, ObjectGuid(BgCreatures[type]).ToString().c_str(), m_MapId, m_InstanceID);
        return false;
    }

    cr->AI()->EnterEvadeMode();
    cr->AddObjectToRemoveList();
    BgCreatures[type].Clear();
    return true;
}

bool Battleground::DelObject(uint32 type)
{
    if(!BgObjects[type])
        return true;

    GameObject *obj = GetBgMap()->GetGameObject(BgObjects[type]);
    if(!obj)
    {
        TC_LOG_ERROR("bg.battleground", "Battleground::DelObject: gameobject (type: %u, %s) not found for BG (map: %u, instance id: %u)!",
            type, ObjectGuid(BgObjects[type]).ToString().c_str(), m_MapId, m_InstanceID);
        return false;
    }
    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete();
    BgObjects[type].Clear();
    return true;
}

bool Battleground::AddSpiritGuide(uint32 type, float x, float y, float z, float o, uint32 team)
{
    uint32 entry = 0;

    if(team == ALLIANCE)
        entry = 13116;
    else
        entry = 13117;

    Creature* pCreature = AddCreature(entry,type,x,y,z,o);
    if(!pCreature)
    {
        TC_LOG_ERROR("bg.battleground","Can't create Spirit guide. Battleground not created!");
        EndNow();
        return false;
    }

    //pCreature->SetDeathState(DEAD);

    pCreature->SetChannelObjectGuid(pCreature->GetGUID());
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

void Battleground::PlayerAddedToBGCheckIfBGIsRunning(Player* player)
{
    if (GetStatus() != STATUS_WAIT_LEAVE)
        return;

    WorldPacket data;
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());

    BlockMovement(player);

    BuildPvPLogDataPacket(data);
    player->SendDirectMessage(&data);

    sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, player->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, GetEndTime(), GetStartTime(), GetArenaType(), player->GetBGTeam());
    player->SendDirectMessage(&data);
}

void Battleground::EndNow()
{
    RemoveFromBGFreeSlotQueue();
    SetStatus(STATUS_WAIT_LEAVE);
    SetEndTime(0);
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
void Battleground::HandleTriggerBuff(ObjectGuid const& go_guid)
{
    GameObject *obj = GetBgMap()->GetGameObject(go_guid);
    if(!obj || obj->GetGoType() != GAMEOBJECT_TYPE_TRAP || !obj->isSpawned())
        return;

    //change buff type, when buff is used:
    int32 index = BgObjects.size() - 1;
    while (index >= 0 && BgObjects[index] != go_guid)
        index--;
    if (index < 0)
    {
        TC_LOG_ERROR("battleground","Battleground (Type: %u) has buff gameobject (Guid: %u Entry: %u Type:%u) but it hasn't that object in its internal data",GetTypeID(),go_guid.GetCounter(),obj->GetEntry(),obj->GetGoType());
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

void Battleground::HandleKillPlayer( Player* player, Player* killer )
{
    //keep in mind that for arena this will have to be changed a bit

    // add +1 deaths
    UpdatePlayerScore(player, SCORE_DEATHS, 1);

    // add +1 kills to group and +1 killing_blows to killer
    if( killer && killer != player) // a player may kill itself in some cases!
    {
        UpdatePlayerScore(killer, SCORE_HONORABLE_KILLS, 1);
        UpdatePlayerScore(killer, SCORE_KILLING_BLOWS, 1);

        for(auto & m_Player : m_Players)
        {
            Player *plr = ObjectAccessor::FindPlayer(m_Player.first);

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
uint32 Battleground::GetPlayerTeam(ObjectGuid guid)
{
    std::map<ObjectGuid, BattlegroundPlayer>::const_iterator itr = m_Players.find(guid);
    if(itr != m_Players.end())
        return itr->second.Team;
    return 0;
}

bool Battleground::IsPlayerInBattleground(ObjectGuid guid)
{
    std::map<ObjectGuid, BattlegroundPlayer>::const_iterator itr = m_Players.find(guid);
    if(itr != m_Players.end())
        return true;
    return false;
}

uint32 Battleground::GetAlivePlayersCountByTeam(uint32 Team) const
{
    int count = 0;
    for(const auto & m_Player : m_Players)
    {
        if(m_Player.second.Team == Team)
        {
            Player * pl = ObjectAccessor::FindPlayer(m_Player.first);
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

int32 Battleground::GetObjectType(ObjectGuid const& guid)
{
    for(uint32 i = 0;i < BgObjects.size(); i++)
        if(BgObjects[i] == guid)
            return i;
    TC_LOG_ERROR("bg.battleground","Battleground: cheating? a player used a gameobject which isnt supposed to be a usable object!");
    return -1;
}

void Battleground::HandleKillUnit(Creature *creature, Player *killer)
{
}

// this method adds player to his team's bg group, or sets his correct group if player is already in bg group
void Battleground::AddOrSetPlayerToCorrectBgGroup(Player* player, uint32 team)
{
    ObjectGuid playerGuid = player->GetGUID();
    Group* group = GetBgRaid(team);
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    if (!group)                                      // first player joined
    {
        group = new Group;
        SetBgRaid(team, group);
        group->Create(player, trans);
    }
    else                                            // raid already exist
    {
        if (group->IsMember(playerGuid))
        {
            uint8 subgroup = group->GetMemberGroup(playerGuid);
#ifdef LICH_KING
            player->SetBattlegroundOrBattlefieldRaid(group, subgroup);
#else
            player->SetBattlegroundRaid(group, subgroup);
#endif
        }
        else
        {
            group->AddMember(player, trans);
            if (Group* originalGroup = player->GetOriginalGroup())
                if (originalGroup->IsLeader(playerGuid))
                {
                    group->ChangeLeader(playerGuid);
                    group->SendUpdate();
                }
        }
    }
    CharacterDatabase.CommitTransaction(trans);
}

// This method should be called when player logs into running battleground
void Battleground::EventPlayerLoggedIn(Player* player)
{
    ObjectGuid guid = player->GetGUID();
    // player is correct pointer
    for (GuidDeque::iterator itr = m_OfflineQueue.begin(); itr != m_OfflineQueue.end(); ++itr)
    {
        if (*itr == guid)
        {
            m_OfflineQueue.erase(itr);
            break;
        }
    }
    m_Players[guid].OfflineRemoveTime = 0;
    PlayerAddedToBGCheckIfBGIsRunning(player);
    // if battleground is starting, then add preparation aura
    // we don't have to do that, because preparation aura isn't removed when player logs out

}
// This method should be called when player logs out from running battleground
void Battleground::EventPlayerLoggedOut(Player* player)
{
    ObjectGuid guid = player->GetGUID();
    if (!IsPlayerInBattleground(guid))  // Check if this player really is in battleground (might be a GM who teleported inside)
        return;

    // player is correct pointer, it is checked in WorldSession::LogoutPlayer()
    m_OfflineQueue.push_back(player->GetGUID());

    m_Players[guid].OfflineRemoveTime = WorldGameTime::GetGameTime() + MAX_OFFLINE_TIME;

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

        if (player->TeleportToBGEntryPoint())
        {
            player->SetSpectate(false);
            RemoveSpectator(player->GetGUID());
        }
    }
}

void Battleground::PlayerInvitedInRatedArena(Player* player, uint32 team)
{
    auto  logInfo = new PlayerLogInfo;
    logInfo->guid = player->GetGUID().GetCounter();
    logInfo->ip = player->GetSession()->GetRemoteAddress();
    logInfo->heal = 0;
    logInfo->damage = 0;
    logInfo->kills = 0;

    if (team == ALLIANCE)
        m_team1LogInfo[player->GetGUID()] = logInfo;
    else
        m_team2LogInfo[player->GetGUID()] = logInfo;
}

void Battleground::SendSpectateAddonsMsg(SpectatorAddonMsg msg)
{
    if (!HaveSpectators())
        return;

    for (ObjectGuid m_Spectator : m_Spectators)
        msg.SendPacket(m_Spectator);
}

bool Battleground::isSpectator(ObjectGuid guid)
{
    for(ObjectGuid m_Spectator : m_Spectators)
    {
        if (guid == m_Spectator)
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

Group* Battleground::GetBgRaid(uint32 TeamID) const
{ 
    return TeamID == ALLIANCE ? m_BgRaids[TEAM_ALLIANCE] : m_BgRaids[TEAM_HORDE]; 
}

void Battleground::SetBgRaid(uint32 TeamID, Group *bg_raid)
{
    Group* &old_raid = TeamID == ALLIANCE ? m_BgRaids[TEAM_ALLIANCE] : m_BgRaids[TEAM_HORDE];
    if (old_raid)
        old_raid->SetBattlegroundGroup(nullptr);
    if (bg_raid)
        bg_raid->SetBattlegroundGroup(this);
    old_raid = bg_raid;
}

void Battleground::SetBracket(PvPDifficultyEntry const* bracketEntry)
{
    m_BracketId = bracketEntry->GetBracketId();
    SetLevelRange(bracketEntry->minLevel, bracketEntry->maxLevel);
}

inline void Battleground::_CheckSafePositions(uint32 diff)
{
    float maxDist = GetStartMaxDist();
    if (!maxDist)
        return;

    m_ValidStartPositionTimer += diff;
    if (m_ValidStartPositionTimer >= CHECK_PLAYER_POSITION_INVERVAL)
    {
        m_ValidStartPositionTimer = 0;

        for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
        {
            if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            {
                if (player->IsGameMaster())
                    continue;

                Position pos = player->GetPosition();
                Position const* startPos = GetTeamStartPosition(Battleground::GetTeamIndexByTeamId(player->GetBGTeam()));
                if (pos.GetExactDistSq(startPos) > maxDist)
                {
                    TC_LOG_DEBUG("bg.battleground", "BATTLEGROUND: Sending %s back to start location (map: %u) (possible exploit)", player->GetName().c_str(), GetMapId());
                    player->TeleportTo(GetMapId(), startPos->GetPositionX(), startPos->GetPositionY(), startPos->GetPositionZ(), startPos->GetOrientation());
                }
            }
        }
    }
}

inline void Battleground::_ProcessOfflineQueue(uint32 diff)
{
    //update offline timer
    for (auto& itr : m_OfflineQueue)
    {
        BattlegroundPlayerMap::iterator itr2 = m_Players.find(itr);
        if (itr2 != m_Players.end())
            itr2->second.TotalOfflineTime += diff;
    }

    // remove offline players from bg after 5 minutes
    if (!m_OfflineQueue.empty())
    {
        BattlegroundPlayerMap::iterator itr = m_Players.find(*(m_OfflineQueue.begin()));
        if (itr != m_Players.end())
        {
            if (itr->second.OfflineRemoveTime <= WorldGameTime::GetGameTime()) //timer expired, kick them
            {
                if (isBattleground() /*&& sWorld->getBoolConfig(CONFIG_BATTLEGROUND_TRACK_DESERTERS) */ &&
                    (GetStatus() == STATUS_IN_PROGRESS || GetStatus() == STATUS_WAIT_JOIN))
                {
                    CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '8' WHERE guid = %u", itr->first.GetCounter()); // AT_LOGIN_SET_DESERTER
                    /*PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_DESERTER_TRACK);
                    stmt->setUInt32(0, itr->first.GetCounter());
                    stmt->setUInt8(1, BG_DESERTION_TYPE_OFFLINE);
                    CharacterDatabase.Execute(stmt);*/
                }

                RemovePlayerAtLeave(itr->first, true, true);// remove player from BG
                m_OfflineQueue.pop_front();                 // remove from offline queue
                                                            //do not use itr for anything, because it is erased in RemovePlayerAtLeave()
            }
        }
    }
}

uint8 Battleground::GetUniqueBracketId() const
{
    return GetMinLevel() / 10;
}

void Battleground::BuildPvPLogDataPacket(WorldPacket& data)
{
    uint32 plScSize = GetPlayerScoresSize();
    data.Initialize(MSG_PVP_LOG_DATA, (1+1+4+4*plScSize));
    
    uint8 type = (IsArena() ? 1 : 0);
    data << uint8(type); // battleground = 0 / arena = 1

    if (type) { // arena
        /* TC LK / TODO LK
        for (uint8 i = 0; i < BG_TEAMS_COUNT; ++i)
            _arenaTeamScores[i].BuildRatingInfoBlock(data);

        for (uint8 i = 0; i < BG_TEAMS_COUNT; ++i)
            _arenaTeamScores[i].BuildTeamInfoBlock(data);
            */

        // it seems this must be according to BG_WINNER_A/H and _NOT_ BG_TEAM_A/H
        for (int i = 1; i >= 0; --i) {
            data << uint32(3000 - m_ArenaTeamRatingChanges[i]); // rating change: showed value - 3000
            data << uint32(3999); // huge thanks to TOM_RUS for this!
        }
        
        for (int i = 1; i >= 0; --i) {
            uint32 teamId = _arenaTeamIds[i];
            ArenaTeam* at = sArenaTeamMgr->GetArenaTeamById(teamId);
            if (at)
                data << at->GetName();
            else
                data << uint8(0);
        }
    }

    uint8 winner = GetWinner();
    if (winner == 2)
        data << uint8(0); // bg in progress
    else {
        data << uint8(1); // bg ended
        data << uint8(winner); // who win
    }

    data << plScSize;

    for (auto itr = GetPlayerScoresBegin(); itr != GetPlayerScoresEnd(); ++itr) {
        data << uint64(itr->first); // GUID
        data << uint32(itr->second->KillingBlows);
        
        if (type) { // Arena
            Player* player = ObjectAccessor::FindConnectedPlayer(itr->first);
            uint32 team = GetPlayerTeam(itr->first);;
            
            if (player) {
                if (!team)
                    team = player->GetTeam();
                
                if (team == ALLIANCE)
                    data << uint8(1);
                else
                    data << uint8(0);
            } else
                data << uint8(0);
        } else { // Battleground
            data << uint32(itr->second->HonorableKills);
            data << uint32(itr->second->Deaths);
            data << uint32(itr->second->BonusHonor);
        }
        
        data << uint32(itr->second->DamageDone);
        data << uint32(itr->second->HealingDone);
        
        switch (GetTypeID()) // battleground specific things
        {
        case BATTLEGROUND_AV:
            data << uint32(5); // count of next fields
            data << uint32(((BattlegroundAVScore*) itr->second)->GraveyardsAssaulted);
            data << uint32(((BattlegroundAVScore*) itr->second)->GraveyardsDefended);
            data << uint32(((BattlegroundAVScore*) itr->second)->TowersAssaulted);
            data << uint32(((BattlegroundAVScore*) itr->second)->TowersDefended);
            data << uint32(((BattlegroundAVScore*) itr->second)->MinesCaptured);
            break;
        case BATTLEGROUND_WS:
            data << uint32(2); // count of next fields
            data << uint32(((BattlegroundWGScore*) itr->second)->FlagCaptures);
            data << uint32(((BattlegroundWGScore*) itr->second)->FlagReturns);
            break;
        case BATTLEGROUND_AB:
            data << uint32(2); // count of next fields
            data << uint32(((BattlegroundABScore*) itr->second)->BasesAssaulted);
            data << uint32(((BattlegroundABScore*) itr->second)->BasesDefended);
            break;
        case BATTLEGROUND_EY:
            data << uint32(1); // count of next fields
            data << uint32(((BattlegroundEYScore*) itr->second)->FlagCaptures);
            break;
        case BATTLEGROUND_NA:
        case BATTLEGROUND_BE:
        case BATTLEGROUND_AA:
        case BATTLEGROUND_RL:
            data << uint32(0); // count of next fields
            break;
        default:
            TC_LOG_ERROR("bg.battleground","Unhandled MSG_PVP_LOG_DATA for BG id %u", GetTypeID());
            data << uint32(0);
            break;
        }
    }
}