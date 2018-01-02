
#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundNA.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"

BattlegroundNA::BattlegroundNA()
{
    BgObjects.resize(BG_NA_OBJECT_MAX);
}

BattlegroundNA::~BattlegroundNA()
{

}

void BattlegroundNA::Update(time_t diff)
{
    Battleground::Update(diff);

    // after bg start we get there
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
            for(uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_4; i++)
                SpawnBGObject(i, RESPAWN_IMMEDIATELY);

            SetStartDelayTime(START_DELAY1);
            SendMessageToAll(LANG_ARENA_ONE_MINUTE);
        }
        // After 30 seconds, warning is signalled
        else if (GetStartDelayTime() <= START_DELAY2 && !(m_Events & BG_STARTING_EVENT_3))
        {
            m_Events |= BG_STARTING_EVENT_3;
            SendMessageToAll(LANG_ARENA_THIRTY_SECONDS);
        }
        // After 15 seconds, warning is signalled
        else if (GetStartDelayTime() <= START_DELAY3 && !(m_Events & BG_STARTING_EVENT_4))
        {
            m_Events |= BG_STARTING_EVENT_4;
            SendMessageToAll(LANG_ARENA_FIFTEEN_SECONDS);
        }
        // delay expired (1 minute)
        else if (GetStartDelayTime() <= 0 && !(m_Events & 0x10))
        {
            m_Events |= 0x10;

            for(uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_2; i++)
                DoorOpen(i);

            for(uint32 i = BG_NA_OBJECT_BUFF_1; i <= BG_NA_OBJECT_BUFF_2; i++)
                SpawnBGObject(i, 60);

            SendMessageToAll(LANG_ARENA_BEGUN);
            SetStatus(STATUS_IN_PROGRESS);
            SetStartDelayTime(0);

            for(const auto & itr : GetPlayers())
                if(Player *plr = sObjectMgr->GetPlayer(itr.first))
                    plr->RemoveAurasDueToSpell(SPELL_ARENA_PREPARATION);

            if(!GetPlayersCountByTeam(ALLIANCE) && GetPlayersCountByTeam(HORDE))
                EndBattleground(HORDE);
            else if(GetPlayersCountByTeam(ALLIANCE) && !GetPlayersCountByTeam(HORDE))
                EndBattleground(ALLIANCE);
        }
    }

    /*if(GetStatus() == STATUS_IN_PROGRESS)
    {
        // update something
    }*/
}

void BattlegroundNA::AddPlayer(Player *plr)
{
    Battleground::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    auto  sc = new BattlegroundNAScore;

    PlayerScores[plr->GetGUID()] = sc;

    UpdateWorldState(0xa0f, GetAlivePlayersCountByTeam(ALLIANCE));
    UpdateWorldState(0xa10, GetAlivePlayersCountByTeam(HORDE));
}

void BattlegroundNA::RemovePlayer(Player* /*plr*/, ObjectGuid /*guid*/)
{
    if(GetStatus() == STATUS_WAIT_LEAVE)
        return;

    UpdateWorldState(0xa0f, GetAlivePlayersCountByTeam(ALLIANCE));
    UpdateWorldState(0xa10, GetAlivePlayersCountByTeam(HORDE));

    if (GetStatus() != STATUS_WAIT_JOIN) {
        if(!GetAlivePlayersCountByTeam(ALLIANCE) && GetPlayersCountByTeam(HORDE))
            EndBattleground(HORDE);
        else if(GetPlayersCountByTeam(ALLIANCE) && !GetAlivePlayersCountByTeam(HORDE))
            EndBattleground(ALLIANCE);
    }
}

void BattlegroundNA::HandleKillPlayer(Player *player, Player *killer)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    if(!killer)
    {
        TC_LOG_ERROR("FIXME","BattlegroundNA: Killer player not found");
        return;
    }

    Battleground::HandleKillPlayer(player,killer);

    UpdateWorldState(0xa0f, GetAlivePlayersCountByTeam(ALLIANCE));
    UpdateWorldState(0xa10, GetAlivePlayersCountByTeam(HORDE));

    if(!GetAlivePlayersCountByTeam(ALLIANCE))
    {
        // all opponents killed
        EndBattleground(HORDE);
    }
    else if(!GetAlivePlayersCountByTeam(HORDE))
    {
        // all opponents killed
        EndBattleground(ALLIANCE);
    }
}

bool BattlegroundNA::HandlePlayerUnderMap(Player *player)
{
    player->TeleportTo(GetMapId(),4055.504395,2919.660645,13.611241,player->GetOrientation(),false);
    return true;
}

void BattlegroundNA::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    //uint32 SpellId = 0;
    //ObjectGuid buff_guid = 0;
    switch(Trigger)
    {
        case 4536:                                          // buff trigger?
        case 4537:                                          // buff trigger?
            break;
        default:
            TC_LOG_ERROR("FIXME","WARNING: Unhandled AreaTrigger in Battleground: %u", Trigger);
            Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
    }

    //if(buff_guid)
    //    HandleTriggerBuff(buff_guid,Source);
}

void BattlegroundNA::FillInitialWorldStates(WorldPacket &data)
{
    data << uint32(0xa0f) << uint32(GetAlivePlayersCountByTeam(ALLIANCE));           // 7
    data << uint32(0xa10) << uint32(GetAlivePlayersCountByTeam(HORDE));           // 8
    data << uint32(0xa11) << uint32(1);           // 9
}

void BattlegroundNA::ResetBGSubclass()
{

}

bool BattlegroundNA::SetupBattleground()
{
    // gates
    if(    !AddObject(BG_NA_OBJECT_DOOR_1, BG_NA_OBJECT_TYPE_DOOR_1, 4031.854, 2966.833, 12.6462, -2.648788, 0, 0, 0.9697962, -0.2439165, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_DOOR_2, BG_NA_OBJECT_TYPE_DOOR_2, 4081.179, 2874.97, 12.39171, 0.4928045, 0, 0, 0.2439165, 0.9697962, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_DOOR_3, BG_NA_OBJECT_TYPE_DOOR_3, 4023.709, 2981.777, 10.70117, -2.648788, 0, 0, 0.9697962, -0.2439165, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_DOOR_4, BG_NA_OBJECT_TYPE_DOOR_4, 4090.064, 2858.438, 10.23631, 0.4928045, 0, 0, 0.2439165, 0.9697962, RESPAWN_IMMEDIATELY)
    // buffs
        || !AddObject(BG_NA_OBJECT_BUFF_1, BG_NA_OBJECT_TYPE_BUFF_1, 4009.189941, 2895.250000, 13.052700, -1.448624, 0, 0, 0.6626201, -0.7489557, 120)
        || !AddObject(BG_NA_OBJECT_BUFF_2, BG_NA_OBJECT_TYPE_BUFF_2, 4103.330078, 2946.350098, 13.051300, -0.06981307, 0, 0, 0.03489945, -0.9993908, 120))
    {
        TC_LOG_ERROR("battleground","BatteGroundNA: Failed to spawn some object!");
        return false;
    }

    return true;
}

/*
20:12:14 id:036668 [S2C] SMSG_INIT_WORLD_STATES (706 = 0x02C2) len: 86
0000: 2f 02 00 00 72 0e 00 00 00 00 00 00 09 00 11 0a  |  /...r...........
0010: 00 00 01 00 00 00 0f 0a 00 00 00 00 00 00 10 0a  |  ................
0020: 00 00 00 00 00 00 d4 08 00 00 00 00 00 00 d8 08  |  ................
0030: 00 00 00 00 00 00 d7 08 00 00 00 00 00 00 d6 08  |  ................
0040: 00 00 00 00 00 00 d5 08 00 00 00 00 00 00 d3 08  |  ................
0050: 00 00 00 00 00 00                                |  ......
*/

