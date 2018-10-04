
#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundAB.h"
#include "Creature.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"
#include "World.h"
#include "Util.h"
#include "DBCStores.h"
#include "BattleGroundMgr.h"

// these variables aren't used outside of this file, so declare them only here
uint32 BG_AB_HonorScoreTicks[BG_HONOR_MODE_NUM] = {
    330, // normal honor
    200  // holiday
};

uint32 BG_AB_ReputationScoreTicks[BG_HONOR_MODE_NUM] = {
    200, // normal honor
    150  // holiday
};

BattlegroundAB::BattlegroundAB()
{
    m_BuffChange = true;
    BgObjects.resize(BG_AB_OBJECT_MAX);
    //BgCreatures.resize(BG_AB_ALL_NODES_COUNT);
}

BattlegroundAB::~BattlegroundAB()
{
}

void BattlegroundAB::Update(time_t diff)
{
    Battleground::Update(diff);

    if (GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize())
    {
        ModifyStartDelayTime(diff);

        if (!(m_Events & BG_STARTING_EVENT_1))
        {
            m_Events |= BG_STARTING_EVENT_1;

            // setup here, only when at least one player has ported to the map
            if(!SetupBattleground())
            {
                EndNow();
                return;
            }

            StartingEventCloseDoors();
            // despawn banners, auras and buffs
            /*for (int obj = BG_AB_OBJECT_BANNER_NEUTRAL; obj < BG_AB_DYNAMIC_NODES_COUNT * 8; ++obj)
                SpawnBGObject(obj, RESPAWN_ONE_DAY);
            for (int i = 0; i < BG_AB_DYNAMIC_NODES_COUNT * 3; ++i)
                SpawnBGObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + i, RESPAWN_ONE_DAY);*/

            // Starting doors
            /*SpawnBGObject(BG_AB_OBJECT_GATE_A, RESPAWN_IMMEDIATELY);
            SpawnBGObject(BG_AB_OBJECT_GATE_H, RESPAWN_IMMEDIATELY);
            DoorClose(BG_AB_OBJECT_GATE_A);
            DoorClose(BG_AB_OBJECT_GATE_H);

            // Starting base spirit guides
            _NodeOccupied(BG_AB_SPIRIT_ALIANCE, ALLIANCE);
            _NodeOccupied(BG_AB_SPIRIT_HORDE, HORDE);*/

            SetStartDelayTime(START_DELAY0);
        }
        // After 1 minute, warning is signaled
        else if (GetStartDelayTime() <= START_DELAY1 && !(m_Events & BG_STARTING_EVENT_3))
        {
            m_Events |= BG_STARTING_EVENT_3;
            SendMessageToAll(GetTrinityString(LANG_BG_AB_ONEMINTOSTART));
        }
        // After 1,5 minute, warning is signaled
        else if (GetStartDelayTime() <= START_DELAY2 && !(m_Events & BG_STARTING_EVENT_4))
        {
            m_Events |= BG_STARTING_EVENT_4;
            SendMessageToAll(GetTrinityString(LANG_BG_AB_HALFMINTOSTART));
        }
        // After 2 minutes, gates OPEN ! 
        else if (GetStartDelayTime() < 0 && !(m_Events & BG_STARTING_EVENT_5))
        {
            m_Events |= BG_STARTING_EVENT_5;
            SendMessageToAll(GetTrinityString(LANG_BG_AB_STARTED));

            StartingEventOpenDoors();

            PlaySoundToAll(SOUND_BG_START);
            SetStatus(STATUS_IN_PROGRESS);

            for(const auto & itr : GetPlayers())
                if(Player* plr = ObjectAccessor::FindPlayer(itr.first))
                    plr->RemoveAurasDueToSpell(SPELL_PREPARATION);
        }

    }
    else if (GetStatus() == STATUS_IN_PROGRESS)
    {
        int team_points[2] = { 0, 0 };

        for (int node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
        {
            // 3 sec delay to spawn new banner instead previous despawned one
            if( m_BannerTimers[node].timer )
            {
                if( m_BannerTimers[node].timer > diff )
                    m_BannerTimers[node].timer -= diff;
                else
                {
                    m_BannerTimers[node].timer = 0;
                    _CreateBanner(node, m_BannerTimers[node].type, m_BannerTimers[node].teamIndex);
                }
            }

            // 1-minute to occupy a node from contested state
            if( m_NodeTimers[node] )
            {
                if( m_NodeTimers[node] > diff )
                    m_NodeTimers[node] -= diff;
                else
                {
                    m_NodeTimers[node] = 0;
                    // Change from contested to occupied !
                    uint8 teamIndex = m_Nodes[node]-1;
                    m_prevNodes[node] = m_Nodes[node];
                    m_Nodes[node] += 2;
                    // burn current contested banner
                    _DelBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex);
                    // create new occupied banner
                    _CreateBanner(node, BG_AB_NODE_TYPE_OCCUPIED, teamIndex);
                    _SendNodeUpdate(node);
                    _NodeOccupied(node,(teamIndex == 0) ? ALLIANCE:HORDE);
                    // Message to chatlog
                    if (teamIndex == 0)
                    {
                        // FIXME: team and node names not localized
                        SendMessage2ToAll(LANG_BG_AB_NODE_TAKEN, CHAT_MSG_BG_SYSTEM_ALLIANCE, nullptr, LANG_BG_AB_ALLY, _GetNodeNameId(node));
                        PlaySoundToAll(SOUND_NODE_CAPTURED_ALLIANCE);
                    }
                    else
                    {
                        // FIXME: team and node names not localized
                        SendMessage2ToAll(LANG_BG_AB_NODE_TAKEN, CHAT_MSG_BG_SYSTEM_HORDE, nullptr, LANG_BG_AB_HORDE, _GetNodeNameId(node));
                        PlaySoundToAll(SOUND_NODE_CAPTURED_HORDE);
                    }
                }
            }

            for (int team = 0; team < 2; ++team)
                if( m_Nodes[node] == team + BG_AB_NODE_TYPE_OCCUPIED )
                    ++team_points[team];
        }

        // Accumulate points
        for (int team = 0; team < 2; ++team)
        {
            int points = team_points[team];
            if( !points )
                continue;
            m_lastTick[team] += diff;
            if( m_lastTick[team] > BG_AB_TickIntervals[points] )
            {
                m_lastTick[team] -= BG_AB_TickIntervals[points];
                m_TeamScores[team] += BG_AB_TickPoints[points];
                m_score[team] = m_TeamScores[team];
                m_HonorScoreTics[team] += BG_AB_TickPoints[points];
                m_ReputationScoreTics[team] += BG_AB_TickPoints[points];
                if( m_ReputationScoreTics[team] >= BG_AB_ReputationScoreTicks[m_HonorMode] )
                {
                    (team == TEAM_ALLIANCE) ? RewardReputationToTeam(509, 10, ALLIANCE) : RewardReputationToTeam(510, 10, HORDE);
                    m_ReputationScoreTics[team] -= BG_AB_ReputationScoreTicks[m_HonorMode];
                }
                if( m_HonorScoreTics[team] >= BG_AB_HonorScoreTicks[m_HonorMode] )
                {
                    (team == TEAM_ALLIANCE) ? RewardHonorToTeam(20, ALLIANCE) : RewardHonorToTeam(20, HORDE);
                    m_HonorScoreTics[team] -= BG_AB_HonorScoreTicks[m_HonorMode];
                }
                if( !m_IsInformedNearVictory && m_TeamScores[team] > 1800 )
                {
                    if( team == TEAM_ALLIANCE )
                        SendMessageToAll(GetTrinityString(LANG_BG_AB_A_NEAR_VICTORY));
                    else
                        SendMessageToAll(GetTrinityString(LANG_BG_AB_H_NEAR_VICTORY));
                    PlaySoundToAll(SOUND_NEAR_VICTORY);
                    m_IsInformedNearVictory = true;
                }

                if( m_TeamScores[team] > 2000 )
                    m_TeamScores[team] = 2000;
                if( team == TEAM_ALLIANCE )
                    UpdateWorldState(BG_AB_OP_RESOURCES_ALLY, m_TeamScores[team]);
                if( team == TEAM_HORDE )
                    UpdateWorldState(BG_AB_OP_RESOURCES_HORDE, m_TeamScores[team]);
            }
        }

        // Test win condition
        if (m_TeamScores[TEAM_ALLIANCE] >= 2000) {
            RewardHonorToTeam(40, ALLIANCE);
            RewardHonorToTeam(20, HORDE);
            EndBattleground(ALLIANCE);
        }
        if (m_TeamScores[TEAM_HORDE] >= 2000) {
            RewardHonorToTeam(40, HORDE);
            RewardHonorToTeam(20, ALLIANCE);
            EndBattleground(HORDE);
        }
    }
}

void BattlegroundAB::StartingEventCloseDoors()
{
    // despawn buffs
    for (int i = 0; i < BG_AB_DYNAMIC_NODES_COUNT * 3; ++i)
        SpawnBGObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + i, RESPAWN_ONE_DAY);
}

void BattlegroundAB::StartingEventOpenDoors()
{
    for (int i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        //randomly select buff to spawn
        uint8 buff = urand(0, 2);
        SpawnBGObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + buff + i * 3, RESPAWN_IMMEDIATELY); 
    }
    OpenDoorEvent(BG_EVENT_DOOR);
}

void BattlegroundAB::AddPlayer(Player *plr)
{
    Battleground::AddPlayer(plr);
    //create score and add it to map, default values are set in the constructor
    auto sc = new BattlegroundABScore;

    PlayerScores[plr->GetGUID()] = sc;
}

void BattlegroundAB::RemovePlayer(Player * /*plr*/, ObjectGuid /*guid*/)
{

}

void BattlegroundAB::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    if( GetStatus() != STATUS_IN_PROGRESS )
        return;

    switch(Trigger)
    {
        case 3948:                                          // Arathi Basin Alliance Exit.
            if( Source->GetTeam() != ALLIANCE )
                Source->GetSession()->SendAreaTriggerMessage("Only The Alliance can use that portal");
            else
                Source->LeaveBattleground();
            break;
        case 3949:                                          // Arathi Basin Horde Exit.
            if( Source->GetTeam() != HORDE )
                Source->GetSession()->SendAreaTriggerMessage("Only The Horde can use that portal");
            else
                Source->LeaveBattleground();
            break;
        case 3866:                                          // Stables
        case 3869:                                          // Gold Mine
        case 3867:                                          // Farm
        case 3868:                                          // Lumber Mill
        case 3870:                                          // Black Smith
        case 4020:                                          // Unk1
        case 4021:                                          // Unk2
            //break;
        default:
            //TC_LOG_ERROR("WARNING: Unhandled AreaTrigger in Battleground: %u", Trigger);
            //Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
    }
}

/*  type: 0-neutral, 1-contested, 3-occupied
    teamIndex: 0-ally, 1-horde                        */
void BattlegroundAB::_CreateBanner(uint8 node, uint8 type, uint8 teamIndex, Player* invoker /*= nullptr*/)
{
    uint32 delay = 0;
    switch (type) 
    {
    case BG_AB_NODE_TYPE_CONTESTED:
        delay = 1;
        break;
    case BG_AB_NODE_TYPE_OCCUPIED:
        delay = 5;
        break;
    }

    // cause the node-type is in the generic form
    // please see in the headerfile for the ids
    if (type != BG_AB_NODE_TYPE_NEUTRAL)
        type += teamIndex;

    SpawnEvent(node, type, true, true, delay, invoker);
}

void BattlegroundAB::_DelBanner(uint8 node, uint8 type, uint8 teamIndex, Player* invoker)
{
    uint8 obj = node*8 + type + teamIndex;
    SpawnBGObject(obj, RESPAWN_ONE_DAY, invoker);

    // handle aura with banner
    if (!type)
        return;
    obj = node * 8 + ((type == BG_AB_NODE_TYPE_OCCUPIED) ? (5 + teamIndex) : 7);
    SpawnBGObject(obj, RESPAWN_ONE_DAY, invoker);
}

int32 BattlegroundAB::_GetNodeNameId(uint8 node)
{
    switch (node)
    {
        case BG_AB_NODE_STABLES:    return LANG_BG_AB_NODE_STABLES;
        case BG_AB_NODE_BLACKSMITH: return LANG_BG_AB_NODE_BLACKSMITH;
        case BG_AB_NODE_FARM:       return LANG_BG_AB_NODE_FARM;
        case BG_AB_NODE_LUMBER_MILL:return LANG_BG_AB_NODE_LUMBER_MILL;
        case BG_AB_NODE_GOLD_MINE:  return LANG_BG_AB_NODE_GOLD_MINE;
        default:
            ABORT();
    }
    //return 0;
}

void BattlegroundAB::FillInitialWorldStates(WorldPacket& data)
{
    const uint8 plusArray[] = {0, 2, 3, 0, 1};

    // Node icons
    for (uint8 node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
        data << uint32(BG_AB_OP_NODEICONS[node]) << uint32((m_Nodes[node]==0)?1:0);

    // Node occupied states
    for (uint8 node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
        for (uint8 i = 1; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
            data << uint32(BG_AB_OP_NODESTATES[node] + plusArray[i]) << uint32((m_Nodes[node]==i)?1:0);

    // How many bases each team owns
    uint8 ally = 0, horde = 0;
    for (unsigned char m_Node : m_Nodes)
        if( m_Node == BG_AB_NODE_STATUS_ALLY_OCCUPIED )
            ++ally;
        else if( m_Node == BG_AB_NODE_STATUS_HORDE_OCCUPIED )
            ++horde;

    data << uint32(BG_AB_OP_OCCUPIED_BASES_ALLY)  << uint32(ally);
    data << uint32(BG_AB_OP_OCCUPIED_BASES_HORDE) << uint32(horde);

    // Team scores
    data << uint32(BG_AB_OP_RESOURCES_MAX)      << uint32(BG_AB_MAX_TEAM_SCORE);
    data << uint32(BG_AB_OP_RESOURCES_WARNING)  << uint32(BG_AB_WARNING_SCORE);
    data << uint32(BG_AB_OP_RESOURCES_ALLY)     << uint32(m_TeamScores[TEAM_ALLIANCE]);
    data << uint32(BG_AB_OP_RESOURCES_HORDE)    << uint32(m_TeamScores[TEAM_HORDE]);

    // other unknown
    data << uint32(0x745) << uint32(0x2);           // 37 1861 unk
}

void BattlegroundAB::_SendNodeUpdate(uint8 node)
{
    // Send node owner state update to refresh map icons on client
    const uint8 plusArray[] = {0, 2, 3, 0, 1};

    if( m_prevNodes[node] )
        UpdateWorldState(BG_AB_OP_NODESTATES[node] + plusArray[m_prevNodes[node]], 0);
    else
        UpdateWorldState(BG_AB_OP_NODEICONS[node], 0);

    UpdateWorldState(BG_AB_OP_NODESTATES[node] + plusArray[m_Nodes[node]], 1);

    // How many bases each team owns
    uint8 ally = 0, horde = 0;
    for (unsigned char m_Node : m_Nodes)
        if( m_Node == BG_AB_NODE_STATUS_ALLY_OCCUPIED )
            ++ally;
        else if( m_Node == BG_AB_NODE_STATUS_HORDE_OCCUPIED )
            ++horde;

    UpdateWorldState(BG_AB_OP_OCCUPIED_BASES_ALLY, ally);
    UpdateWorldState(BG_AB_OP_OCCUPIED_BASES_HORDE, horde);
}

void BattlegroundAB::_NodeOccupied(uint8 node,Team team)
{
    uint8 capturedNodes = 0;
    for (int m_NodeTimer : m_NodeTimers)
    {
        if( m_Nodes[node] == GetTeamIndexByTeamId(team) + BG_AB_NODE_TYPE_OCCUPIED && !m_NodeTimer)
            ++capturedNodes;
    }
    if(capturedNodes >= 5)
        CastSpellOnTeam(SPELL_AB_QUEST_REWARD_5_BASES, team);
    if(capturedNodes >= 4)
        CastSpellOnTeam(SPELL_AB_QUEST_REWARD_4_BASES, team);
}

void BattlegroundAB::_NodeDeOccupied(uint8 node)
{
    if (node >= BG_AB_DYNAMIC_NODES_COUNT)
        return;

    // Those who are waiting to resurrect at this node are taken to the closest own node's graveyard
    for (auto & itr : m_ReviveQueue)
    {
        Creature* creature = GetBgMap()->GetCreature(itr.first);
        if (!creature)
            continue;
        uint8 event = (sBattlegroundMgr->GetCreatureEventIndex(creature->GetSpawnId())).event1;
        if (event != node)
            continue;
        std::vector<ObjectGuid> ghost_list = itr.second;
        WorldSafeLocsEntry const* closestGrave = nullptr;
        Player* plr;
        for (auto itr2 : ghost_list)
        {
            plr = GetBgMap()->GetPlayer(itr2);
            if (!plr)
                continue;
            if (!closestGrave)
                closestGrave = GetClosestGraveYard(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetTeam());

            plr->TeleportTo(GetMapId(), closestGrave->x, closestGrave->y, closestGrave->z, plr->GetOrientation());
        }
    }
}

/* Invoked if a player used a banner as a gameobject */
void BattlegroundAB::EventPlayerClickedOnFlag(Player* source, GameObject* target_obj)
{
    if( GetStatus() != STATUS_IN_PROGRESS )
        return;

    //prevent taking a base in Spirit form (bug abuse)
    if( source->HasAura(27792) || source->HasAura(27795) )
        return;

    uint8 event = (sBattlegroundMgr->GetGameObjectEventIndex(target_obj->GetSpawnId())).event1;
    if (event >= BG_AB_DYNAMIC_NODES_COUNT)                           // not a node
        return;

    BG_AB_BattlegroundNodes node = BG_AB_BattlegroundNodes(event);
    
    uint8 teamIndex = GetTeamIndexByTeamId(source->GetTeam());

    // Message to chatlog
    char buf[256];
    ChatMsg type = (teamIndex == 0) ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE;

    // Check if player really could use this banner, not cheated
    if (!(m_Nodes[node] == 0 || teamIndex == m_Nodes[node] % 2))
        return;

    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
    uint32 sound = 0;
    // If node is neutral, change to contested
    if( m_Nodes[node] == BG_AB_NODE_TYPE_NEUTRAL )
    {
        UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
        m_prevNodes[node] = m_Nodes[node];
        m_Nodes[node] = teamIndex + 1;
        // burn current neutral banner
        _DelBanner(node, BG_AB_NODE_TYPE_NEUTRAL, 0, source);
        // create new contested banner
        _CreateBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex);
        _SendNodeUpdate(node);
        m_NodeTimers[node] = BG_AB_FLAG_CAPTURING_TIME;

        // FIXME: team and node names not localized
        if (teamIndex == 0)
            SendMessage2ToAll(LANG_BG_AB_NODE_CLAIMED, CHAT_MSG_BG_SYSTEM_ALLIANCE, source, _GetNodeNameId(node), LANG_BG_AB_ALLY);
        else
            SendMessage2ToAll(LANG_BG_AB_NODE_CLAIMED, CHAT_MSG_BG_SYSTEM_HORDE, source, _GetNodeNameId(node), LANG_BG_AB_HORDE);

        sound = SOUND_NODE_CLAIMED;
    }
    // If node is contested
    else if( (m_Nodes[node] == BG_AB_NODE_STATUS_ALLY_CONTESTED) || (m_Nodes[node] == BG_AB_NODE_STATUS_HORDE_CONTESTED) )
    {
        // If last state is NOT occupied, change node to enemy-contested
        if( m_prevNodes[node] < BG_AB_NODE_TYPE_OCCUPIED )
        {
            UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
            m_prevNodes[node] = m_Nodes[node];
            m_Nodes[node] = teamIndex + BG_AB_NODE_TYPE_CONTESTED;
            // burn current contested banner
            _DelBanner(node, BG_AB_NODE_TYPE_CONTESTED, !teamIndex, source);
            // create new contested banner
            _CreateBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex, source);
            _SendNodeUpdate(node);
            m_NodeTimers[node] = BG_AB_FLAG_CAPTURING_TIME;

            // FIXME: node names not localized
            if (teamIndex == TEAM_ALLIANCE)
                SendMessage2ToAll(LANG_BG_AB_NODE_ASSAULTED, CHAT_MSG_BG_SYSTEM_ALLIANCE, source, _GetNodeNameId(node));
            else
                SendMessage2ToAll(LANG_BG_AB_NODE_ASSAULTED, CHAT_MSG_BG_SYSTEM_HORDE, source, _GetNodeNameId(node));
        }
        // If contested, change back to occupied
        else
        {
            UpdatePlayerScore(source, SCORE_BASES_DEFENDED, 1);
            m_prevNodes[node] = m_Nodes[node];
            m_Nodes[node] = teamIndex + BG_AB_NODE_TYPE_OCCUPIED;
            // burn current contested banner
            _DelBanner(node, BG_AB_NODE_TYPE_CONTESTED, !teamIndex, source);
            // create new occupied banner
            _CreateBanner(node, BG_AB_NODE_TYPE_OCCUPIED, teamIndex, source);
            _SendNodeUpdate(node);
            m_NodeTimers[node] = 0;
            _NodeOccupied(node,(teamIndex == 0) ? ALLIANCE:HORDE);
           
            // FIXME: node names not localized
            if (teamIndex == TEAM_ALLIANCE)
                SendMessage2ToAll(LANG_BG_AB_NODE_DEFENDED, CHAT_MSG_BG_SYSTEM_ALLIANCE, source, _GetNodeNameId(node));
            else
                SendMessage2ToAll(LANG_BG_AB_NODE_DEFENDED, CHAT_MSG_BG_SYSTEM_HORDE, source, _GetNodeNameId(node));
        }
        sound = (teamIndex == 0) ? SOUND_NODE_ASSAULTED_ALLIANCE : SOUND_NODE_ASSAULTED_HORDE;
    }
    // If node is occupied, change to enemy-contested
    else
    {
        UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
        m_prevNodes[node] = m_Nodes[node];
        m_Nodes[node] = teamIndex + BG_AB_NODE_TYPE_CONTESTED;
        // burn current occupied banner
        _DelBanner(node, BG_AB_NODE_TYPE_OCCUPIED, !teamIndex, source);
        // create new contested banner
        _CreateBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex, source);
        _SendNodeUpdate(node);
        _NodeDeOccupied(node);
        m_NodeTimers[node] = BG_AB_FLAG_CAPTURING_TIME;

        // FIXME: node names not localized
        if (teamIndex == TEAM_ALLIANCE)
            SendMessage2ToAll(LANG_BG_AB_NODE_ASSAULTED, CHAT_MSG_BG_SYSTEM_ALLIANCE, source, _GetNodeNameId(node));
        else
            SendMessage2ToAll(LANG_BG_AB_NODE_ASSAULTED, CHAT_MSG_BG_SYSTEM_HORDE, source, _GetNodeNameId(node));

        sound = (teamIndex == 0) ? SOUND_NODE_ASSAULTED_ALLIANCE : SOUND_NODE_ASSAULTED_HORDE;
    }
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, type, LANG_UNIVERSAL, source, source, buf);
    SendPacketToAll(&data);
    // If node is occupied again, send "X has taken the Y" msg.
    if (m_Nodes[node] >= BG_AB_NODE_TYPE_OCCUPIED)
    {
        // FIXME: team and node names not localized
        if (teamIndex == TEAM_ALLIANCE)
            SendMessage2ToAll(LANG_BG_AB_NODE_TAKEN, CHAT_MSG_BG_SYSTEM_ALLIANCE, nullptr, LANG_BG_AB_ALLY, _GetNodeNameId(node));
        else
            SendMessage2ToAll(LANG_BG_AB_NODE_TAKEN, CHAT_MSG_BG_SYSTEM_HORDE, nullptr, LANG_BG_AB_HORDE, _GetNodeNameId(node));
    }
    PlaySoundToAll(sound);
}

void BattlegroundAB::Reset()
{
    Battleground::Reset();

    m_TeamScores[TEAM_ALLIANCE] = 0;
    m_TeamScores[TEAM_HORDE] = 0;
    m_lastTick[TEAM_ALLIANCE] = 0;
    m_lastTick[TEAM_HORDE] = 0;
    m_HonorScoreTics[TEAM_ALLIANCE] = 0;
    m_HonorScoreTics[TEAM_HORDE] = 0;
    m_ReputationScoreTics[TEAM_ALLIANCE] = 0;
    m_ReputationScoreTics[TEAM_HORDE] = 0;
    m_IsInformedNearVictory = false;
    for (uint8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        m_Nodes[i] = 0;
        m_prevNodes[i] = 0;
        m_NodeTimers[i] = 0;
        m_BannerTimers[i].timer = 0;

        // all nodes owned by neutral team at beginning
        m_ActiveEvents[i] = BG_AB_NODE_TYPE_NEUTRAL;
    }
}

bool BattlegroundAB::SetupBattleground()
{
    //buffs
    for (int i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        if(    !AddObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + 3 * i, Buff_Entries[0], BG_AB_BuffPositions[i][0], BG_AB_BuffPositions[i][1], BG_AB_BuffPositions[i][2], BG_AB_BuffPositions[i][3], 0, 0, sin(BG_AB_BuffPositions[i][3]/2), cos(BG_AB_BuffPositions[i][3]/2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + 3 * i + 1, Buff_Entries[1], BG_AB_BuffPositions[i][0], BG_AB_BuffPositions[i][1], BG_AB_BuffPositions[i][2], BG_AB_BuffPositions[i][3], 0, 0, sin(BG_AB_BuffPositions[i][3]/2), cos(BG_AB_BuffPositions[i][3]/2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + 3 * i + 2, Buff_Entries[2], BG_AB_BuffPositions[i][0], BG_AB_BuffPositions[i][1], BG_AB_BuffPositions[i][2], BG_AB_BuffPositions[i][3], 0, 0, sin(BG_AB_BuffPositions[i][3]/2), cos(BG_AB_BuffPositions[i][3]/2), RESPAWN_ONE_DAY)
            )
            TC_LOG_ERROR("battleground","BatteGroundAB: Failed to spawn buff object!");
    }

    return true;
}

WorldSafeLocsEntry const* BattlegroundAB::GetClosestGraveYard(float x, float y, float /*z*/, uint32 team)
{
    uint8 teamIndex = GetTeamIndexByTeamId(team);

    // Is there any occupied node for this team?
    std::vector<uint8> nodes;
    for (uint8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
        if( m_Nodes[i] == teamIndex + 3 )
            nodes.push_back(i);

    WorldSafeLocsEntry const* good_entry = nullptr;
    // If so, select the closest node to place ghost on
    if (!nodes.empty())
    {
        float mindist = 999999.0f;
        for (unsigned char node : nodes)
        {
            WorldSafeLocsEntry const*entry = sWorldSafeLocsStore.LookupEntry( BG_AB_GraveyardIds[node] );
            if( !entry )
                continue;
            float dist = (entry->x - x)*(entry->x - x)+(entry->y - y)*(entry->y - y);
            if( mindist > dist )
            {
                mindist = dist;
                good_entry = entry;
            }
        }
        nodes.clear();
    }
    // If not, place ghost on starting location
    if (!good_entry)
        good_entry = sWorldSafeLocsStore.LookupEntry( BG_AB_GraveyardIds[teamIndex+5] );

    return good_entry;
}

void BattlegroundAB::UpdatePlayerScore(Player *Source, uint32 type, uint32 value)
{
    auto itr = PlayerScores.find(Source->GetGUID());

    if (itr == PlayerScores.end())                         // player not found...
        return;

    switch(type)
    {
        case SCORE_BASES_ASSAULTED:
            ((BattlegroundABScore*)itr->second)->BasesAssaulted += value;
            break;
        case SCORE_BASES_DEFENDED:
            ((BattlegroundABScore*)itr->second)->BasesDefended += value;
            break;
        default:
            Battleground::UpdatePlayerScore(Source,type,value);
            break;
    }
}

