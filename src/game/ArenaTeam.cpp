/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
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

#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "ArenaTeam.h"
#include "Chat.h"
#include "World.h"

ArenaTeam::ArenaTeam()
{
    Id                     = 0;
    Type                   = 0;
    Name                   = "";
    CaptainGuid            = 0;
    BackgroundColor        = 0;                                // background
    EmblemStyle            = 0;                                // icon
    EmblemColor            = 0;                                // icon color
    BorderStyle            = 0;                                // border
    BorderColor            = 0;                                // border color
    stats.games_week       = 0;
    stats.games_season     = 0;
    stats.rank             = 0;
    stats.rating           = 1500;
    stats.wins_week        = 0;
    stats.wins_season      = 0;
    stats.non_played_weeks = 0;
}

ArenaTeam::~ArenaTeam()
{

}

bool ArenaTeam::Create(uint64 captainGuid, uint32 type, std::string ArenaTeamName)
{
    if(!sObjectMgr->GetPlayer(captainGuid))                      // player not exist
        return false;
    if(sObjectMgr->GetArenaTeamByName(ArenaTeamName))            // arena team with this name already exist
        return false;
    if(!sObjectMgr->IsValidCharterName(ArenaTeamName))            // Invalid name
        return false;

    CaptainGuid = captainGuid;
    Name = ArenaTeamName;
    Type = type;

    Id = sObjectMgr->GenerateArenaTeamId();

    // ArenaTeamName already assigned to ArenaTeam::name, use it to encode string for DB
    CharacterDatabase.EscapeString(ArenaTeamName);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    // CharacterDatabase.PExecute("DELETE FROM arena_team WHERE arenateamid='%u'", Id); - MAX(arenateam)+1 not exist
    //trans->PAppend("DELETE FROM arena_team_member WHERE arenateamid='%u'", Id);
    AddMember(CaptainGuid, trans);
    trans->PAppend("INSERT INTO arena_team (arenateamid,name,captainguid,type,BackgroundColor,EmblemStyle,EmblemColor,BorderStyle,BorderColor) "
        "VALUES('%u','%s','%u','%u','%u','%u','%u','%u','%u')",
        Id, ArenaTeamName.c_str(), GUID_LOPART(CaptainGuid), Type, BackgroundColor, EmblemStyle, EmblemColor, BorderStyle, BorderColor);
    trans->PAppend("INSERT INTO arena_team_stats (arenateamid, rating, games, wins, played, wins2, rank) VALUES "
        "('%u', '%u', '%u', '%u', '%u', '%u', '%u')", Id, stats.rating, stats.games_week, stats.wins_week, stats.games_season, stats.wins_season, stats.rank);

    CharacterDatabase.CommitTransaction(trans);
    TC_LOG_DEBUG("arena","New ArenaTeam created [Id: %u] [Type: %u] [Captain GUID: %u]", GetId(), GetType(), GetCaptain());
    
    std::string ip = "unknown";
    if (Player* captain = sObjectMgr->GetPlayer(GetCaptain()))
        ip = captain->GetSession()->GetRemoteAddress();
    LogsDatabase.PExecute("INSERT INTO arena_team_event (id, event, type, player, ip, time) VALUES (%u, %u, %u, %u, '%s', %u)",
            GetId(), uint32(AT_EV_CREATE), GetType(), GUID_LOPART(GetCaptain()), ip.c_str(), time(NULL));
    
    return true;
}

bool ArenaTeam::AddMember(const uint64& PlayerGuid, SQLTransaction trans)
{
    // arena team is full (can't have more than type * 2 players!)
    if(GetMembersSize() >= GetType() * 2)
        return false;

    Player *pl = sObjectMgr->GetPlayer(PlayerGuid);
    if(pl)
    {
        if(pl->GetArenaTeamId(GetSlot()))
        {
            TC_LOG_ERROR("FIXME","Arena::AddMember() : player already in this sized team");
            return false;
        }
    }
    else
    {
        // check if player already in arenateam of that size
        if(Player::GetArenaTeamIdFromDB(PlayerGuid, GetType()) != 0)
        {
            TC_LOG_ERROR("FIXME","Arena::AddMember() : player already in this sized team");
            return false;
        }
    }

    // remove all player signs from another petitions
    // this will be prevent attempt joining player to many arenateams and corrupt arena team data integrity
    Player::RemovePetitionsAndSigns(PlayerGuid, GetType(), trans);

    ArenaTeamMember newmember;
    newmember.guid              = PlayerGuid;
    newmember.games_season      = 0;
    newmember.games_week        = 0;
    newmember.wins_season       = 0;
    newmember.wins_week         = 0;
    newmember.personal_rating   = 1500;
    members.push_back(newmember);

    trans->PAppend("INSERT INTO arena_team_member (arenateamid, guid, personal_rating) VALUES ('%u', '%u', '%u')", Id, GUID_LOPART(newmember.guid), newmember.personal_rating );

    if(pl)
    {
        pl->SetInArenaTeam(Id, GetSlot());
        pl->SetArenaTeamIdInvited(0);
        pl->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (GetSlot()*6) + 5, newmember.personal_rating );

        // hide promote/remove buttons
        if(CaptainGuid != PlayerGuid)
            pl->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (GetSlot() * 6) + 1, 1);
        TC_LOG_DEBUG("arena","Player: %s [GUID: %u] joined arena team type: %u [Id: %u].", pl->GetName().c_str(), pl->GetGUIDLow(), GetType(), GetId());
        LogsDatabase.PExecute("INSERT INTO arena_team_event (id, event, type, player, ip, time) VALUES (%u, %u, %u, %u, '%s', %u)",
            GetId(), uint32(AT_EV_JOIN), GetType(), GUID_LOPART(PlayerGuid), (pl ? pl->GetSession()->GetRemoteAddress().c_str() : ""), time(NULL));

    }
    return true;
}

bool ArenaTeam::LoadArenaTeamFromDB(const std::string teamname)
{
    std::string escapedname = teamname;
    CharacterDatabase.EscapeString(escapedname);

    QueryResult result = CharacterDatabase.PQuery("SELECT arenateamid FROM arena_team WHERE name='%s'", escapedname.c_str());
    if (!result)
        return false;

    Field *fields = result->Fetch();

    uint32 teamId = fields[0].GetUInt32();

    return LoadArenaTeamFromDB(teamId);
}

bool ArenaTeam::LoadArenaTeamFromDB(uint32 ArenaTeamId)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT arenateamid,name,captainguid,type,BackgroundColor,EmblemStyle,EmblemColor,BorderStyle,BorderColor FROM arena_team WHERE arenateamid = '%u'", ArenaTeamId);

    if(!result)
        return false;

    Field *fields = result->Fetch();

    Id = fields[0].GetUInt32();
    Name = fields[1].GetString();
    CaptainGuid  = MAKE_NEW_GUID(fields[2].GetUInt32(), 0, HIGHGUID_PLAYER);
    Type = fields[3].GetUInt32();
    BackgroundColor = fields[4].GetUInt32();
    EmblemStyle = fields[5].GetUInt32();
    EmblemColor = fields[6].GetUInt32();
    BorderStyle = fields[7].GetUInt32();
    BorderColor = fields[8].GetUInt32();

    // only load here, so additional checks can be made
    LoadStatsFromDB(ArenaTeamId);
    LoadMembersFromDB(ArenaTeamId);

    if(Empty())
    {
        // arena team is empty, delete from db
        TC_LOG_ERROR("arena","ArenaTeam %u does not have any members, deleting from db.", ArenaTeamId);
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        trans->PAppend("DELETE FROM arena_team WHERE arenateamid = '%u'", ArenaTeamId);
        trans->PAppend("DELETE FROM arena_team_member WHERE arenateamid = '%u'", ArenaTeamId);
        trans->PAppend("DELETE FROM arena_team_stats WHERE arenateamid = '%u'", ArenaTeamId);
        CharacterDatabase.CommitTransaction(trans);
        return false;
    }

    return true;
}

void ArenaTeam::LoadStatsFromDB(uint32 ArenaTeamId)
{
    //                                                     0      1     2    3      4     5         6
    QueryResult result = CharacterDatabase.PQuery("SELECT rating,games,wins,played,wins2,rank,nonplayedweeks FROM arena_team_stats WHERE arenateamid = '%u'", ArenaTeamId);

    if(!result)
        return;

    Field *fields = result->Fetch();

    stats.rating           = fields[0].GetUInt32();
    stats.games_week       = fields[1].GetUInt32();
    stats.wins_week        = fields[2].GetUInt32();
    stats.games_season     = fields[3].GetUInt32();
    stats.wins_season      = fields[4].GetUInt32();
    stats.rank             = fields[5].GetUInt32();
    stats.non_played_weeks = fields[6].GetUInt32();
}

void ArenaTeam::LoadMembersFromDB(uint32 ArenaTeamId)
{
    //                                                           0                1           2         3             4        5 
    QueryResult result = CharacterDatabase.PQuery("SELECT member.guid,played_week,wons_week,played_season,wons_season,personal_rating "
                                                   "FROM arena_team_member member "
                                                   "WHERE member.arenateamid = '%u'", ArenaTeamId);
    if(!result)
        return;

    do
    {
        Field *fields = result->Fetch();
        ArenaTeamMember newmember;
        newmember.guid            = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        newmember.games_week      = fields[1].GetUInt32();
        newmember.wins_week       = fields[2].GetUInt32();
        newmember.games_season    = fields[3].GetUInt32();
        newmember.wins_season     = fields[4].GetUInt32();
        newmember.personal_rating = fields[5].GetUInt32();
        
        // Put the player in the team
        members.push_back(newmember);

    }while( result->NextRow() );
}

void ArenaTeam::SetCaptain(const uint64& guid)
{
    // disable remove/promote buttons
    Player *oldcaptain = sObjectMgr->GetPlayer(GetCaptain());
    if(oldcaptain)
        oldcaptain->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 1 + (GetSlot() * 6), 1);

    // set new captain
    CaptainGuid = guid;

    // update database
    CharacterDatabase.PExecute("UPDATE arena_team SET captainguid = '%u' WHERE arenateamid = '%u'", GUID_LOPART(guid), Id);

    // enable remove/promote buttons
    Player *newcaptain = sObjectMgr->GetPlayer(guid);
    if(newcaptain)
    {
        newcaptain->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 1 + (GetSlot() * 6), 0);
        TC_LOG_DEBUG("arena","Player: %s [GUID: %u] promoted player: %s [GUID: %u] to leader of arena team [Id: %u] [Type: %u].", oldcaptain->GetName().c_str(), oldcaptain->GetGUIDLow(), newcaptain->GetName().c_str(), newcaptain->GetGUID(), GetId(), GetType());
        LogsDatabase.PExecute("INSERT INTO arena_team_event (id, event, type, player, ip, time) VALUES (%u, %u, %u, %u, '%s', %u)",
            GetId(), uint32(AT_EV_PROMOTE), GetType(), GUID_LOPART(guid), newcaptain->GetSession()->GetRemoteAddress().c_str(), time(NULL));
    }
}

void ArenaTeam::DelMember(uint64 guid)
{
    Player *player = sObjectMgr->GetPlayer(guid);
    if (player && player->InArena())
        return;
    
    for (MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        if (itr->guid == guid)
        {
            members.erase(itr);
            break;
        }
    }

    if(player)
    {
        player->SetInArenaTeam(0, GetSlot());
        player->GetSession()->SendArenaTeamCommandResult(ERR_ARENA_TEAM_QUIT_S, GetName(), "", 0);
        if(sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB))
            player->UpdateArenaTitles();
        // delete all info regarding this team
        for(int i = 0; i < 6; ++i)
        {
            player->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (GetSlot() * 6) + i, 0);
        }
        TC_LOG_DEBUG("arena","Player: %s [GUID: %u] left arena team type: %u [Id: %u].", player->GetName().c_str(), player->GetGUIDLow(), GetType(), GetId());
        LogsDatabase.PExecute("INSERT INTO arena_team_event (id, event, type, player, ip, time) VALUES (%u, %u, %u, %u, '%s', %u)",
            GetId(), uint32(AT_EV_LEAVE), GetType(), GUID_LOPART(guid), (player ? player->GetSession()->GetRemoteAddress().c_str() : ""), time(NULL));
    }
    CharacterDatabase.PExecute("DELETE FROM arena_team_member WHERE arenateamid = '%u' AND guid = '%u'", GetId(), GUID_LOPART(guid));
}

void ArenaTeam::Disband(WorldSession *session)
{
    if (Player *plr = session->GetPlayer()) {
        if (plr->InArena()) {
            ChatHandler chH = ChatHandler(plr);
            chH.PSendSysMessage("Vous ne pouvez pas détruire une équipe d'arène pendant un match d'arène.");
            return;
        }
    }
    
    // event
    WorldPacket data;
    session->BuildArenaTeamEventPacket(&data, ERR_ARENA_TEAM_DISBANDED_S, 2, session->GetPlayerName(), GetName(), "");
    BroadcastPacket(&data);

    while (!members.empty())
    {
        // Removing from members is done in DelMember.
        DelMember(members.front().guid);
    }

    Player *player = session->GetPlayer();
    if(player)
        TC_LOG_DEBUG("arena","Player: %s [GUID: %u] disbanded arena team type: %u [Id: %u].", player->GetName().c_str(), player->GetGUIDLow(), GetType(), GetId());
  
    LogsDatabase.PExecute("INSERT INTO arena_team_event (id, event, type, player, ip, time) VALUES (%u, %u, %u, %u, '%s', %u)",
            GetId(), uint32(AT_EV_DISBAND), GetType(), (player ? player->GetGUIDLow() : 0), session->GetRemoteAddress().c_str(), time(NULL));

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM arena_team WHERE arenateamid = '%u'", Id);
    trans->PAppend("DELETE FROM arena_team_member WHERE arenateamid = '%u'", Id); //< this should be alredy done by calling DelMember(memberGuids[j]); for each member
    trans->PAppend("DELETE FROM arena_team_stats WHERE arenateamid = '%u'", Id);
    CharacterDatabase.CommitTransaction(trans);
    sObjectMgr->RemoveArenaTeam(Id);
}

void ArenaTeam::Roster(WorldSession *session)
{
    Player *pl = NULL;
    WorldPacket data(SMSG_ARENA_TEAM_ROSTER, 100);
    data << uint32(GetId());                                // arena team id
    if(session->GetClientBuild() == BUILD_335)
        data << uint8(0);                                   // 3.0.8 unknown value but affect packet structure
    data << uint32(GetMembersSize());                       // members count
    data << uint32(GetType());                              // arena team type?
    
    for (auto itr : members)
    {
        CharacterNameData const* pData = sWorld->GetCharacterNameData(itr.guid);
        
        pl = ObjectAccessor::FindConnectedPlayer(itr.guid);

        data << uint64(itr.guid);                      // guid
        data << uint8((pl ? 1 : 0));                    // online flag
        data << (pData ? pData->m_name : "Unknown");       // member name
        data << uint32((itr.guid == GetCaptain() ? 0 : 1));// captain flag 0 captain 1 member
        data << uint8(pData ? pData->m_level : 0);        // unknown, level?
        data << uint8(pData ? pData->m_class : 0);        // class
        data << uint32(itr.games_week);                // played this week
        data << uint32(itr.wins_week);                 // wins this week
        data << uint32(itr.games_season);              // played this season
        data << uint32(itr.wins_season);               // wins this season
        data << uint32(itr.personal_rating);           // personal rating
    }
    session->SendPacket(&data);
    
}

void ArenaTeam::Query(WorldSession *session)
{
    WorldPacket data(SMSG_ARENA_TEAM_QUERY_RESPONSE, 4*7+GetName().size()+1);
    data << uint32(GetId());                                // team id
    data << GetName();                                      // team name
    data << uint32(GetType());                              // arena team type (2=2x2, 3=3x3 or 5=5x5)
    data << uint32(BackgroundColor);                        // background color
    data << uint32(EmblemStyle);                            // emblem style
    data << uint32(EmblemColor);                            // emblem color
    data << uint32(BorderStyle);                            // border style
    data << uint32(BorderColor);                            // border color
    session->SendPacket(&data);
}

void ArenaTeam::Stats(WorldSession *session)
{
    WorldPacket data(SMSG_ARENA_TEAM_STATS, 4*7);
    data << uint32(GetId());                                // arena team id
    data << uint32(stats.rating);                           // rating
    data << uint32(stats.games_week);                       // games this week
    data << uint32(stats.wins_week);                        // wins this week
    data << uint32(stats.games_season);                     // played this season
    data << uint32(stats.wins_season);                      // wins this season
    data << uint32(stats.rank);                             // rank
    session->SendPacket(&data);
}

void ArenaTeam::NotifyStatsChanged()
{
    // this is called after a rated match ended
    // updates arena team stats for every member of the team (not only the ones who participated!)
    for(MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        Player * plr = sObjectMgr->GetPlayer(itr->guid);
        if(plr)
            Stats(plr->GetSession());
    }
}

void ArenaTeam::InspectStats(WorldSession *session, uint64 guid)
{
    ArenaTeamMember* member = GetMember(guid);
    if(!member)
        return;

    WorldPacket data(MSG_INSPECT_ARENA_TEAMS, 8+1+4*6);
    data << uint64(guid);                                   // player guid
    data << uint8(GetSlot());                               // slot (0...2)
    data << uint32(GetId());                                // arena team id
    data << uint32(stats.rating);                           // rating
    data << uint32(stats.games_season);                     // season played
    data << uint32(stats.wins_season);                      // season wins
    data << uint32(member->games_season);                   // played (count of all games, that the inspected member participated...)
    data << uint32(member->personal_rating);                // personal rating
    session->SendPacket(&data);
}

void ArenaTeam::SetEmblem(uint32 backgroundColor, uint32 emblemStyle, uint32 emblemColor, uint32 borderStyle, uint32 borderColor)
{
    BackgroundColor = backgroundColor;
    EmblemStyle = emblemStyle;
    EmblemColor = emblemColor;
    BorderStyle = borderStyle;
    BorderColor = borderColor;

    CharacterDatabase.PExecute("UPDATE arena_team SET BackgroundColor='%u', EmblemStyle='%u', EmblemColor='%u', BorderStyle='%u', BorderColor='%u' WHERE arenateamid='%u'", BackgroundColor, EmblemStyle, EmblemColor, BorderStyle, BorderColor, Id);
}

void ArenaTeam::HandleDecay()
{
    uint32 minRating = sWorld->getConfig(CONFIG_ARENA_DECAY_MINIMUM_RATING);
    if(   stats.rating > minRating
       && stats.non_played_weeks >= sWorld->getConfig(CONFIG_ARENA_DECAY_CONSECUTIVE_WEEKS) )
    {
        uint32 decayValue = sWorld->getConfig(CONFIG_ARENA_DECAY_VALUE);
        uint32 diff = stats.rating - minRating; //can't be negative here because of if condition
        stats.rating -= diff < decayValue ? diff : decayValue; //don't go below decayValue
    }

    UpdateRank();
}

void ArenaTeam::SetStats(uint32 stat_type, uint32 value)
{
    switch(stat_type)
    {
        case STAT_TYPE_RATING:
            stats.rating = value;
            CharacterDatabase.PExecute("UPDATE arena_team_stats SET rating = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_GAMES_WEEK:
            stats.games_week = value;
            CharacterDatabase.PExecute("UPDATE arena_team_stats SET games = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_WINS_WEEK:
            stats.wins_week = value;
            CharacterDatabase.PExecute("UPDATE arena_team_stats SET wins = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_GAMES_SEASON:
            stats.games_season = value;
            CharacterDatabase.PExecute("UPDATE arena_team_stats SET played = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_WINS_SEASON:
            stats.wins_season = value;
            CharacterDatabase.PExecute("UPDATE arena_team_stats SET wins2 = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_RANK:
            stats.rank = value;
            CharacterDatabase.PExecute("UPDATE arena_team_stats SET rank = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_NONPLAYEDWEEKS:
            stats.non_played_weeks = value;
            CharacterDatabase.PExecute("UPDATE arena_team_stats SET nonplayedweeks = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        default:
            TC_LOG_ERROR("arena","unknown stat type in ArenaTeam::SetStats() %u", stat_type);
            break;
    }
}

void ArenaTeam::BroadcastPacket(WorldPacket *packet)
{
    for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        Player *player = sObjectMgr->GetPlayer(itr->guid);
        if(player)
            player->GetSession()->SendPacket(packet);
    }
}

uint8 ArenaTeam::GetSlotByType( uint32 type )
{
    switch(type)
    {
        case ARENA_TEAM_2v2: return 0;
        case ARENA_TEAM_3v3: return 1;
        case ARENA_TEAM_5v5: return 2;
        default:
            break;
    }
    TC_LOG_ERROR("arena","FATAL: Unknown arena team type %u for some arena team", type);
    return 0xFF;
}

bool ArenaTeam::HaveMember( const uint64& guid ) const
{
    for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
        if(itr->guid == guid)
            return true;

    return false;
}

uint32 ArenaTeam::GetPoints(uint32 MemberRating)
{
    // returns how many points would be awarded with this team type with this rating
    float points;

    uint32 rating = MemberRating + 150 < stats.rating ? MemberRating : stats.rating;

    if(rating<=1500)
        points = (float)rating * 0.22f + 14.0f;
    else
        points = 1511.26f / (1.0f + 1639.28f * exp(-0.00412f * (float)rating));

    // type penalties for <5v5 teams
    if(Type == ARENA_TEAM_2v2)
        points *= 0.76f;
    else if(Type == ARENA_TEAM_3v3)
        points *= 0.88f;

    return (uint32) points;
}

float ArenaTeam::GetChanceAgainst(uint32 own_rating, uint32 enemy_rating)
{
    // returns the chance to win against a team with the given rating, used in the rating adjustment calculation
    // ELO system
    return 1.0f/(1.0f+exp(log(10.0f)*(float)((float)enemy_rating - (float)own_rating)/400.0f));
}

void ArenaTeam::UpdateRank()
{
    stats.rank = 1;
    ObjectMgr::ArenaTeamMap::iterator i = sObjectMgr->GetArenaTeamMapBegin();
    for ( ; i != sObjectMgr->GetArenaTeamMapEnd(); ++i)
    {
        if (i->second->GetType() == this->Type && i->second->GetStats().rating > stats.rating)
            ++stats.rank;
    }
}

int32 ArenaTeam::WonAgainst(uint32 againstRating)
{
    // called when the team has won
    
    //update team rating
    int32 mod;
    if (sWorld->getConfig(CONFIG_BATTLEGROUND_ARENA_ALTERNATE_RATING) && stats.rating < 1900 && againstRating < 1900 && stats.rating > 1450 && againstRating > 1450) {
        mod = int32(15); // in case of 2 teams <1900, rating mod is 15
    } else {        
        float chance = GetChanceAgainst(stats.rating,againstRating); //'chance' calculation - to beat the opponent
        mod = (int32)floor(32.0f * (1.0f - chance)); //(ELO system with k=32)
        if(mod < 2) mod = 2; //custom rule
    }
    // modify the team stats accordingly
    stats.rating += mod;
    stats.games_week += 1;
    stats.wins_week += 1;
    stats.games_season += 1;
    stats.wins_season += 1;

    UpdateRank();

    if(sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB))
        sWorld->updateArenaLeadersTitles();

    // return the rating change, used to display it on the results screen
    return mod;
}

int32 ArenaTeam::LostAgainst(uint32 againstRating)
{
    // called when the team has lost

    //update team rating
    int32 mod;
    if (sWorld->getConfig(CONFIG_BATTLEGROUND_ARENA_ALTERNATE_RATING) && stats.rating < 1900 && againstRating < 1900 && stats.rating > 1450 && againstRating > 1450) {
        mod = int32(-15);  // in case of 2 teams <1900, rating mod is 15
    } else {
        //'chance' calculation - to loose to the opponent
        float chance = GetChanceAgainst(stats.rating,againstRating);
        mod = (int32)ceil(32.0f * (0.0f - chance)); //ELO system with k=32
    }

    // modify the team stats accordingly
    stats.rating += mod;
    stats.games_week += 1;
    stats.games_season += 1;
    //update team's rank

    stats.rank = 1;
    ObjectMgr::ArenaTeamMap::iterator i = sObjectMgr->GetArenaTeamMapBegin();
    for ( ; i != sObjectMgr->GetArenaTeamMapEnd(); ++i)
    {
        if (i->second->GetType() == this->Type && i->second->GetStats().rating > stats.rating)
            ++stats.rank;
    }

    if(sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB))
        sWorld->updateArenaLeadersTitles();

    // return the rating change, used to display it on the results screen
    return mod;
}

void ArenaTeam::MemberLost(Player * plr, uint32 againstRating)
{
    // called for each participant of a match after losing
    for(MemberList::iterator itr = members.begin(); itr !=  members.end(); ++itr)
    {
        if(itr->guid == plr->GetGUID())
        {
            // update personal rating
            int32 mod;
            if (sWorld->getConfig(CONFIG_BATTLEGROUND_ARENA_ALTERNATE_RATING) && stats.rating < 1900 && againstRating < 1900 && stats.rating > 1450 && againstRating > 1450) {
                mod = int32(-15); // in case of 2 teams <1900, rating mod is 15
            } else {
                float chance = GetChanceAgainst(itr->personal_rating, againstRating);
                mod = (int32)ceil(32.0f * (0.0f - chance)); //(ELO system with k=32)
            }
            itr->ModifyPersonalRating(plr, mod, GetSlot());
            // update personal played stats
            itr->games_week +=1;
            itr->games_season +=1;
            // update the unit fields
            plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 6 * GetSlot() + 2, itr->games_week);
            plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 6 * GetSlot() + 3, itr->games_season);
            return;
        }
    }
}

void ArenaTeam::MemberWon(Player * plr, uint32 againstRating)
{
    // called for each participant after winning a match
    for(MemberList::iterator itr = members.begin(); itr !=  members.end(); ++itr)
    {
        if(itr->guid == plr->GetGUID())
        {
            // update personal rating
            int32 mod;
            if (sWorld->getConfig(CONFIG_BATTLEGROUND_ARENA_ALTERNATE_RATING) && stats.rating < 1900 && againstRating < 1900 && stats.rating > 1450 && againstRating > 1450) {
                mod = int32(15);
            } else {
                float chance = GetChanceAgainst(itr->personal_rating, againstRating);
                 mod = (int32)floor(32.0f * (1.0f - chance)); //(ELO system with k=32)
                 if (mod < 2) mod = 2; //custom rule
            }
            itr->ModifyPersonalRating(plr, mod, GetSlot());
            // update personal stats
            itr->games_week +=1;
            itr->games_season +=1;
            itr->wins_season += 1;
            itr->wins_week += 1;
            // update unit fields
            plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 6 * GetSlot() + 2, itr->games_week);
            plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 6 * GetSlot() + 3, itr->games_season);
            return;
        }
    }
}

void ArenaTeam::UpdateArenaPointsHelper(std::map<uint32, uint32>& PlayerPoints)
{
    // helper function for arena point distribution (this way, when distributing, no actual calculation is required, just a few comparisons)
    // 10 played games per week is a minimum
    if (stats.games_week < 10)
    {
        stats.non_played_weeks++;
        return;
    } else { //reset counter
        stats.non_played_weeks = 0;
    }

    // to get points, a player has to participate in at least 30% of the matches
    uint32 min_plays = (uint32) ceil(stats.games_week * 0.3);
    for(MemberList::iterator itr = members.begin(); itr !=  members.end(); ++itr)
    {
        // the player participated in enough games, update his points
        uint32 points_to_add = 0;
        if (itr->games_week >= min_plays)
            points_to_add = GetPoints(itr->personal_rating);
        // OBSOLETE : CharacterDatabase.PExecute("UPDATE arena_team_member SET points_to_add = '%u' WHERE arenateamid = '%u' AND guid = '%u'", points_to_add, Id, itr->guid);

        std::map<uint32, uint32>::iterator plr_itr = PlayerPoints.find(GUID_LOPART(itr->guid));
        if (plr_itr != PlayerPoints.end())
        {
            //check if there is already more points
            if (plr_itr->second < points_to_add)
                PlayerPoints[GUID_LOPART(itr->guid)] = points_to_add;
        }
        else
            PlayerPoints[GUID_LOPART(itr->guid)] = points_to_add;
    }
}

void ArenaTeam::SaveToDB()
{
    // save team and member stats to db
    // called after a match has ended, or when calculating arena_points
    CharacterDatabase.PExecute("UPDATE arena_team_stats SET rating = '%u',games = '%u',played = '%u',rank = '%u',wins = '%u',wins2 = '%u', nonplayedweeks = '%u' WHERE arenateamid = '%u'", stats.rating, stats.games_week, stats.games_season, stats.rank, stats.wins_week, stats.wins_season, stats.non_played_weeks, GetId());
    for(MemberList::iterator itr = members.begin(); itr !=  members.end(); ++itr)
    {
        CharacterDatabase.PExecute("UPDATE arena_team_member SET played_week = '%u', wons_week = '%u', played_season = '%u', wons_season = '%u', personal_rating = '%u' WHERE arenateamid = '%u' AND guid = '%u'", itr->games_week, itr->wins_week, itr->games_season, itr->wins_season, itr->personal_rating, Id, itr->guid);
    }
}

void ArenaTeam::FinishWeek()
{
    stats.games_week = 0;                                   // played this week
    stats.wins_week = 0;                                    // wins this week
    for(MemberList::iterator itr = members.begin(); itr !=  members.end(); ++itr)
    {
        itr->games_week = 0;
        itr->wins_week = 0;
    }
}

void ArenaTeam::GetMembers(std::list<ArenaTeamMember*>& memberList)
{
    memberList.clear();
    for (MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
        memberList.push_back(&(*itr));
}

/*
arenateam fields (id from 2.3.3 client):
1414 - arena team id 2v2
1415 - 0=captain, 1=member
1416 - played this week
1417 - played this season
1418 - unk - rank?
1419 - personal arena rating
1420 - arena team id 3v3
1421 - 0=captain, 1=member
1422 - played this week
1423 - played this season
1424 - unk - rank?
1425 - personal arena rating
1426 - arena team id 5v5
1427 - 0=captain, 1=member
1428 - played this week
1429 - played this season
1430 - unk - rank?
1431 - personal arena rating
*/

