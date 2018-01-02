
#include "Common.h"
#include "Language.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "ArenaTeam.h"
#include "MapManager.h"
#include "GossipDef.h"
#include "SocialMgr.h"
#include "CharacterCache.h"

/*enum PetitionType // dbc data
{
    PETITION_TYPE_GUILD      = 1,
    PETITION_TYPE_ARENA_TEAM = 3
};*/

// Charters ID in item_template
#define GUILD_CHARTER               5863
#define GUILD_CHARTER_COST          1000                    // 10 S
#define ARENA_TEAM_CHARTER_2v2      23560
#define ARENA_TEAM_CHARTER_2v2_COST 800000                  // 80 G
#define ARENA_TEAM_CHARTER_3v3      23561
#define ARENA_TEAM_CHARTER_3v3_COST 1200000                 // 120 G
#define ARENA_TEAM_CHARTER_5v5      23562
#define ARENA_TEAM_CHARTER_5v5_COST 2000000                 // 200 G

void WorldSession::HandlePetitionBuyOpcode(WorldPacket & recvData)
{
    ObjectGuid guidNPC;
    ObjectGuid unk1, unk3, unk4, unk5, unk6, unk7;
    uint32 unk2;
    std::string name;
    uint16 unk8;
    uint8  unk9;
    uint32 unk10;                                           // selected index
    uint32 unk11;
    recvData >> guidNPC;                                   // NPC GUID
    recvData >> unk1;                                      // 0
    recvData >> unk2;                                      // 0
    recvData >> name;                                      // name
    recvData >> unk3;                                      // 0
    recvData >> unk4;                                      // 0
    recvData >> unk5;                                      // 0
    recvData >> unk6;                                      // 0
    recvData >> unk7;                                      // 0
    recvData >> unk8;                                      // 0
    recvData >> unk9;                                      // 0
    recvData >> unk10;                                     // index
    recvData >> unk11;                                     // 0

    // prevent cheating
    Creature *pCreature = GetPlayer()->GetNPCIfCanInteractWith(guidNPC, UNIT_NPC_FLAG_PETITIONER);
    if (!pCreature)
    {
        TC_LOG_ERROR("network","WORLD: HandlePetitionBuyOpcode - Unit (GUID: %u) not found or you can't interact with him.", guidNPC.GetCounter());
        return;
    }

    // remove fake death
    if(GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    uint32 charterid = 0;
    uint32 cost = 0;
    uint32 type = 0;
    if(pCreature->IsTabardDesigner())
    {
        // if tabard designer, then trying to buy a guild charter.
        // do not let if already in guild.
        if(_player->GetGuildId())
            return;

        charterid = GUILD_CHARTER;
        cost = GUILD_CHARTER_COST;
        type = 9;
    }
    else
    {
        // TODO: find correct opcode
        if(_player->GetLevel() < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            SendNotification(LANG_ARENA_ONE_TOOLOW, sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL));
            return;
        }

        switch(unk10)
        {
            case 1:
                charterid = ARENA_TEAM_CHARTER_2v2;
                cost = ARENA_TEAM_CHARTER_2v2_COST;
                type = 2;                                   // 2v2
                break;
            case 2:
                charterid = ARENA_TEAM_CHARTER_3v3;
                cost = ARENA_TEAM_CHARTER_3v3_COST;
                type = 3;                                   // 3v3
                break;
            case 3:
                charterid = ARENA_TEAM_CHARTER_5v5;
                cost = ARENA_TEAM_CHARTER_5v5_COST;
                type = 5;                                   // 5v5
                break;
            default:
                TC_LOG_ERROR("FIXME","unknown selection at buy petition: %u", unk10);
                return;
        }

        if(_player->GetArenaTeamId(unk10-1))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, name, "", ERR_ALREADY_IN_ARENA_TEAM);
            return;
        }
    }

    if(type == 9)
    {
        if(sObjectMgr->GetGuildByName(name))
        {
            SendGuildCommandResult(GUILD_CREATE_S, name, GUILD_NAME_EXISTS);
            return;
        }
        if(sObjectMgr->IsReservedName(name) || !ObjectMgr::IsValidCharterName(name))
        {
            SendGuildCommandResult(GUILD_CREATE_S, name, GUILD_NAME_INVALID);
            return;
        }
    }
    else
    {
        if(sObjectMgr->GetArenaTeamByName(name))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, name, "", ERR_ARENA_TEAM_NAME_EXISTS_S);
            return;
        }
        if(sObjectMgr->IsReservedName(name) || !ObjectMgr::IsValidCharterName(name))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, name, "", ERR_ARENA_TEAM_NAME_INVALID);
            return;
        }
    }

    ItemTemplate const *pProto = sObjectMgr->GetItemTemplate(charterid);
    if(!pProto)
    {
        _player->SendBuyError(BUY_ERR_CANT_FIND_ITEM, nullptr, charterid, 0);
        return;
    }

    if(_player->GetMoney() < cost)
    {                                                       //player hasn't got enough money
        _player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, charterid, 0);
        return;
    }

    ItemPosCountVec dest;
    uint8 msg = _player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, charterid, pProto->BuyCount );
    if(msg != EQUIP_ERR_OK)
    {
        _player->SendBuyError(msg, pCreature, charterid, 0);
        return;
    }

    _player->ModifyMoney(-(int32)cost);
    Item *charter = _player->StoreNewItem(dest, charterid, true);
    if(!charter)
        return;

    charter->SetUInt32Value(ITEM_FIELD_ENCHANTMENT, charter->GetGUID().GetCounter());
    // ITEM_FIELD_ENCHANTMENT is guild/arenateam id
    // ITEM_FIELD_ENCHANTMENT+1 is current signatures count (showed on item)
    charter->SetState(ITEM_CHANGED, _player);
    _player->SendNewItem(charter, 1, true, false);

    // a petition is invalid, if both the owner and the type matches
    // we checked above, if this player is in an arenateam, so this must be
    // datacorruption
    QueryResult result = CharacterDatabase.PQuery("SELECT petitionguid FROM petition WHERE ownerguid = '%u'  AND type = '%u'", _player->GetGUID().GetCounter(), type);

    std::ostringstream ssInvalidPetitionGUIDs;

    if (result)
    {

        do
        {
            Field *fields = result->Fetch();
            ssInvalidPetitionGUIDs << "'" << fields[0].GetUInt32() << "' , ";
        } while (result->NextRow());
    }

    // delete petitions with the same guid as this one
    ssInvalidPetitionGUIDs << "'" << charter->GetGUID().GetCounter() << "'";

    TC_LOG_ERROR("FIXME","Invalid petition GUIDs: %s", ssInvalidPetitionGUIDs.str().c_str());
    CharacterDatabase.EscapeString(name);
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM petition WHERE petitionguid IN ( %s )",  ssInvalidPetitionGUIDs.str().c_str());
    trans->PAppend("DELETE FROM petition_sign WHERE petitionguid IN ( %s )", ssInvalidPetitionGUIDs.str().c_str());
    trans->PAppend("INSERT INTO petition (ownerguid, petitionguid, name, type) VALUES ('%u', '%u', '%s', '%u')",
        _player->GetGUID().GetCounter(), charter->GetGUID().GetCounter(), name.c_str(), type);
    CharacterDatabase.CommitTransaction(trans);
}

void WorldSession::HandlePetitionShowSignOpcode(WorldPacket & recvData)
{
    uint8 signs = 0;
    ObjectGuid petitionguid;
    recvData >> petitionguid;                              // petition guid

    // solve (possible) some strange compile problems with explicit use petitionguid.GetCounter() at some GCC versions (wrong code optimization in compiler?)
    ObjectGuid::LowType petitionguid_low = petitionguid.GetCounter();

    QueryResult result = CharacterDatabase.PQuery("SELECT type FROM petition WHERE petitionguid = '%u'", petitionguid_low);
    if(!result)
    {
        TC_LOG_ERROR("FIXME","Table `petition` is empty.");
        return;
    }
    Field *fields = result->Fetch();
    uint32 type = fields[0].GetUInt32();

    // if guild petition and has guild => error, return;
    if(type==9 && _player->GetGuildId())
        return;

    result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", petitionguid_low);

    // result==NULL also correct in case no sign yet
    if(result)
        signs = result->GetRowCount();

    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, (8+8+4+1+signs*12));
    data << petitionguid;                                   // petition guid
    data << _player->GetGUID();                             // owner guid
    data << petitionguid_low;                               // guild guid (in Trinity always same as petitionguid.GetCounter()
    data << signs;                                          // sign's count

    for(uint8 i = 1; i <= signs; i++)
    {
        Field *fields = result->Fetch();
        ObjectGuid plguid = ObjectGuid(fields[0].GetUInt64());

        data << plguid;                                     // Player GUID
        data << (uint32)0;                                  // there 0 ...

        result->NextRow();
    }
    SendPacket(&data);
}

void WorldSession::HandlePetitionQueryOpcode(WorldPacket & recvData)
{
    ObjectGuid::LowType guildguid;
    ObjectGuid petitionguid;
    recvData >> guildguid;                                 // in Trinity always same as petitionguid.GetCounter()
    recvData >> petitionguid;                              // petition guid

    SendPetitionQueryOpcode(petitionguid);
}

void WorldSession::SendStableResult(uint8 res)
{
    WorldPacket data(SMSG_STABLE_RESULT, 1);
    data << uint8(res);
    SendPacket(&data);
}

void WorldSession::SendPetitionQueryOpcode(ObjectGuid petitionguid)
{
    ObjectGuid ownerguid = ObjectGuid::Empty;
    uint32 type;
    std::string name = "NO_NAME_FOR_GUID";
    //uint8 signs = 0; //not used

    QueryResult result = CharacterDatabase.PQuery(
        "SELECT ownerguid, name, "
        "  (SELECT COUNT(playerguid) FROM petition_sign WHERE petition_sign.petitionguid = '%u') AS signs, "
        "  type "
        "FROM petition WHERE petitionguid = '%u'", petitionguid.GetCounter(), petitionguid.GetCounter());

    if(result)
    {
        Field* fields = result->Fetch();
        ownerguid = ObjectGuid(HighGuid::Player, fields[0].GetUInt32());
        name      = fields[1].GetString();
        //signs     = fields[2].GetUInt8(); //not used
        type      = fields[3].GetUInt32();
    }
    else
    {
        TC_LOG_ERROR("FIXME","CMSG_PETITION_QUERY failed for petition (GUID: %u)", petitionguid.GetCounter());
        return;
    }

    WorldPacket data(SMSG_PETITION_QUERY_RESPONSE, (4+8+name.size()+1+1+4*13));
    data << petitionguid.GetCounter();                      // guild/team guid (in Trinity always same as GUID_LOPART(petition guid)
    data << ownerguid;                                      // charter owner guid
    data << name;                                           // name (guild/arena team)
    data << uint8(0);                                       // 1
    if(type == 9)
    {
        data << uint32(9);
        data << uint32(9);
        data << uint32(0);                                  // bypass client - side limitation, a different value is needed here for each petition
    }
    else
    {
        data << type-1;
        data << type-1;
        data << type;                                       // bypass client - side limitation, a different value is needed here for each petition
    }
    data << uint32(0);                                      // 5
    data << uint32(0);                                      // 6
    data << uint32(0);                                      // 7
    data << uint32(0);                                      // 8
    data << uint16(0);                                      // 9 2 bytes field
    data << uint32(0);                                      // 10
    data << uint32(0);                                      // 11
    data << uint32(0);                                      // 13 count of next strings?
    data << uint32(0);                                      // 14
    if(type == 9)
        data << uint32(0);                                  // 15 0 - guild, 1 - arena team
    else
        data << uint32(1);
    SendPacket(&data);
}

void WorldSession::HandlePetitionRenameOpcode(WorldPacket & recvData)
{
    ObjectGuid petitionguid;
    uint32 type;
    std::string newname;

    recvData >> petitionguid;                              // guid
    recvData >> newname;                                   // new name

    Item *item = _player->GetItemByGuid(petitionguid);
    if(!item)
        return;

    QueryResult result = CharacterDatabase.PQuery("SELECT type FROM petition WHERE petitionguid = '%u'", petitionguid.GetCounter());

    if(result)
    {
        Field* fields = result->Fetch();
        type = fields[0].GetUInt32();
    }
    else
    {
        TC_LOG_ERROR("FIXME","CMSG_PETITION_QUERY failed for petition (GUID: %u)", petitionguid.GetCounter());
        return;
    }

    if(type == 9)
    {
        if(sObjectMgr->GetGuildByName(newname))
        {
            SendGuildCommandResult(GUILD_CREATE_S, newname, GUILD_NAME_EXISTS);
            return;
        }
        if(sObjectMgr->IsReservedName(newname) || !ObjectMgr::IsValidCharterName(newname))
        {
            SendGuildCommandResult(GUILD_CREATE_S, newname, GUILD_NAME_INVALID);
            return;
        }
    }
    else
    {
        if(sObjectMgr->GetArenaTeamByName(newname))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, newname, "", ERR_ARENA_TEAM_NAME_EXISTS_S);
            return;
        }
        if(sObjectMgr->IsReservedName(newname) || !ObjectMgr::IsValidCharterName(newname))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, newname, "", ERR_ARENA_TEAM_NAME_INVALID);
            return;
        }
    }

    std::string db_newname = newname;
    CharacterDatabase.EscapeString(db_newname);
    CharacterDatabase.PExecute("UPDATE petition SET name = '%s' WHERE petitionguid = '%u'",
        db_newname.c_str(), petitionguid.GetCounter());

    WorldPacket data(MSG_PETITION_RENAME, (8+newname.size()+1));
    data << petitionguid;
    data << newname;
    SendPacket(&data);
}

void WorldSession::HandlePetitionSignOpcode(WorldPacket & recvData)
{
    Field *fields;
    ObjectGuid petitionguid;
    uint8 unk;
    recvData >> petitionguid;                              // petition guid
    recvData >> unk;

    QueryResult result = CharacterDatabase.PQuery(
        "SELECT ownerguid, "
        "  (SELECT COUNT(playerguid) FROM petition_sign WHERE petition_sign.petitionguid = '%u') AS signs, "
        "  type "
        "FROM petition WHERE petitionguid = '%u'", petitionguid.GetCounter(), petitionguid.GetCounter());

    if(!result)
    {
        TC_LOG_ERROR("FIXME","Table `petition` is empty.");
        return;
    }

    fields = result->Fetch();
    ObjectGuid ownerguid = ObjectGuid(HighGuid::Player, fields[0].GetUInt32());
    uint8 signs = fields[1].GetUInt8();
    uint32 type = fields[2].GetUInt32();

    ObjectGuid::LowType plguidlo = _player->GetGUID().GetCounter();
    if(ownerguid.GetCounter() == plguidlo)
        return;

    // not let enemies sign guild charter
    if(!sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD) && GetPlayer()->GetTeam() != sCharacterCache->GetCharacterTeamByGuid(ownerguid))
    {
        if(type != 9)
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", "", ERR_ARENA_TEAM_NOT_ALLIED);
        else
            SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_NOT_ALLIED);
        return;
    }

    if(type != 9)
    {
        if(_player->GetLevel() < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, "", _player->GetName(), ERR_ARENA_TEAM_PLAYER_TO_LOW);
            return;
        }

        uint8 slot = ArenaTeam::GetSlotByType(type);
        if(slot >= MAX_ARENA_SLOT)
            return;

        if(_player->GetArenaTeamId(slot))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", _player->GetName(), ERR_ALREADY_IN_ARENA_TEAM_S);
            return;
        }

        if(_player->GetArenaTeamIdInvited())
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", _player->GetName(), ERR_ALREADY_INVITED_TO_ARENA_TEAM_S);
            return;
        }
    }
    else
    {
        if(_player->GetGuildId())
        {
            SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ALREADY_IN_GUILD);
            return;
        }
        if(_player->GetGuildIdInvited())
        {
            SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ALREADY_INVITED_TO_GUILD);
            return;
        }
    }

    if(++signs > type)                                        // client signs maximum
        return;

    //client doesn't allow to sign petition two times by one character, but not check sign by another character from same account
    //not allow sign another player from already sign player account
    result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE player_account = '%u' AND petitionguid = '%u'", GetAccountId(), petitionguid.GetCounter());

    if(result)
    {
        WorldPacket data(SMSG_PETITION_SIGN_RESULTS, (8+8+4));
        data << petitionguid;
        data << _player->GetGUID();
        data << (uint32)PETITION_SIGN_ALREADY_SIGNED;

        // close at signer side
        SendPacket(&data);

        // update for owner if online
        if(Player *owner = sObjectMgr->GetPlayer(ownerguid))
            owner->SendDirectMessage(&data);
        return;
    }

    CharacterDatabase.PExecute("INSERT INTO petition_sign (ownerguid,petitionguid, playerguid, player_account) VALUES ('%u', '%u', '%u','%u')", ownerguid.GetCounter(),petitionguid.GetCounter(), plguidlo,GetAccountId());

    WorldPacket data(SMSG_PETITION_SIGN_RESULTS, (8+8+4));
    data << petitionguid;
    data << _player->GetGUID();
    data << (uint32)PETITION_SIGN_OK;

    // close at signer side
    SendPacket(&data);

    // update signs count on charter, required testing...
    //Item *item = _player->GetItemByGuid(petitionguid));
    //if(item)
    //    item->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+1, signs);

    // update for owner if online
    if(Player *owner = sObjectMgr->GetPlayer(ownerguid))
        owner->SendDirectMessage(&data);
}

void WorldSession::HandlePetitionDeclineOpcode(WorldPacket & recvData)
{
    ObjectGuid petitionguid;
    ObjectGuid ownerguid;
    recvData >> petitionguid;                              // petition guid

    QueryResult result = CharacterDatabase.PQuery("SELECT ownerguid FROM petition WHERE petitionguid = '%u'", petitionguid.GetCounter());
    if(!result)
        return;

    Field *fields = result->Fetch();
    ownerguid = ObjectGuid(HighGuid::Player, fields[0].GetUInt32());

    Player *owner = sObjectMgr->GetPlayer(ownerguid);
    if(owner)                                               // petition owner online
    {
        WorldPacket data(MSG_PETITION_DECLINE, 8);
        data << _player->GetGUID();
        owner->SendDirectMessage(&data);
    }
}

void WorldSession::HandleOfferPetitionOpcode(WorldPacket & recvData)
{
    uint8 signs = 0;
    ObjectGuid petitionguid, plguid;
    uint32 type, junk;
    Player *player;
    recvData >> junk;                                      // this is not petition type!
    recvData >> petitionguid;                              // petition guid
    recvData >> plguid;                                    // player guid

    player = ObjectAccessor::FindPlayer(plguid);
    if (!player)
        return;

    QueryResult result = CharacterDatabase.PQuery("SELECT type FROM petition WHERE petitionguid = '%u'", petitionguid.GetCounter());
    if (!result)
        return;

    Field *fields = result->Fetch();
    type = fields[0].GetUInt32();

    if (!sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD) && GetPlayer()->GetTeam() != player->GetTeam() )
    {
        if(type != 9)
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", "", ERR_ARENA_TEAM_NOT_ALLIED);
        else
            SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_NOT_ALLIED);
        return;
    }

    if(type != 9)
    {
        if(player->GetLevel() < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            // player is too low level to join an arena team
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, player->GetName(), "", ERR_ARENA_TEAM_PLAYER_TO_LOW);
            return;
        }

        uint8 slot = ArenaTeam::GetSlotByType(type);
        if(slot >= MAX_ARENA_SLOT)
            return;

        if(player->GetArenaTeamId(slot))
        {
            // player is already in an arena team
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, player->GetName(), "", ERR_ALREADY_IN_ARENA_TEAM_S);
            return;
        }

        if(player->GetArenaTeamIdInvited())
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", _player->GetName(), ERR_ALREADY_INVITED_TO_ARENA_TEAM_S);
            return;
        }
    }
    else
    {
        if(player->GetGuildId())
        {
            SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ALREADY_IN_GUILD);
            return;
        }

        if(player->GetGuildIdInvited())
        {
            SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ALREADY_INVITED_TO_GUILD);
            return;
        }
    }

    result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", petitionguid.GetCounter());
    // result==NULL also correct charter without signs
    if(result)
        signs = result->GetRowCount();

    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, (8+8+4+signs+signs*12));
    data << petitionguid;                                   // petition guid
    data << _player->GetGUID();                             // owner guid
    data << petitionguid.GetCounter();                      // guild guid (in Trinity always same as GUID_LOPART(petition guid)
    data << signs;                                          // sign's count

    for(uint8 i = 1; i <= signs; i++)
    {
        Field *fields = result->Fetch();
        ObjectGuid plguid = ObjectGuid(fields[0].GetUInt64());

        data << plguid;                                     // Player GUID
        data << (uint32)0;                                  // there 0 ...

        result->NextRow();
    }

    player->SendDirectMessage(&data);
}

void WorldSession::HandleTurnInPetitionOpcode(WorldPacket & recvData)
{
    WorldPacket data;
    ObjectGuid petitionguid;

    ObjectGuid::LowType ownerguidlo;
    uint32 type;
    std::string name;

    recvData >> petitionguid;
    
    // data
    QueryResult result = CharacterDatabase.PQuery("SELECT ownerguid, name, type FROM petition WHERE petitionguid = '%u'", petitionguid.GetCounter());
    if(result)
    {
        Field *fields = result->Fetch();
        ownerguidlo = fields[0].GetUInt32();
        name = fields[1].GetString();
        type = fields[2].GetUInt32();
    }
    else
    {
        TC_LOG_ERROR("FIXME","petition table has broken data!");
        return;
    }

    if(type == 9)
    {
        if(_player->GetGuildId())
        {
            data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
            data << (uint32)PETITION_TURN_ALREADY_IN_GUILD; // already in guild
            _player->SendDirectMessage(&data);
            return;
        }
    }
    else
    {
        uint8 slot = ArenaTeam::GetSlotByType(type);
        if(slot >= MAX_ARENA_SLOT)
            return;

        if(_player->GetArenaTeamId(slot))
        {
            //data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
            //data << (uint32)PETITION_TURN_ALREADY_IN_GUILD;                          // already in guild
            //_player->SendDirectMessage(&data);
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, name, "", ERR_ALREADY_IN_ARENA_TEAM);
            return;
        }
    }

    if(_player->GetGUID().GetCounter() != ownerguidlo)
        return;

    // signs
    uint8 signs;
    result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", petitionguid.GetCounter());
    if(result)
        signs = result->GetRowCount();
    else
        signs = 0;
        
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    uint32 count;
    //if(signs < sWorld->getConfig(CONFIG_MIN_PETITION_SIGNS))
    if(type == 9)
        count = sWorld->getConfig(CONFIG_MIN_PETITION_SIGNS);
    else
        count = type-1;
    if(signs < count)
    {
        data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
        data << (uint32)PETITION_TURN_NEED_MORE_SIGNATURES; // need more signatures...
        SendPacket(&data);
        return;
    }

    if(type == 9)
    {
        if(sObjectMgr->GetGuildByName(name))
        {
            SendGuildCommandResult(GUILD_CREATE_S, name, GUILD_NAME_EXISTS);
            return;
        }
    }
    else
    {
        if(sObjectMgr->GetArenaTeamByName(name))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, name, "", ERR_ARENA_TEAM_NAME_EXISTS_S);
            return;
        }
    }

    // and at last charter item check
    Item *item = _player->GetItemByGuid(petitionguid);
    if(!item)
    {
        return;
    }

    // OK!

    // delete charter item
    _player->DestroyItem(item->GetBagSlot(),item->GetSlot(), true);

    if(type == 9)                                           // create guild
    {
        auto  guild = new Guild;
        if(!guild->create(_player->GetGUID(), name))
        {
            delete guild;
            return;
        }

        // register guild and add guildmaster
        sObjectMgr->AddGuild(guild);

        // add members
        for(uint8 i = 0; i < signs; ++i)
        {
            Field* fields = result->Fetch();
            guild->AddMember(ObjectGuid(fields[0].GetUInt64()), guild->GetLowestRank(), trans);
            result->NextRow();
        }
    }
    else                                                    // or arena team
    {
        auto  at = new ArenaTeam;
        if(!at->Create(_player->GetGUID(), type, name))
        {
            TC_LOG_ERROR("FIXME","PetitionsHandler: arena team create failed.");
            delete at;
            return;
        }

        
        uint32 icon, iconcolor, border, bordercolor, backgroud;
        recvData >> backgroud >> icon >> iconcolor >> border >> bordercolor;

        at->SetEmblem(backgroud, icon, iconcolor, border, bordercolor);

        // register team and add captain
        sObjectMgr->AddArenaTeam(at);

        // add members
        for(uint8 i = 0; i < signs; ++i)
        {
            Field* fields = result->Fetch();
            ObjectGuid memberGUID = ObjectGuid(fields[0].GetUInt64());
            at->AddMember(memberGUID, trans);
            result->NextRow();
        }
    }

    trans->PAppend("DELETE FROM petition WHERE petitionguid = '%u'", petitionguid.GetCounter());
    trans->PAppend("DELETE FROM petition_sign WHERE petitionguid = '%u'", petitionguid.GetCounter());
    CharacterDatabase.CommitTransaction(trans);

    data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
    data << (uint32)PETITION_TURN_OK;
    SendPacket(&data);
}

void WorldSession::HandlePetitionShowListOpcode(WorldPacket & recvData)
{
    
    
    

    ObjectGuid guid;
    recvData >> guid;

    SendPetitionShowList(guid);
}

void WorldSession::SendPetitionShowList(ObjectGuid guid)
{
    Creature *pCreature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_PETITIONER);
    if (!pCreature)
    {
        TC_LOG_ERROR("FIXME","WORLD: HandlePetitionShowListOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(guid.GetCounter()));
        return;
    }

    // remove fake death
    if(GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    uint8 count = 0;
    if(pCreature->IsTabardDesigner())
        count = 1;
    else
        count = 3;

    WorldPacket data(SMSG_PETITION_SHOWLIST, 8+1+4*6);
    data << guid;                                           // npc guid
    data << count;                                          // count
    if(count == 1)
    {
        data << uint32(1);                                  // index
        data << uint32(GUILD_CHARTER);                      // charter entry
        data << uint32(16161);                              // charter display id
        data << uint32(GUILD_CHARTER_COST);                 // charter cost
        data << uint32(0);                                  // unknown
        data << uint32(9);                                  // required signs?
    }
    else
    {
        // 2v2
        data << uint32(1);                                  // index
        data << uint32(ARENA_TEAM_CHARTER_2v2);             // charter entry
        data << uint32(16161);                              // charter display id
        data << uint32(ARENA_TEAM_CHARTER_2v2_COST);        // charter cost
        data << uint32(2);                                  // unknown
        data << uint32(2);                                  // required signs?
        // 3v3
        data << uint32(2);                                  // index
        data << uint32(ARENA_TEAM_CHARTER_3v3);             // charter entry
        data << uint32(16161);                              // charter display id
        data << uint32(ARENA_TEAM_CHARTER_3v3_COST);        // charter cost
        data << uint32(3);                                  // unknown
        data << uint32(3);                                  // required signs?
        // 5v5
        data << uint32(3);                                  // index
        data << uint32(ARENA_TEAM_CHARTER_5v5);             // charter entry
        data << uint32(16161);                              // charter display id
        data << uint32(ARENA_TEAM_CHARTER_5v5_COST);        // charter cost
        data << uint32(5);                                  // unknown
        data << uint32(5);                                  // required signs?
    }
    //for(uint8 i = 0; i < count; i++)
    //{
    //    data << uint32(i);                      // index
    //    data << uint32(GUILD_CHARTER);          // charter entry
    //    data << uint32(16161);                  // charter display id
    //    data << uint32(GUILD_CHARTER_COST+i);   // charter cost
    //    data << uint32(0);                      // unknown
    //    data << uint32(9);                      // required signs?
    //}
    SendPacket(&data);
}

