
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "ArenaTeam.h"
#include "Chat.h"
#include "World.h"
#include "LogsDatabaseAccessor.h"
#include "CharacterCache.h"
#include "BattleGroundMgr.h"
#include "ArenaTeamMgr.h"

ArenaTeam::ArenaTeam()
    : TeamId(0), Type(0), TeamName(), CaptainGuid(), BackgroundColor(0), EmblemStyle(0), EmblemColor(0),
    BorderStyle(0), BorderColor(0)
{
    Stats.WeekGames        = 0;
    Stats.SeasonGames      = 0;
    Stats.Rank             = 0;
    Stats.Rating           = 1500; /*sWorld->getIntConfig(CONFIG_ARENA_START_RATING);*/
    Stats.WeekWins        = 0;
    Stats.SeasonWins      = 0;
    Stats.NonPlayedWeeks = 0;
}

ArenaTeam::~ArenaTeam()
{
}

bool ArenaTeam::Create(ObjectGuid captainGuid, uint8 type, std::string const teamName, uint32 backgroundColor, uint8 emblemStyle, uint32 emblemColor, uint8 borderStyle, uint32 borderColor)
{
    // Check if captain is present
    if (!ObjectAccessor::FindPlayer(captainGuid))
        return false;

    // Check if arena team name is already taken
    if (sArenaTeamMgr->GetArenaTeamByName(teamName))
        return false;

    // Generate new arena team id
    TeamId = sArenaTeamMgr->GenerateArenaTeamId();

    // Assign member variables
    CaptainGuid = captainGuid;
    Type = type;
    TeamName = teamName;
    BackgroundColor = backgroundColor;
    EmblemStyle = emblemStyle;
    EmblemColor = emblemColor;
    BorderStyle = borderStyle;
    BorderColor = borderColor;
    ObjectGuid::LowType captainLowGuid = captainGuid.GetCounter();

    // Save arena team to db
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ARENA_TEAM);
    stmt->setUInt32(0, TeamId);
    stmt->setString(1, TeamName);
    stmt->setUInt32(2, captainLowGuid);
    stmt->setUInt8(3, Type);
    stmt->setUInt16(4, Stats.Rating);
    stmt->setUInt32(5, BackgroundColor);
    stmt->setUInt8(6, EmblemStyle);
    stmt->setUInt32(7, EmblemColor);
    stmt->setUInt8(8, BorderStyle);
    stmt->setUInt32(9, BorderColor);
    trans->Append(stmt);

    std::string ip = "unknown";
    if (Player* captain = ObjectAccessor::FindPlayer(GetCaptain()))
        ip = captain->GetSession()->GetRemoteAddress();
    LogsDatabase.PExecute("INSERT INTO arena_team_event (id, event, type, player, ip, time) VALUES (%u, %u, %u, %u, '%s', %u)",
        GetId(), uint32(AT_EV_CREATE), GetType(), GetCaptain().GetCounter(), ip.c_str(), time(nullptr));

    // Add captain as member
    AddMember(CaptainGuid, trans);
    CharacterDatabase.CommitTransaction(trans);

    TC_LOG_DEBUG("bg.arena", "New ArenaTeam created [Id: %u, Name: %s] [Type: %u] [Captain low GUID: %u]", GetId(), GetName().c_str(), GetType(), captainLowGuid);
    return true;
}

bool ArenaTeam::AddMember(ObjectGuid playerGuid, SQLTransaction trans)
{
    // arena team is full (can't have more than type * 2 players!)
    if(GetMembersSize() >= GetType() * 2)
        return false;

    std::string playerName;
    uint8 playerClass;

    Player* player = ObjectAccessor::FindPlayer(playerGuid);
    if(player)
    {
        playerClass = player->GetClass();
        playerName = player->GetName();
    }
    else
    {
        CharacterCacheEntry const* cInfo = sCharacterCache->GetCharacterCacheByGuid(playerGuid);
        if (!cInfo)
            return false;

        playerName = cInfo->name;
        playerClass = cInfo->playerClass;
    }

    // Check if player is already in a similar arena team
    if ((player && player->GetArenaTeamId(GetSlot())) || Player::GetArenaTeamIdFromCharacterInfo(playerGuid, GetType()) != 0)
    {
        TC_LOG_DEBUG("bg.arena", "Arena: %u %s already has an arena team of type %u", playerGuid.GetCounter(), playerName.c_str(), GetType());
        return false;
    }

    // remove all player signs from another petitions
    // this will be prevent attempt joining player to many arenateams and corrupt arena team data integrity
    Player::RemovePetitionsAndSigns(trans, playerGuid, CharterTypes(GetType()));

    ArenaTeamMember newmember;
    newmember.Guid              = playerGuid;
    newmember.SeasonGames      = 0;
    newmember.WeekGames        = 0;
    newmember.SeasonWins       = 0;
    newmember.WeekWins         = 0;
    newmember.PersonalRating   = 1500;
    Members.push_back(newmember);

    sCharacterCache->UpdateCharacterArenaTeamId(playerGuid, GetSlot(), GetId());

    // Save player's arena team membership to db
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ARENA_TEAM_MEMBER);
    stmt->setUInt32(0, TeamId);
    stmt->setUInt32(1, playerGuid.GetCounter());
    CharacterDatabase.Execute(stmt);

    // Inform player if online
    if(player)
    {
        player->SetInArenaTeam(TeamId, GetSlot(), GetType());
        player->SetArenaTeamIdInvited(0);
        player->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (GetSlot()*6) + 5, newmember.PersonalRating );

        // hide promote/remove buttons
        if(CaptainGuid != playerGuid)
            player->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (GetSlot() * 6) + 1, 1);

        TC_LOG_DEBUG("arena","Player: %s [GUID: %u] joined arena team type: %u [Id: %u].", player->GetName().c_str(), player->GetGUID().GetCounter(), GetType(), GetId());
    }
    return true;
}

bool ArenaTeam::SetName(std::string const& name)
{
    if (TeamName == name || name.empty() || name.length() > 24 || sObjectMgr->IsReservedName(name) || !ObjectMgr::IsValidCharterName(name))
        return false;

    TeamName = name;
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ARENA_TEAM_NAME);
    stmt->setString(0, TeamName);
    stmt->setUInt32(1, GetId());
    CharacterDatabase.Execute(stmt);
    return true;
}

bool ArenaTeam::LoadArenaTeamFromDB(QueryResult const result)
{
    Field *fields = result->Fetch();

    TeamId = fields[0].GetUInt32();
    TeamName = fields[1].GetString();
    CaptainGuid  = ObjectGuid(HighGuid::Player, fields[2].GetUInt32());
    Type = fields[3].GetUInt8();
    BackgroundColor = fields[4].GetUInt32();
    EmblemStyle = fields[5].GetUInt8();
    EmblemColor = fields[6].GetUInt32();
    BorderStyle = fields[7].GetUInt8();
    BorderColor = fields[8].GetUInt32();
    Stats.Rating = fields[9].GetUInt16();
    Stats.WeekGames = fields[10].GetUInt16();
    Stats.WeekWins = fields[11].GetUInt16();
    Stats.SeasonGames = fields[12].GetUInt16();
    Stats.SeasonWins = fields[13].GetUInt16();
    Stats.Rank = fields[14].GetUInt32();
    Stats.NonPlayedWeeks = fields[15].GetUInt16();

    return true;
}

bool ArenaTeam::LoadMembersFromDB(QueryResult const result)
{
    bool captainPresentInTeam = false;

    do
    {
        Field *fields = result->Fetch();
        // Prevent crash if db records are broken when all members in result are already processed and current team doesn't have any members
        if (!fields)
            break;

        uint32 arenaTeamId = fields[0].GetUInt32();

        // We loaded all members for this arena_team already, break cycle
        if (arenaTeamId > TeamId)
            break;

        ArenaTeamMember newMember;
        newMember.Guid            = ObjectGuid(HighGuid::Player, fields[1].GetUInt32());
        newMember.WeekGames      = fields[2].GetUInt16();
        newMember.WeekWins       = fields[3].GetUInt16();
        newMember.SeasonGames    = fields[4].GetUInt16();
        newMember.SeasonWins     = fields[5].GetUInt16();
        newMember.Name           = fields[6].GetString();
        newMember.Class          = fields[7].GetUInt8();
        newMember.PersonalRating = fields[8].GetUInt16();

        // Check if team team has a valid captain
        if (newMember.Guid == GetCaptain())
            captainPresentInTeam = true;

        // Put the player in the team
        Members.push_back(std::move(newMember));
        sCharacterCache->UpdateCharacterArenaTeamId(newMember.Guid, GetSlot(), GetId());

    }while( result->NextRow() );

    if (Empty() || !captainPresentInTeam)
    {
        // Arena team is empty or captain is not in team, delete from db
        TC_LOG_DEBUG("bg.arena", "ArenaTeam %u does not have any members or its captain is not in team, disbanding it...", TeamId);
        return false;
    }

    return true;
}

void ArenaTeam::SetCaptain(ObjectGuid guid)
{
    // disable remove/promote buttons
    Player *oldcaptain = ObjectAccessor::FindPlayer(GetCaptain());
    if(oldcaptain)
        oldcaptain->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 1 + (GetSlot() * 6), 1);

    // set new captain
    CaptainGuid = guid;

    // update database
    CharacterDatabase.PExecute("UPDATE arena_team SET captainguid = '%u' WHERE arenateamid = '%u'", guid.GetCounter(), TeamId);

    // enable remove/promote buttons
    Player *newcaptain = ObjectAccessor::FindPlayer(guid);
    if(newcaptain)
    {
        newcaptain->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 1 + (GetSlot() * 6), 0);
        TC_LOG_DEBUG("arena", "Player: %s [GUID: %u] promoted player : %s[GUID: %u] to leader of arena team[Id:%u][Type:%u].", oldcaptain ? oldcaptain->GetName().c_str() : "", oldcaptain ? oldcaptain->GetGUID().GetCounter() : 0, newcaptain->GetName().c_str(), newcaptain->GetGUID().GetCounter(), GetId(), GetType());
        LogsDatabase.PExecute("INSERT INTO arena_team_event (id, event, type, player, ip, time) VALUES (%u, %u, %u, %u, '%s', %u)",
            GetId(), uint32(AT_EV_PROMOTE), GetType(), guid.GetCounter(), newcaptain->GetSession()->GetRemoteAddress().c_str(), time(nullptr));
    }
}

void ArenaTeam::DeleteMember(ObjectGuid guid, bool cleanDb)
{
    Player *player = ObjectAccessor::FindConnectedPlayer(guid);
    Group* group = (player && player->GetGroup()) ? player->GetGroup() : nullptr;
    if (player && player->InArena())
        return;
    
    for (auto itr = Members.begin(); itr != Members.end(); ++itr)
    {
        // Remove queues of members
        if (Player* playerMember = ObjectAccessor::FindConnectedPlayer(itr->Guid))
        {
            if (group && playerMember->GetGroup() && group->GetGUID() == playerMember->GetGroup()->GetGUID())
            {
                if (BattlegroundQueueTypeId bgQueue = BattlegroundMgr::BGQueueTypeId(BATTLEGROUND_AA, GetType()))
                {
                    GroupQueueInfo ginfo;
                    BattlegroundQueue& queue = sBattlegroundMgr->GetBattlegroundQueue(bgQueue);
                    if (queue.GetPlayerGroupInfoData(playerMember->GetGUID(), &ginfo))
                        if (!ginfo.IsInvitedToBGInstanceGUID)
                        {
                            WorldPacket data;
                            playerMember->RemoveBattlegroundQueueId(bgQueue);
                            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, nullptr, playerMember->GetBattlegroundQueueIndex(bgQueue), STATUS_NONE, 0, 0, 0, 0);
                            queue.RemovePlayer(playerMember->GetGUID(), true);
                            playerMember->GetSession()->SendPacket(&data);
                        }
                }
            }
        }

        if (itr->Guid == guid)
        {
            Members.erase(itr);
            sCharacterCache->UpdateCharacterArenaTeamId(guid, GetSlot(), 0);
            break;
        }
    }

    if(player)
    {
        player->SetInArenaTeam(0, GetSlot(), GetType());
        player->GetSession()->SendArenaTeamCommandResult(ERR_ARENA_TEAM_QUIT_S, GetName(), "", 0);
        if(sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB))
            player->UpdateArenaTitles();
        // delete all info regarding this team
        for(int i = 0; i < 6; ++i)
        {
            player->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (GetSlot() * 6) + i, 0);
        }
        TC_LOG_DEBUG("arena","Player: %s [GUID: %u] left arena team type: %u [Id: %u].", player->GetName().c_str(), player->GetGUID().GetCounter(), GetType(), GetId());
        LogsDatabase.PExecute("INSERT INTO arena_team_event (id, event, type, player, ip, time) VALUES (%u, %u, %u, %u, '%s', %u)",
            GetId(), uint32(AT_EV_LEAVE), GetType(), guid.GetCounter(), player->GetSession()->GetRemoteAddress().c_str(), time(nullptr));
    }
    if(cleanDb)
        CharacterDatabase.PExecute("DELETE FROM arena_team_member WHERE arenateamid = '%u' AND guid = '%u'", GetId(), guid.GetCounter());
}

void ArenaTeam::Disband(WorldSession* session)
{
    if (session)
    {
        if (Player *plr = session->GetPlayer()) 
        {
            if (plr->InArena()) 
            {
                ChatHandler chH = ChatHandler(plr);
                //chH.PSendSysMessage("Vous ne pouvez pas détruire une équipe d'arène pendant un match d'arène."); //TODO TRanslate
                chH.PSendSysMessage("Not while in arena.");
                return;
            }
        }

        // Broadcast update
        WorldPacket data;
        session->BuildArenaTeamEventPacket(&data, ERR_ARENA_TEAM_DISBANDED_S, 2, session->GetPlayerName(), GetName(), "");
        BroadcastPacket(&data);
    }
    
    while (!Members.empty())
    {
        // Removing from Members is done in DeleteMember.
        DeleteMember(Members.front().Guid, false);
    }

    Player* player = session ? session->GetPlayer() : nullptr;
    if(player)
        TC_LOG_DEBUG("arena","Player: %s [GUID: %u] disbanded arena team type: %u [Id: %u].", player->GetName().c_str(), player->GetGUID().GetCounter(), GetType(), GetId());
  
    LogsDatabase.PExecute("INSERT INTO arena_team_event (id, event, type, player, ip, time) VALUES (%u, %u, %u, %u, '%s', %u)",
            GetId(), uint32(AT_EV_DISBAND), GetType(), (player ? player->GetGUID().GetCounter() : 0), session ? session->GetRemoteAddress().c_str() : "", time(nullptr));

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ARENA_TEAM);
    stmt->setUInt32(0, TeamId);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ARENA_TEAM_MEMBERS);
    stmt->setUInt32(0, TeamId);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);

    sArenaTeamMgr->RemoveArenaTeam(TeamId);
}

void ArenaTeam::Roster(WorldSession *session)
{
    Player *pl = nullptr;
    WorldPacket data(SMSG_ARENA_TEAM_ROSTER, 100);
    data << uint32(GetId());                                // arena team id
#ifdef LICH_KING
    data << uint8(0);                                       // 3.0.8 unknown value
#endif
    data << uint32(GetMembersSize());                       // Members count
    data << uint32(GetType());                              // arena team type?
    
    for (auto itr : Members)
    {
        CharacterCacheEntry const* pData = sCharacterCache->GetCharacterCacheByGuid(itr.Guid);
        
        pl = ObjectAccessor::FindConnectedPlayer(itr.Guid);

        data << uint64(itr.Guid);                      // guid
        data << uint8((pl ? 1 : 0));                    // online flag
        data << (pData ? pData->name : "Unknown");       // member name
        data << uint32((itr.Guid == GetCaptain() ? 0 : 1));// captain flag 0 captain 1 member
        data << uint8(pData ? pData->level : 0);        // unknown, level?
        data << uint8(pData ? pData->playerClass : 0);        // class
        data << uint32(itr.WeekGames);                // played this week
        data << uint32(itr.WeekWins);                 // wins this week
        data << uint32(itr.SeasonGames);              // played this season
        data << uint32(itr.SeasonWins);               // wins this season
        data << uint32(itr.PersonalRating);           // personal rating
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

void ArenaTeam::SendStats(WorldSession *session)
{
    WorldPacket data(SMSG_ARENA_TEAM_STATS, 4*7);
    data << uint32(GetId());                                // arena team id
    data << uint32(Stats.Rating);                           // rating
    data << uint32(Stats.WeekGames);                       // games this week
    data << uint32(Stats.WeekWins);                        // wins this week
    data << uint32(Stats.SeasonGames);                     // played this season
    data << uint32(Stats.SeasonWins);                      // wins this season
    data << uint32(Stats.Rank);                             // rank
    session->SendPacket(&data);
}

void ArenaTeam::NotifyStatsChanged()
{
    // this is called after a rated match ended
    // updates arena team stats for every member of the team (not only the ones who participated!)
    for(MemberList::const_iterator itr = Members.begin(); itr != Members.end(); ++itr)
    {
        Player * plr = ObjectAccessor::FindConnectedPlayer(itr->Guid);
        if(plr)
            SendStats(plr->GetSession());
    }
}

void ArenaTeam::Inspect(WorldSession *session, ObjectGuid guid)
{
    ArenaTeamMember* member = GetMember(guid);
    if(!member)
        return;

    WorldPacket data(MSG_INSPECT_ARENA_TEAMS, 8+1+4*6);  //LK ok
    data << uint64(guid);                                   // player guid
    data << uint8(GetSlot());                               // slot (0...2)
    data << uint32(GetId());                                // arena team id
    data << uint32(Stats.Rating);                           // rating
    data << uint32(Stats.SeasonGames);                     // season played
    data << uint32(Stats.SeasonWins);                      // season wins
    data << uint32(member->SeasonGames);                   // played (count of all games, that the inspected member participated...)
    data << uint32(member->PersonalRating);                // personal rating
    session->SendPacket(&data);
}

void ArenaTeam::HandleDecay()
{
    uint32 minRating = sWorld->getConfig(CONFIG_ARENA_DECAY_MINIMUM_RATING);
    if(   Stats.Rating > minRating
       && Stats.NonPlayedWeeks >= sWorld->getConfig(CONFIG_ARENA_DECAY_CONSECUTIVE_WEEKS) )
    {
        uint32 decayValue = sWorld->getConfig(CONFIG_ARENA_DECAY_VALUE);
        uint32 diff = Stats.Rating - minRating; //can't be negative here because of if condition
        Stats.Rating -= diff < decayValue ? diff : decayValue; //don't go below decayValue
    }

    UpdateRank();
}

void ArenaTeam::BroadcastPacket(WorldPacket *packet)
{
    for (MemberList::const_iterator itr = Members.begin(); itr != Members.end(); ++itr)
        if (Player* player = ObjectAccessor::FindConnectedPlayer(itr->Guid))
            player->SendDirectMessage(packet);
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

bool ArenaTeam::HaveMember(ObjectGuid guid ) const
{
    for (const auto & member : Members)
        if(member.Guid == guid)
            return true;

    return false;
}

ArenaTeamMember* ArenaTeam::GetMember(ObjectGuid guid)
{
    for (auto & member : Members)
        if (member.Guid == guid)
            return &member;

    return nullptr;
}

ArenaTeamMember* ArenaTeam::GetMember(const std::string& name)
{
    for (auto & member : Members)
    {
        std::string fetchedName;
        bool result = sCharacterCache->GetCharacterNameByGuid(member.Guid, fetchedName);
        if (!result)
            return nullptr;

        if (fetchedName == name)
            return &member;
    }

    return nullptr;
}

uint32 ArenaTeam::GetPoints(uint32 MemberRating)
{
    // returns how many points would be awarded with this team type with this rating
    float points;

    uint32 rating = MemberRating + 150 < Stats.Rating ? MemberRating : Stats.Rating;

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
    Stats.Rank = 1;
    auto i = sArenaTeamMgr->GetArenaTeamMapBegin();
    for ( ; i != sArenaTeamMgr->GetArenaTeamMapEnd(); ++i)
    {
        if (i->second->GetType() == this->Type && i->second->GetStats().Rating > Stats.Rating)
            ++Stats.Rank;
    }
}


int32 ArenaTeam::GetRatingMod(uint32 ownRating, uint32 opponentRating, bool won /*, float confidence_factor*/)
{
    // 'Chance' calculation - to beat the opponent
    // This is a simulation. Not much info on how it really works
    float chance = GetChanceAgainst(ownRating, opponentRating);

#ifdef LICH_KING
    // Calculate the rating modification
    float mod;

    /// @todo Replace this hack with using the confidence factor (limiting the factor to 2.0f)
    if (won)
    {
        if (ownRating < 1300)
        {
            float win_rating_modifier1 = 1.0f; //TC sWorld->getFloatConfig(CONFIG_ARENA_WIN_RATING_MODIFIER_1);

            if (ownRating < 1000)
                mod = win_rating_modifier1 * (1.0f - chance);
            else
                mod = ((win_rating_modifier1 / 2.0f) + ((win_rating_modifier1 / 2.0f) * (1300.0f - float(ownRating)) / 300.0f)) * (1.0f - chance);
        }
        else
            mod = /* sWorld->getFloatConfig(CONFIG_ARENA_WIN_RATING_MODIFIER_2) * */ (1.0f - chance);
    }
    else
        mod = /* sWorld->getFloatConfig(CONFIG_ARENA_LOSE_RATING_MODIFIER) * */ (-chance);

    return (int32)ceil(mod);
#else
    //update team rating
    int32 mod;
    if (sWorld->getConfig(CONFIG_BATTLEGROUND_ARENA_ALTERNATE_RATING) && ownRating < 1900 && opponentRating < 1900 && ownRating > 1450 && opponentRating > 1450) {
        mod = int32(15); // in case of 2 teams <1900, rating mod is 15
    }
    else {
        mod = (int32)floor(32.0f * (1.0f - chance)); //(ELO system with k=32)
        if (mod < 1) 
            mod = 1; //custom rule
    }
    return won ? mod : -mod;
#endif
}

int32 ArenaTeam::WonAgainst(uint32 againstRating)
{
    int32 mod = GetRatingMod(Stats.Rating, againstRating, true);

    // modify the team stats accordingly
    Stats.Rating += mod;
    Stats.WeekGames += 1;
    Stats.WeekWins += 1;
    Stats.SeasonGames += 1;
    Stats.SeasonWins += 1;

    UpdateRank();

    if(sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB))
        sWorld->updateArenaLeadersTitles();

    // return the rating change, used to display it on the results screen
    return mod;
}

int32 ArenaTeam::LostAgainst(uint32 againstRating)
{
    int32 mod = GetRatingMod(Stats.Rating, againstRating, false);

    // modify the team stats accordingly
    Stats.Rating += mod;
    Stats.WeekGames += 1;
    Stats.SeasonGames += 1;

    UpdateRank();

    if(sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB))
        sWorld->updateArenaLeadersTitles();

    // return the rating change, used to display it on the results screen
    return mod;
}

void ArenaTeam::MemberLost(Player * plr, uint32 againstRating)
{
    // called for each participant of a match after losing
    for(auto & member : Members)
    {
        if(member.Guid == plr->GetGUID())
        {
            // update personal rating
            int32 mod = GetRatingMod(Stats.Rating, againstRating, false);
            member.ModifyPersonalRating(plr, mod, GetSlot());

            // Update matchmaker rating
            //TC itr->ModifyMatchmakerRating(MatchmakerRatingChange, GetSlot());

            // update personal played stats
            member.WeekGames +=1;
            member.SeasonGames +=1;
            // update the unit fields
            plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 6 * GetSlot() + 2, member.WeekGames);
            plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 6 * GetSlot() + 3, member.SeasonGames);
            return;
        }
    }
}

void ArenaTeam::OfflineMemberLost(ObjectGuid guid, uint32 againstMatchmakerRating, int32 MatchmakerRatingChange)
{
    // Called for offline player after ending rated arena match!
    for (MemberList::iterator itr = Members.begin(); itr != Members.end(); ++itr)
    {
        if (itr->Guid == guid)
        {
            // update personal rating
            int32 mod = GetRatingMod(itr->PersonalRating, againstMatchmakerRating, false);
            itr->ModifyPersonalRating(nullptr, mod, GetType());

            // update matchmaker rating
            //TC itr->ModifyMatchmakerRating(MatchmakerRatingChange, GetSlot());

            // update personal played stats
            itr->WeekGames += 1;
            itr->SeasonGames += 1;
            return;
        }
    }
}

void ArenaTeam::MemberWon(Player * plr, uint32 againstRating)
{
    // called for each participant after winning a match
    for(auto & member : Members)
    {
        if(member.Guid == plr->GetGUID())
        {
            // update personal rating
            int32 mod = GetRatingMod(member.PersonalRating, againstRating, true);
            member.ModifyPersonalRating(plr, mod, GetSlot());

            // update matchmaker rating
           //TC itr->ModifyMatchmakerRating(MatchmakerRatingChange, GetSlot());

            // update personal stats
            member.WeekGames +=1;
            member.SeasonGames +=1;
            member.SeasonWins += 1;
            member.WeekWins += 1;
            // update unit fields
            plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 6 * GetSlot() + 2, member.WeekGames);
            plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 6 * GetSlot() + 3, member.SeasonGames);
            return;
        }
    }
}

void ArenaTeam::UpdateArenaPointsHelper(std::map<ObjectGuid::LowType, uint32>& PlayerPoints)
{
    // helper function for arena point distribution (this way, when distributing, no actual calculation is required, just a few comparisons)
    // 10 played games per week is a minimum
    if (Stats.WeekGames < 10)
    {
        Stats.NonPlayedWeeks++;
        return;
    } else { //reset counter
        Stats.NonPlayedWeeks = 0;
    }

    // to get points, a player has to participate in at least 30% of the matches
    uint32 min_plays = (uint32) ceil(Stats.WeekGames * 0.3);
    for(auto itr = Members.begin(); itr !=  Members.end(); ++itr)
    {
        // the player participated in enough games, update his points
        uint32 points_to_add = 0;
        if (itr->WeekGames >= min_plays)
            points_to_add = GetPoints(itr->PersonalRating);
        // OBSOLETE : CharacterDatabase.PExecute("UPDATE arena_team_member SET points_to_add = '%u' WHERE arenateamid = '%u' AND guid = '%u'", points_to_add, Id, itr->guid);

        auto plr_itr = PlayerPoints.find(itr->Guid.GetCounter());
        if (plr_itr != PlayerPoints.end())
        {
            //check if there is already more points
            if (plr_itr->second < points_to_add)
                PlayerPoints[itr->Guid.GetCounter()] = points_to_add;
        }
        else
            PlayerPoints[itr->Guid.GetCounter()] = points_to_add;
    }
}

void ArenaTeam::SaveToDB()
{
    // Save team and member stats to db
    // Called after a match has ended or when calculating arena_points

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ARENA_TEAM_STATS);
    stmt->setUInt16(0, Stats.Rating);
    stmt->setUInt16(1, Stats.WeekGames);
    stmt->setUInt16(2, Stats.WeekWins);
    stmt->setUInt16(3, Stats.SeasonGames);
    stmt->setUInt16(4, Stats.SeasonWins);
    stmt->setUInt32(5, Stats.Rank);
    stmt->setUInt32(6, Stats.NonPlayedWeeks);
    stmt->setUInt32(7, GetId());
    trans->Append(stmt);

    for (MemberList::const_iterator itr = Members.begin(); itr != Members.end(); ++itr)
    {
        // Save the effort and go
        if (itr->WeekGames == 0)
            continue;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ARENA_TEAM_MEMBER);
        stmt->setUInt16(0, itr->PersonalRating);
        stmt->setUInt16(1, itr->WeekGames);
        stmt->setUInt16(2, itr->WeekWins);
        stmt->setUInt16(3, itr->SeasonGames);
        stmt->setUInt16(4, itr->SeasonWins);
        stmt->setUInt32(5, GetId());
        stmt->setUInt32(6, itr->Guid.GetCounter());
        trans->Append(stmt);
    }

    CharacterDatabase.CommitTransaction(trans);
}

bool ArenaTeam::FinishWeek()
{
    if(Stats.WeekGames == 0)
        return false;

    Stats.WeekGames = 0;                                   // played this week
    Stats.WeekWins = 0;                                    // wins this week
    for(auto & member : Members)
    {
        member.WeekGames = 0;
        member.WeekWins = 0;
    }

    return true;
}

void ArenaTeam::GetMembers(std::list<ArenaTeamMember*>& memberList)
{
    memberList.clear();
    for (auto & member : Members)
        memberList.push_back(&member);
}

bool ArenaTeam::IsFighting() const
{
    for (const auto & member : Members)
    {
        if (Player *p = ObjectAccessor::FindPlayer(member.Guid))
        {
            if (p->GetMap()->IsBattleArena())
                return true;
        }
    }
    return false;
}

#ifdef LICH_KING
uint32 ArenaTeam::GetAverageMMR(Group* group) const
{
    if (!group)
        return 0;

    uint32 matchMakerRating = 0;
    uint32 playerDivider = 0;
    for (MemberList::const_iterator itr = Members.begin(); itr != Members.end(); ++itr)
    {
        // Skip if player is not online
        if (!ObjectAccessor::FindConnectedPlayer(itr->Guid))
            continue;

        // Skip if player is not member of group
        if (!group->IsMember(itr->Guid))
            continue;

        matchMakerRating += itr->MatchMakerRating;
        ++playerDivider;
    }

    // x/0 = crash
    if (playerDivider == 0)
        playerDivider = 1;

    matchMakerRating /= playerDivider;

    return matchMakerRating;
}
#endif

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

void ArenaTeamMember::ModifyPersonalRating(Player* player, int32 mod, uint32 slot)
{
    if ((int32(PersonalRating) + mod) < 0)
        PersonalRating = 0;
    else
        PersonalRating += mod;

    if (player)
    {

#ifdef LICH_KING
        player->SetArenaTeamInfoField(ArenaTeam::GetSlotByType(type), ARENA_TEAM_PERSONAL_RATING, PersonalRating);
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING, PersonalRating, type);
#else
        player->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (slot * 6) + 5, PersonalRating);
#endif
    }
}