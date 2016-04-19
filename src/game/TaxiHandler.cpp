
#include "Common.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "WaypointMovementGenerator.h"

void WorldSession::HandleTaxiNodeStatusQueryOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_TAXINODE_STATUS_QUERY");

    uint64 guid;

    recvData >> guid;
    SendTaxiStatus(guid);
}

void WorldSession::SendTaxiStatus(uint64 guid)
{
    // cheating checks
    Creature* unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WorldSession::SendTaxiStatus - Unit (GUID: %u) not found.", uint32(GUID_LOPART(guid)));
        return;
    }

    uint32 curloc = sObjectMgr->GetNearestTaxiNode(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetMapId(), GetPlayer()->GetTeam());

    // not found nearest
    if (curloc == 0)
        return;

    TC_LOG_DEBUG("network", "WORLD: current location %u ", curloc);

    WorldPacket data(SMSG_TAXINODE_STATUS, 9);
    data << guid;
    data << uint8(GetPlayer()->m_taxi.IsTaximaskNodeKnown(curloc) ? 1 : 0);
    SendPacket(&data);
    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_TAXINODE_STATUS");
}

void WorldSession::HandleTaxiQueryAvailableNodes(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_TAXIQUERYAVAILABLENODES");

    uint64 guid;
    recvData >> guid;

    // cheating checks
    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleTaxiQueryAvailableNodes - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // unknown taxi node case
    if (SendLearnNewTaxiNode(unit))
        return;

    // known taxi node case
    SendTaxiMenu(unit);
}

void WorldSession::SendTaxiMenu(Creature* unit)
{
    // find current node
    uint32 curloc = sObjectMgr->GetNearestTaxiNode(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetMapId(), GetPlayer()->GetTeam());

    if (curloc == 0)
        return;

    bool lastTaxiCheaterState = GetPlayer()->IsTaxiCheater();
    if (unit->GetEntry() == 29480) GetPlayer()->SetTaxiCheater(true); // Grimwing in Ebon Hold, special case. NOTE: Not perfect, Zul'Aman should not be included according to WoWhead, and I think taxicheat includes it.

    TC_LOG_DEBUG("network", "WORLD: CMSG_TAXINODE_STATUS_QUERY %u ", curloc);

    WorldPacket data(SMSG_SHOWTAXINODES, (4 + 8 + 4 + 8 * 4));
    data << uint32(1);
    data << uint64(unit->GetGUID());
    data << uint32(curloc);
    GetPlayer()->m_taxi.AppendTaximaskTo(data, GetPlayer()->IsTaxiCheater());
    SendPacket(&data);

    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_SHOWTAXINODES");

    GetPlayer()->SetTaxiCheater(lastTaxiCheaterState);
}

void WorldSession::SendDoFlight(uint32 mountDisplayId, uint32 path, uint32 pathNode)
{
    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    while (GetPlayer()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
        GetPlayer()->GetMotionMaster()->MovementExpired(false);

    if (mountDisplayId)
        GetPlayer()->Mount(mountDisplayId);

    GetPlayer()->GetMotionMaster()->MoveTaxiFlight(path, pathNode);
}

bool WorldSession::SendLearnNewTaxiNode(Creature* unit)
{
    // find current node
    uint32 curloc = sObjectMgr->GetNearestTaxiNode(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetMapId(), GetPlayer()->GetTeam());

    if (curloc == 0)
        return true;                                        // `true` send to avoid WorldSession::SendTaxiMenu call with one more curlock seartch with same false result.

    if (GetPlayer()->m_taxi.SetTaximaskNode(curloc))
    {
        WorldPacket msg(SMSG_NEW_TAXI_PATH, 0);
        SendPacket(&msg);

        WorldPacket update(SMSG_TAXINODE_STATUS, 9);
        update << uint64(unit->GetGUID());
        update << uint8(1);
        SendPacket(&update);

        return true;
    }
    else
        return false;
}

void WorldSession::SendDiscoverNewTaxiNode(uint32 nodeid)
{
    if (GetPlayer()->m_taxi.SetTaximaskNode(nodeid))
    {
        WorldPacket msg(SMSG_NEW_TAXI_PATH, 0);
        SendPacket(&msg);
    }
}

void WorldSession::HandleActivateTaxiExpressOpcode (WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_ACTIVATETAXIEXPRESS");

    uint64 guid;
    uint32 node_count;
    uint32 _totalcost; //unused

    recvData >> guid >> _totalcost >> node_count;

    Creature* npc = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!npc)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleActivateTaxiExpressOpcode - Unit (GUID: %u) not found or you can't interact with it.", uint32(GUID_LOPART(guid)));
        return;
    }
    std::vector<uint32> nodes;

    for (uint32 i = 0; i < node_count; ++i)
    {
        uint32 node;
        recvData >> node;

        if (!GetPlayer()->m_taxi.IsTaximaskNodeKnown(node) && !GetPlayer()->IsTaxiCheater())
        {
            SendActivateTaxiReply(ERR_TAXINOTVISITED);
            recvData.rfinish();
            return;
        }

        nodes.push_back(node);
    }

    if (nodes.empty())
        return;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_ACTIVATETAXIEXPRESS from %d to %d", nodes.front(), nodes.back());

    GetPlayer()->ActivateTaxiPathTo(nodes, npc);
}

void WorldSession::HandleMoveSplineDoneOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_MOVE_SPLINE_DONE");

    /* LK uint64 guid; // used only for proper packet read
    recvData.readPackGUID(guid); */

    MovementInfo movementInfo;                              // used only for proper packet read
    recvData >> movementInfo;
    GetPlayer()->m_anti_lastmovetime = movementInfo.time;

   //LK? recvData.read_skip<uint32>();                          // spline id

    // in taxi flight packet received in 2 case:
    // 1) end taxi path in far (multi-node) flight
    // 2) switch from one map to other in case multim-map taxi path
    // we need process only (1)

    uint32 curDest = GetPlayer()->m_taxi.GetTaxiDestination();
    if (curDest)
    {
        TaxiNodesEntry const* curDestNode = sTaxiNodesStore.LookupEntry(curDest);

        // far teleport case
        if (curDestNode && curDestNode->map_id != GetPlayer()->GetMapId())
        {
            if (GetPlayer()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
            {
                // short preparations to continue flight
                FlightPathMovementGenerator* flight = (FlightPathMovementGenerator*)(GetPlayer()->GetMotionMaster()->top());

                flight->SetCurrentNodeAfterTeleport();
                TaxiPathNodeEntry const* node = flight->GetPath()[flight->GetCurrentNode()];
                flight->SkipCurrentNode();

                GetPlayer()->TeleportTo(curDestNode->map_id, node->LocX, node->LocY, node->LocZ, GetPlayer()->GetOrientation());
            }
        }

        return;
    }

    // at this point only 1 node is expected (final destination)
    if (GetPlayer()->m_taxi.GetPath().size() != 1)
        return;

    GetPlayer()->CleanupAfterTaxiFlight();
    GetPlayer()->SetFallInformation(0, GetPlayer()->GetPositionZ());
    if (GetPlayer()->pvpInfo.inHostileArea)
        GetPlayer()->CastSpell(GetPlayer(), 2479, true);
}

void WorldSession::HandleActivateTaxiOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_ACTIVATETAXI");

    uint64 guid;
    std::vector<uint32> nodes;
    nodes.resize(2);

    recvData >> guid >> nodes[0] >> nodes[1];
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_ACTIVATETAXI from %d to %d", nodes[0], nodes[1]);
    Creature* npc = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!npc)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleActivateTaxiOpcode - Unit (GUID: %u) not found or you can't interact with it.", uint32(GUID_LOPART(guid)));
        return;
    }

    if (!GetPlayer()->IsTaxiCheater())
    {
        if (!GetPlayer()->m_taxi.IsTaximaskNodeKnown(nodes[0]) || !GetPlayer()->m_taxi.IsTaximaskNodeKnown(nodes[1]))
        {
            SendActivateTaxiReply(ERR_TAXINOTVISITED);
            return;
        }
    }

    GetPlayer()->ActivateTaxiPathTo(nodes, npc);
}

void WorldSession::SendActivateTaxiReply(ActivateTaxiReply reply)
{
    WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
    data << uint32(reply);
    SendPacket(&data);

    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_ACTIVATETAXIREPLY");
}
