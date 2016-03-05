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

#include <vector>
#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "SharedDefines.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Guild.h"
#include "UpdateMask.h"
#include "openssl/md5.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Group.h"
#include "PlayerDump.h"
#include "SocialMgr.h"
#include "Util.h"
#include "ArenaTeam.h"
#include "Language.h"
#include "Chat.h"
#include "SystemConfig.h"
#include "Config.h"
#include "ScriptCalls.h"
#include "ScriptMgr.h"
#include "IRCMgr.h"
#include "LogsDatabaseAccessor.h"

#ifdef PLAYERBOT
#include "playerbot.h"
#include "PlayerbotAIConfig.h"
#endif

class LoginQueryHolder : public SQLQueryHolder
{
    private:
        uint32 m_accountId;
        uint64 m_guid;
    public:
        LoginQueryHolder(uint32 accountId, uint64 guid)
            : m_accountId(accountId), m_guid(guid) { }
        uint64 GetGuid() const { return m_guid; }
        uint32 GetAccountId() const { return m_accountId; }
        bool Initialize();
};

#ifdef PLAYERBOT

class PlayerbotLoginQueryHolder : public LoginQueryHolder
{
private:
    uint32 masterAccountId;
    PlayerbotHolder* playerbotHolder;

public:
    PlayerbotLoginQueryHolder(PlayerbotHolder* playerbotHolder, uint32 masterAccount, uint32 accountId, uint64 guid)
        : LoginQueryHolder(accountId, ObjectGuid(guid)), masterAccountId(masterAccount), playerbotHolder(playerbotHolder) { }

public:
    uint32 GetMasterAccountId() const { return masterAccountId; }
    PlayerbotHolder* GetPlayerbotHolder() { return playerbotHolder; }
};

void PlayerbotHolder::AddPlayerBot(uint64 playerGuid, uint32 masterAccount)
{
    // has bot already been added?x
	Player* bot = sObjectMgr->GetPlayer(playerGuid);

	if (bot && bot->IsInWorld())
        return;

    uint32 accountId = sObjectMgr->GetPlayerAccountIdByGUID(ObjectGuid(playerGuid));
    if (accountId == 0)
        return;

    PlayerbotLoginQueryHolder *holder = new PlayerbotLoginQueryHolder(this, masterAccount, accountId, playerGuid);
    if(!holder->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        return;
    }

    QueryResultHolderFuture future = CharacterDatabase.DelayQueryHolder(holder);
    future.get();

    WorldSession* masterSession = masterAccount ? sWorld->FindSession(masterAccount) : NULL;
    uint32 botAccountId = holder->GetAccountId();
    WorldSession *botSession = new WorldSession(botAccountId, BUILD_243, NULL, SEC_PLAYER, 2, 0, LOCALE_enUS, 0, false);

    botSession->HandlePlayerLogin(holder); // will delete lqh

	bot = botSession->GetPlayer();
	if (!bot)
		return;

	PlayerbotMgr *mgr = bot->GetPlayerbotMgr();
	bot->SetPlayerbotMgr(NULL);
	delete mgr;
	sRandomPlayerbotMgr.OnPlayerLogout(bot);

    bool allowed = false;
    if (botAccountId == masterAccount)
        allowed = true;
    else if (masterSession && sPlayerbotAIConfig.allowGuildBots && bot->GetGuildId() == masterSession->GetPlayer()->GetGuildId())
        allowed = true;
    else if (sPlayerbotAIConfig.IsInRandomAccountList(botAccountId))
        allowed = true;

    if (allowed)
        OnBotLogin(bot);
    else if (masterSession)
    {
        ChatHandler ch(masterSession);
        ch.PSendSysMessage("You are not allowed to control bot %s...", bot->GetName().c_str());
        LogoutPlayerBot(bot->GetGUID());
    }
}

#endif

bool LoginQueryHolder::Initialize()
{
    SetSize(MAX_PLAYER_LOGIN_QUERY);

    bool res = true;

    // NOTE: all fields in `characters` must be read to prevent lost character data at next save in case wrong DB structure.
    // !!! NOTE: including unused `zone`,`online`
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADFROM,            "SELECT characters.guid, account, data, name, race, class, gender, level, xp, money, playerBytes, playerBytes2, playerFlags, position_x, position_y, position_z, map, instance_id, orientation, taximask, online, cinematic, totaltime, leveltime, logout_time, is_logout_resting, rest_bonus, resettalents_cost, resettalents_time, trans_x, trans_y, trans_z, trans_o, transguid, extra_flags, stable_slots, at_login, zone, death_expire_time, taxi_path, dungeon_difficulty, arena_pending_points, arenapoints, totalHonorPoints, todayHonorPoints, yesterdayHonorPoints, totalKills, todayKills, yesterdayKills, chosenTitle, watchedFaction, drunk, health, power1, power2, power3, power4, power5, exploredZones, equipmentCache, ammoId, knownTitles, actionBars, xp_blocked, lastGenderChange, IFNULL(custom_xp, 0.0) FROM characters LEFT OUTER JOIN character_custom_xp on characters.guid = character_custom_xp.guid WHERE characters.guid = '%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADGROUP,           "SELECT leaderGuid FROM group_member WHERE memberGuid ='%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADBOUNDINSTANCES,  "SELECT id, permanent, map, difficulty, resettime FROM character_instance LEFT JOIN instance ON instance = id WHERE guid = '%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADAURAS,           "SELECT caster_guid,spell,effect_index,stackcount,amount,maxduration,remaintime,remaincharges FROM character_aura WHERE guid = '%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADSPELLS,          "SELECT spell,slot,active,disabled FROM character_spell WHERE guid = '%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADQUESTSTATUS,     "SELECT quest,status,rewarded,explored,timer,mobcount1,mobcount2,mobcount3,mobcount4,itemcount1,itemcount2,itemcount3,itemcount4 FROM character_queststatus WHERE guid = '%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADDAILYQUESTSTATUS,"SELECT quest,time FROM character_queststatus_daily WHERE guid = '%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADREPUTATION,      "SELECT faction,standing,flags FROM character_reputation WHERE guid = '%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADINVENTORY,       "SELECT bag,slot,item,item_template FROM character_inventory JOIN item_instance ON character_inventory.item = item_instance.guid WHERE character_inventory.guid = '%u' ORDER BY bag,slot", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADACTIONS,         "SELECT button,action,type,misc FROM character_action WHERE guid = '%u' ORDER BY button", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADMAILCOUNT,       "SELECT COUNT(id) FROM mail WHERE receiver = '%u' AND (checked & 1)=0 AND deliver_time <= '" UI64FMTD "'", GUID_LOPART(m_guid),(uint64)time(NULL));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADMAILDATE,        "SELECT MIN(deliver_time) FROM mail WHERE receiver = '%u' AND (checked & 1)=0", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADSOCIALLIST,      "SELECT friend,flags,note FROM character_social WHERE guid = '%u' LIMIT 255", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADHOMEBIND,        "SELECT map,zone,position_x,position_y,position_z FROM character_homebind WHERE guid = '%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADSPELLCOOLDOWNS,  "SELECT spell,item,time FROM character_spell_cooldown WHERE guid = '%u'", GUID_LOPART(m_guid));
    if(sWorld->getConfig(CONFIG_DECLINED_NAMES_USED))
        res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADDECLINEDNAMES,   "SELECT genitive, dative, accusative, instrumental, prepositional FROM character_declinedname WHERE guid = '%u'",GUID_LOPART(m_guid));
    // in other case still be dummy query
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADGUILD,           "SELECT guildid,rank FROM guild_member WHERE guid = '%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADARENAINFO,       "SELECT arenateamid, played_week, played_season, personal_rating FROM arena_team_member WHERE guid='%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADBGCOORD,         "SELECT bgid, bgteam, bgmap, bgx, bgy, bgz, bgo FROM character_bgcoord WHERE guid = '%u'", GUID_LOPART(m_guid));
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADSKILLS,          "SELECT skill, value, max FROM character_skills WHERE guid = '%u'", GUID_LOPART(m_guid));

    return res;
}

void WorldSession::HandleCharEnum(PreparedQueryResult result)
{
    WorldPacket data(SMSG_CHAR_ENUM, 100);                  // we guess size

    uint8 num = 0;

    data << num;

    _legitCharacters.clear();
    if (result)
    {
        do
        {
            uint32 guidlow = (*result)[0].GetUInt32();
            TC_LOG_INFO("network", "Loading char guid %u from account %u.", guidlow, GetAccountId());
            if (Player::BuildEnumData(result, &data))
            {
                // Do not allow banned characters to log in
                if (!(*result)[20].GetUInt32())
                    _legitCharacters.insert(guidlow);

                /*       0        1      2        3      4
                SELECT c.guid, c.name, c.race, c.class, c.gender, c.playerBytes, c.playerBytes2, c.level, c.zone, c.map, c.position_x, c.position_y, c.position_z, "
                "gm.guildid, c.playerFlags, c.at_login, cp.entry, cp.modelid, cp.level, c.equipmentCache, cb.guid "
                */
                if (!sWorld->HasGlobalPlayerData(guidlow)) // This can happen if characters are inserted into the database manually. Core hasn't loaded name data yet.
                {
                    std::string name = (*result)[1].GetString();
                    uint8 gender = (*result)[4].GetUInt8();
                    uint8 race = (*result)[2].GetUInt8();
                    uint8 playerclass = (*result)[3].GetUInt8();
                    uint8 level = (*result)[7].GetUInt8();
                    sWorld->AddGlobalPlayerData(guidlow, GetAccountId(), name, gender, race, playerclass, level, 0, 0);
                }
                ++num;
            }
        }
        while (result->NextRow());
    }

    data.put<uint8>(0, num);

    SendPacket(&data);
}

void WorldSession::HandleCharEnumOpcode( WorldPacket & /*recvData*/ )
{
    PreparedStatement* stmt;
    // remove expired bans
/*TODO no ban per character yet
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_EXPIRED_BANS);
    CharacterDatabase.Execute(stmt);
    */
    /// get all the data necessary for loading all characters (along with their pets) on the account

    if (sWorld->getConfig(CONFIG_DECLINED_NAMES_USED))
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ENUM_DECLINED_NAME);
    else
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ENUM);

    stmt->setUInt8(0, PET_SAVE_AS_CURRENT);
    stmt->setUInt32(1, GetAccountId());

    _charEnumCallback = CharacterDatabase.AsyncQuery(stmt);
}

void WorldSession::HandleCharCreateOpcode( WorldPacket & recvData )
{
    CHECK_PACKET_SIZE(recvData,1+1+1+1+1+1+1+1+1+1);

    CharacterCreateInfo createInfo;

    recvData >> createInfo.Name
             >> createInfo.Race
             >> createInfo.Class
             >> createInfo.Gender
             >> createInfo.Skin
             >> createInfo.Face
             >> createInfo.HairStyle
             >> createInfo.HairColor
             >> createInfo.FacialHair
             >> createInfo.OutfitId;

    //still got data to extract for packet but lets perform some checks first

    if(GetSecurity() == SEC_PLAYER)
    {
        if(uint32 mask = sWorld->getConfig(CONFIG_CHARACTERS_CREATING_DISABLED))
        {
            bool disabled = false;

            uint32 team = Player::TeamForRace(createInfo.Race);
            switch(team)
            {
                case TEAM_ALLIANCE: disabled = mask & (1<<0); break;
                case TEAM_HORDE:    disabled = mask & (1<<1); break;
            }

            if(disabled)
            {
                SendCharCreate(CHAR_CREATE_DISABLED);
                return;
            }
        }
    }

    ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(createInfo.Class);
    ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(createInfo.Race);

    if( !classEntry || !raceEntry )
    {
        SendCharCreate(CHAR_CREATE_FAILED);
        TC_LOG_ERROR("network","Class: %u or Race %u not found in DBC (Wrong DBC files?) or Cheater?", createInfo.Class, createInfo.Race);
        return;
    }

    // prevent character creating Expansion race without Expansion account
    if (raceEntry->addon > Expansion())
    {
        SendCharCreate(CHAR_CREATE_EXPANSION);
        TC_LOG_ERROR("network","Not Expansion 1 account:[%d] but tried to Create character with expansion 1 race (%u)",GetAccountId(),createInfo.Race);
        return;
    }

    // prevent character creating Expansion class without Expansion account
    // TODO: use possible addon field in ChrClassesEntry in next dbc version
    if (Expansion() < 2 && createInfo.Class == CLASS_DEATH_KNIGHT)
    {
        SendCharCreate(CHAR_CREATE_EXPANSION);
        TC_LOG_ERROR("network","Not Expansion 2 account:[%d] but tried to Create character with expansion 2 class (%u)",GetAccountId(),createInfo.Class);
        return;
    }

    // prevent character creating with invalid name
    if(!normalizePlayerName(createInfo.Name))
    {
        SendCharCreate(CHAR_NAME_INVALID_CHARACTER);
        TC_LOG_ERROR("network","Account:[%d] but tried to Create character with empty [name] ",GetAccountId());
        return;
    }

    // check name limitations
    ResponseCodes res = ObjectMgr::CheckPlayerName(createInfo.Name, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        SendCharCreate(res);
        return;
    }

    if(GetSecurity() == SEC_PLAYER && sObjectMgr->IsReservedName(createInfo.Name))
    {
        SendCharCreate(CHAR_NAME_RESERVED);
        return;
    }

    if(sObjectMgr->GetPlayerGUIDByName(createInfo.Name))
    {
        SendCharCreate(CHAR_CREATE_NAME_IN_USE);
        return;
    }

    QueryResult resultacct = LoginDatabase.PQuery("SELECT SUM(numchars) FROM realmcharacters WHERE acctid = '%d'", GetAccountId());
    if ( resultacct )
    {
        Field *fields=resultacct->Fetch();
        uint32 acctcharcount = fields[0].GetDouble();

        if (acctcharcount >= sWorld->getConfig(CONFIG_CHARACTERS_PER_ACCOUNT))
        {
            SendCharCreate(CHAR_CREATE_ACCOUNT_LIMIT);
            return;
        }
    }

    QueryResult result = CharacterDatabase.PQuery("SELECT COUNT(guid) FROM characters WHERE account = '%d'", GetAccountId());
    uint8 charcount = 0;
    if ( result )
    {
        Field *fields=result->Fetch();
        charcount = fields[0].GetUInt64();

        if (charcount >= sWorld->getConfig(CONFIG_CHARACTERS_PER_REALM))
        {
            SendCharCreate(CHAR_CREATE_SERVER_LIMIT);
            return;
        }
    }

    bool AllowTwoSideAccounts = !sWorld->IsPvPRealm() || sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS) || GetSecurity() > SEC_PLAYER;
    uint32 skipCinematics = sWorld->getConfig(CONFIG_SKIP_CINEMATICS);

    if(!AllowTwoSideAccounts || skipCinematics == 1)
    {
        QueryResult result2 = CharacterDatabase.PQuery("SELECT DISTINCT race FROM characters WHERE account = '%u' %s", GetAccountId(),skipCinematics == 1 ? "" : "LIMIT 1");
        if(result2)
        {
            uint32 team_= Player::TeamForRace(createInfo.Race);

            Field* field = result2->Fetch();
            uint8 race = field[0].GetUInt32();

            // need to check team only for first character
            // TODO: what to if account already has characters of both races?
            if (!AllowTwoSideAccounts)
            {
                uint32 team=0;
                if(race > 0)
                    team = Player::TeamForRace(race);

                if(team != team_)
                {
                    SendCharCreate(CHAR_CREATE_PVP_TEAMS_VIOLATION);
                    return;
                }
            }

            if (skipCinematics == 1)
            {
                // TODO: check if cinematic already shown? (already logged in?; cinematic field)
                while (createInfo.Race != race && result2->NextRow())
                {
                    field = result2->Fetch();
                    race = field[0].GetUInt32();
                }
            }
        }
    }
    
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
    stmt->setString(0, createInfo.Name);

    delete _charCreateCallback.GetParam();  // Delete existing if any, to make the callback chain reset to stage 0
    _charCreateCallback.SetParam(new CharacterCreateInfo(std::move(createInfo)));
    _charCreateCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));

    std::string IP_str = GetRemoteAddress();
    TC_LOG_DEBUG("network","Account: %d (IP: %s) Create Character:[%s]",GetAccountId(),IP_str.c_str(),createInfo.Name.c_str());
    //sLog->outChar("network","Account: %d (IP: %s) Create Character:[%s]",GetAccountId(),IP_str.c_str(),name.c_str());
}

void WorldSession::HandleCharCreateCallback(PreparedQueryResult result, CharacterCreateInfo* createInfo)
{
    /** This is a series of callbacks executed consecutively as a result from the database becomes available.
        This is much more efficient than synchronous requests on packet handler, and much less DoS prone.
        It also prevents data syncrhonisation errors.
    */
    switch (_charCreateCallback.GetStage())
    {
        case 0:
        {
            if (result)
            {
                SendCharCreate(CHAR_CREATE_NAME_IN_USE);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }

            ASSERT(_charCreateCallback.GetParam() == createInfo);

            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_SUM_REALM_CHARACTERS);
            stmt->setUInt32(0, GetAccountId());

            _charCreateCallback.FreeResult();
            _charCreateCallback.SetFutureResult(LoginDatabase.AsyncQuery(stmt));
            _charCreateCallback.NextStage();
        }
        break;
        case 1:
        {
            uint16 acctCharCount = 0;
            if (result)
            {
                Field* fields = result->Fetch();
                // SELECT SUM(x) is MYSQL_TYPE_NEWDECIMAL - needs to be read as string
                const char* ch = fields[0].GetCString();
                if (ch)
                    acctCharCount = atoi(ch);
            }

            if (acctCharCount >= sWorld->getConfig(CONFIG_CHARACTERS_PER_ACCOUNT))
            {
                SendCharCreate(CHAR_CREATE_ACCOUNT_LIMIT);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }


            ASSERT(_charCreateCallback.GetParam() == createInfo);

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_SUM_CHARS);
            stmt->setUInt32(0, GetAccountId());

            _charCreateCallback.FreeResult();
            _charCreateCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
            _charCreateCallback.NextStage();
        }
        break;
        case 2:
        {
            if (result)
            {
                Field* fields = result->Fetch();
                createInfo->CharCount = uint8(fields[0].GetUInt64()); // SQL's COUNT() returns uint64 but it will always be less than uint8.Max

                if (createInfo->CharCount >= sWorld->getConfig(CONFIG_CHARACTERS_PER_REALM))
                {
                    SendCharCreate(CHAR_CREATE_SERVER_LIMIT);
                    delete createInfo;
                    _charCreateCallback.Reset();
                    return;
                }
            }

            bool allowTwoSideAccounts = !sWorld->IsPvPRealm() || sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS) || GetSecurity() > SEC_PLAYER;//|| HasPermission(rbac::RBAC_PERM_TWO_SIDE_CHARACTER_CREATION);
            uint32 skipCinematics = sWorld->getConfig(CONFIG_SKIP_CINEMATICS);

            _charCreateCallback.FreeResult();

            if (!allowTwoSideAccounts || skipCinematics == 1 || createInfo->Class == CLASS_DEATH_KNIGHT)
            {
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_CREATE_INFO);
                stmt->setUInt32(0, GetAccountId());
                stmt->setUInt32(1, (skipCinematics == 1 || createInfo->Class == CLASS_DEATH_KNIGHT) ? 10 : 1);
                _charCreateCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
                _charCreateCallback.NextStage();
                return;
            }

            _charCreateCallback.NextStage();
            HandleCharCreateCallback(PreparedQueryResult(NULL), createInfo);   // Will jump to case 3
        }
        break;
        case 3:
        {
            bool haveSameRace = false;
            //uint32 heroicReqLevel = sWorld->getConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER);
            //bool hasHeroicReqLevel = (heroicReqLevel == 0);
            bool allowTwoSideAccounts = !sWorld->IsPvPRealm() || sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS) || GetSecurity() > SEC_PLAYER ;// HasPermission(rbac::RBAC_PERM_TWO_SIDE_CHARACTER_CREATION);
            uint32 skipCinematics = sWorld->getConfig(CONFIG_SKIP_CINEMATICS);

            if (result)
            {
                uint32 team = Player::TeamForRace(createInfo->Race);
                //uint32 freeHeroicSlots = sWorld->getConfig(CONFIG_HEROIC_CHARACTERS_PER_REALM);

                Field* field = result->Fetch();
                uint8 accRace  = field[1].GetUInt8();

                // need to check team only for first character
                /// @todo what to if account already has characters of both races?
                if (!allowTwoSideAccounts)
                {
                    uint32 accTeam = 0;
                    if (accRace > 0)
                        accTeam = Player::TeamForRace(accRace);

                    if (accTeam != team)
                    {
                        SendCharCreate(CHAR_CREATE_PVP_TEAMS_VIOLATION);
                        delete createInfo;
                        _charCreateCallback.Reset();
                        return;
                    }
                }

                // search same race for cinematic or same class if need
                /// @todo check if cinematic already shown? (already logged in?; cinematic field)
                while ((skipCinematics == 1 && !haveSameRace))// || createInfo->Class == CLASS_DEATH_KNIGHT)
                {
                    if (!result->NextRow())
                        break;

                    field = result->Fetch();
                    accRace = field[1].GetUInt8();

                    if (!haveSameRace)
                        haveSameRace = createInfo->Race == accRace;

                    //uint8 acc_class = field[2].GetUInt8();
                }
            }
#ifdef LICH_KING
            if (checkHeroicReqs && !hasHeroicReqLevel)
            {
                SendCharCreate(CHAR_CREATE_LEVEL_REQUIREMENT);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }
#endif
            if (createInfo->Data.rpos() < createInfo->Data.wpos())
            {
                uint8 unk;
                createInfo->Data >> unk;
                TC_LOG_DEBUG("network", "Character creation %s (account %u) has unhandled tail data: [%u]", createInfo->Name.c_str(), GetAccountId(), unk);
            }

            Player newChar(this);
            newChar.GetMotionMaster()->Initialize();
            if (!newChar.Create(sObjectMgr->GenerateLowGuid(HIGHGUID_PLAYER), createInfo))
            {
                // Player not create (race/class/etc problem?)
                newChar.CleanupsBeforeDelete();

                SendCharCreate(CHAR_CREATE_ERROR);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }

            if ((haveSameRace && skipCinematics == 1) || skipCinematics == 2)
                newChar.setCinematic(1);                          // not show intro

            newChar.SetAtLoginFlag(AT_LOGIN_FIRST);               // First login

            // Player created, save it now
            newChar.SaveToDB(true);
            createInfo->CharCount += 1;

            SQLTransaction trans = LoginDatabase.BeginTransaction();

            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_REALM_CHARACTERS_BY_REALM);
            stmt->setUInt32(0, GetAccountId());
            stmt->setUInt32(1, realmID);
            trans->Append(stmt);

            stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_REALM_CHARACTERS);
            stmt->setUInt32(0, createInfo->CharCount);
            stmt->setUInt32(1, GetAccountId());
            stmt->setUInt32(2, realmID);
            trans->Append(stmt);

            LoginDatabase.CommitTransaction(trans);

            SendCharCreate(CHAR_CREATE_SUCCESS);

            std::string IP_str = GetRemoteAddress();
            TC_LOG_INFO("entities.player.character", "Account: %d (IP: %s) Create Character:[%s] (GUID: %u)", GetAccountId(), IP_str.c_str(), createInfo->Name.c_str(), newChar.GetGUIDLow());
            sScriptMgr->OnPlayerCreate(&newChar);
            sWorld->AddGlobalPlayerData(newChar.GetGUIDLow(), GetAccountId(), newChar.GetName(), newChar.GetGender(), newChar.GetRace(), newChar.GetClass(), newChar.GetLevel(), 0, 0);

            newChar.CleanupsBeforeDelete();
            delete createInfo;
            _charCreateCallback.Reset();
        }
        break;
    }
}

void WorldSession::HandleCharDeleteOpcode( WorldPacket & recvData )
{
    CHECK_PACKET_SIZE(recvData,8);

    uint64 guid;
    recvData >> guid;

    uint32 initAccountId = GetAccountId();

    // can't delete loaded character
    if (ObjectAccessor::FindPlayer(guid))
    {
        sScriptMgr->OnPlayerFailedDelete(guid, initAccountId);
        return;
    }

    uint32 accountId = 0;
    uint8 level = 0;
    std::string name;

    // is guild leader
    if(sObjectMgr->IsGuildLeader(guid))
    {
        sScriptMgr->OnPlayerFailedDelete(guid, initAccountId);
        SendCharDelete(CHAR_DELETE_FAILED_GUILD_LEADER);
        return;
    }

    // is arena team captain
    if(sObjectMgr->IsArenaTeamCaptain(guid))
    {
        sScriptMgr->OnPlayerFailedDelete(guid, initAccountId);
        SendCharDelete(CHAR_DELETE_FAILED_ARENA_CAPTAIN);
        return;
    }

    QueryResult result = CharacterDatabase.PQuery("SELECT account, name, level FROM characters WHERE guid='%u'", GUID_LOPART(guid));
    if(result)
    {
        Field *fields = result->Fetch();
        accountId = fields[0].GetUInt32();
        name = fields[1].GetString();
        level = fields[2].GetUInt8();
    }

    // prevent deleting other players' characters using cheating tools
    if(accountId != GetAccountId())
        return;

    std::string IP_str = GetRemoteAddress();
    TC_LOG_DEBUG("entities.player","Account: %d (IP: %s) Delete Character:[%s] (guid:%u)",GetAccountId(),IP_str.c_str(),name.c_str(),GUID_LOPART(guid));

    sScriptMgr->OnPlayerDelete(guid, initAccountId); // To prevent race conditioning, but as it also makes sense, we hand the accountId over for successful delete.

    std::string fname = sConfigMgr->GetStringDefault("LogsDir", "");
    if (fname.length() > 0 && fname.at(fname.length()-1) != '/')
        fname.append("/");
    char fpath[64];
    snprintf(fpath, 64, "chardump/%d_%d_%s", GetAccountId(), GUID_LOPART(guid), name.c_str());
    fname.append(fpath);
    PlayerDumpWriter().WriteDump(fname, GUID_LOPART(guid));


    LogsDatabaseAccessor::CharacterDelete(this, GUID_LOPART(guid), name, level, GetRemoteAddress());

#ifdef LICH_LING
    sCalendarMgr->RemoveAllPlayerEventsAndInvites(guid);
#endif
    Player::DeleteFromDB(guid, GetAccountId());

    SendCharDelete(CHAR_DELETE_SUCCESS);
}

void WorldSession::HandlePlayerLoginOpcode( WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,8);

    if(PlayerLoading() || GetPlayer() != NULL)
    {
        TC_LOG_ERROR("FIXME","Player tryes to login again, AccountId = %d",GetAccountId());
        return;
    }

    m_playerLoading = true;
    uint64 playerGuid = 0;

    TC_LOG_DEBUG( "FIXME","WORLD: Recvd Player Logon Message" );

    recvData >> playerGuid;

    LoginQueryHolder *holder = new LoginQueryHolder(GetAccountId(), playerGuid);
    if(!holder->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        m_playerLoading = false;
        return;
    }

    _charLoginCallback = CharacterDatabase.DelayQueryHolder((SQLQueryHolder*)holder);
}

void WorldSession::HandlePlayerLogin(LoginQueryHolder * holder)
{
    uint64 playerGuid = holder->GetGuid();

    Player* pCurrChar = new Player(this);
     // for send server info and strings (config)
    ChatHandler chH = ChatHandler(pCurrChar);

    // "GetAccountId()==db stored account id" checked in LoadFromDB (prevent login not own character using cheating tools)
    if(!pCurrChar->LoadFromDB(GUID_LOPART(playerGuid), holder))
    {
        KickPlayer();                                       // disconnect client, player no set to session and it will not deleted or saved at kick
        delete pCurrChar;                                   // delete it manually
        delete holder;                                      // delete all unprocessed queries
        m_playerLoading = false;
        return;
    }
    
    pCurrChar->GetMotionMaster()->Initialize();

    SetPlayer(pCurrChar);

    pCurrChar->SendDungeonDifficulty(false);

    WorldPacket data( SMSG_LOGIN_VERIFY_WORLD, 20 );
    data << pCurrChar->GetMapId();
    data << pCurrChar->GetPositionX();
    data << pCurrChar->GetPositionY();
    data << pCurrChar->GetPositionZ();
    data << pCurrChar->GetOrientation();
    SendPacket(&data);

#ifdef LICH_KING //need db support
    LoadAccountData(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_ACCOUNT_DATA), PER_CHARACTER_CACHE_MASK);
#endif
    SendAccountDataTimes(GLOBAL_CACHE_MASK);

    data.Initialize(SMSG_FEATURE_SYSTEM_STATUS, 2);         // added in 2.2.0
    data << uint8(2);                                       // unknown value
    data << uint8(0);                                       // enable(1)/disable(0) voice chat interface in client
    SendPacket(&data);

    {
        SendMotd();

        // send server info
        if(sWorld->getConfig(CONFIG_ENABLE_SINFO_LOGIN) == 1)
            chH.PSendSysMessage(_FULLVERSION);

        TC_LOG_DEBUG( "network", "WORLD: Sent server info" );
    }

    //warn player if no mail associated to account
    /*disabled QueryResult resultMail = LoginDatabase.PQuery("SELECT email, newMail FROM account WHERE id = '%u'", pCurrChar->GetSession()->GetAccountId());
    if(resultMail)
    {
        Field *fields = resultMail->Fetch();
        const char* mail = fields[0].GetCString();
        const char* mail_temp = fields[1].GetCString();

        if(!(mail && strcmp(mail, "") != 0) && !(mail_temp && strcmp(mail_temp, "") != 0))
            chH.PSendSysMessage("|cffff0000Aucune adresse email n'est actuellement associée a ce compte. Un compte sans mail associé ne peux etre recupéré en cas de perte. Vous pouvez utiliser le manager pour corriger ce problème.|r");
    } 
    */
    
    //QueryResult result = CharacterDatabase.PQuery("SELECT guildid,rank FROM guild_member WHERE guid = '%u'",pCurrChar->GetGUIDLow());
    QueryResult resultGuild = holder->GetResult(PLAYER_LOGIN_QUERY_LOADGUILD);

    uint32 guildId = 0;
    if(resultGuild)
    {
        Field *fields = resultGuild->Fetch();
        guildId = fields[0].GetUInt32();
        pCurrChar->SetInGuild(guildId);
        pCurrChar->SetRank(fields[1].GetUInt8());
    }
    else if(pCurrChar->GetGuildId())                        // clear guild related fields in case wrong data about non existed membership
    {
        pCurrChar->SetInGuild(0);
        pCurrChar->SetRank(0);
    }

    // Default guild for GMs
    if (GetSecurity() > SEC_PLAYER)
    {
        if (guildId == 0)
        {
            if (uint32 defaultGuildId = sWorld->getConfig(CONFIG_GM_DEFAULT_GUILD))
            {
                Guild* gmGuild = sObjectMgr->GetGuildById(defaultGuildId);
                if (gmGuild)
                {
                    SQLTransaction trans = CharacterDatabase.BeginTransaction();
                    if (gmGuild->AddMember(pCurrChar->GetGUID(), gmGuild->GetLowestRank(), trans))
                    {
                        guildId = defaultGuildId;
                        pCurrChar->SetInGuild(defaultGuildId);
                        pCurrChar->SetRank(gmGuild->GetLowestRank());
                    }
                    else
                    {
                        TC_LOG_ERROR("entities.player", "Could not add player to default guild Id %u for GM player %s (%u)", defaultGuildId, pCurrChar->GetName().c_str(), pCurrChar->GetGUIDLow());
                    }
                    CharacterDatabase.CommitTransaction(trans);
                }
                else {
                    TC_LOG_ERROR("entities.player", "Could not find default guild Id %u for GM player %s (%u)", defaultGuildId, pCurrChar->GetName().c_str(), pCurrChar->GetGUIDLow());
                }
            }
        }

        //force gm player in guild if needed
        
        if (uint32 GMForcedGuildId = sWorld->getIntConfig(CONFIG_GM_FORCE_GUILD))
        {
            if (GMForcedGuildId != guildId)
            {
                //remove from current guild if any
                if (Guild* currentGuild = sObjectMgr->GetGuildById(guildId))
                    currentGuild->DelMember(pCurrChar->GetGUID());

                if (Guild* gmGuild = sObjectMgr->GetGuildById(GMForcedGuildId))
                {
                    if (gmGuild->AddMember(pCurrChar->GetGUID(), gmGuild->GetLowestRank()))
                    {
                        pCurrChar->SetInGuild(GMForcedGuildId);
                        pCurrChar->SetRank(gmGuild->GetLowestRank());
                    }
                    else {
                        TC_LOG_ERROR("entities.player", "Could not add player to forced guild Id %u for GM player %s (%u)", GMForcedGuildId, pCurrChar->GetName().c_str(), pCurrChar->GetGUIDLow());
                    }
                }
            }
        }
    }

    if(pCurrChar->GetGuildId() != 0)
    {
        Guild* guild = sObjectMgr->GetGuildById(pCurrChar->GetGuildId());
        if(guild)
        {
            data.Initialize(SMSG_GUILD_EVENT, (2+guild->GetMOTD().size()+1));
            data << (uint8)GE_MOTD;
            data << (uint8)1;
            data << guild->GetMOTD();
            SendPacket(&data);
            TC_LOG_DEBUG( "network", "WORLD: Sent guild-motd (SMSG_GUILD_EVENT)" );

            data.Initialize(SMSG_GUILD_EVENT, (5+10));      // we guess size
            data<<(uint8)GE_SIGNED_ON;
            data<<(uint8)1;
            data<<pCurrChar->GetName();
            data<<pCurrChar->GetGUID();
            guild->BroadcastPacket(&data);
            
            if (sWorld->getConfig(CONFIG_IRC_ENABLED))
                sIRCMgr->onIngameGuildJoin(guild->GetId(), guild->GetName(), pCurrChar->GetName());

            TC_LOG_DEBUG( "network", "WORLD: Sent guild-signed-on (SMSG_GUILD_EVENT)" );

            // Increment online members of the guild
            guild->IncOnlineMemberCount();
        }
        else
        {
            // remove wrong guild data
            TC_LOG_ERROR("network","Player %s (GUID: %u) marked as member not existed guild (id: %u), removing guild membership for player.",pCurrChar->GetName().c_str(),pCurrChar->GetGUIDLow(),pCurrChar->GetGuildId());
            pCurrChar->SetInGuild(0);
        }
    }

    if(!pCurrChar->IsAlive())
        pCurrChar->SendCorpseReclaimDelay(true);

    pCurrChar->SendInitialPacketsBeforeAddToMap();

    //Show cinematic at the first time that player login
    if( !pCurrChar->getCinematic() )
    {
        pCurrChar->setCinematic(1);

        ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(pCurrChar->GetRace());
        if(rEntry)
        {
            data.Initialize( SMSG_TRIGGER_CINEMATIC,4 );
            data << uint32(rEntry->startmovie);
            SendPacket( &data );

            // send new char string if not empty
            if (!sWorld->GetNewCharString().empty())
                chH.PSendSysMessage("%s",sWorld->GetNewCharString().c_str());
        }
    }

    if (!pCurrChar->GetMap()->Add(pCurrChar))
    {
        AreaTrigger const* at = sObjectMgr->GetGoBackTrigger(pCurrChar->GetMapId());
        if(at)
            pCurrChar->TeleportTo(at->target_mapId, at->target_X, at->target_Y, at->target_Z, pCurrChar->GetOrientation());
        else
            pCurrChar->TeleportTo(pCurrChar->m_homebindMapId, pCurrChar->m_homebindX, pCurrChar->m_homebindY, pCurrChar->m_homebindZ, pCurrChar->GetOrientation());
    }

    sObjectAccessor->AddObject(pCurrChar);
    //TC_LOG_DEBUG("FIXME","Player %s added to Map.",pCurrChar->GetName());
    pCurrChar->GetSocial()->SendSocialList();

    pCurrChar->SendInitialPacketsAfterAddToMap();

    CharacterDatabase.PExecute("UPDATE characters SET online = 1 WHERE guid = '%u'", pCurrChar->GetGUIDLow());
    LoginDatabase.PExecute("UPDATE account SET online = 1 WHERE id = '%u'", GetAccountId());
    pCurrChar->SetInGameTime( GetMSTime() );

    // friend status
    sSocialMgr->SendFriendStatus(pCurrChar, FRIEND_ONLINE, pCurrChar->GetGUIDLow(), true);

    if(sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB))
        pCurrChar->UpdateArenaTitles();

    // Place character in world (and load zone) before some object loading
    pCurrChar->LoadCorpse();

    // setting Ghost+speed if dead
    if (pCurrChar->m_deathState != ALIVE)
    {
        // not blizz like, we must correctly save and load player instead...
        if(pCurrChar->GetRace() == RACE_NIGHTELF)
            pCurrChar->CastSpell(pCurrChar, 20584, true, 0);// auras SPELL_AURA_INCREASE_SPEED(+speed in wisp form), SPELL_AURA_INCREASE_SWIM_SPEED(+swim speed in wisp form), SPELL_AURA_TRANSFORM (to wisp form)
        pCurrChar->CastSpell(pCurrChar, 8326, true, 0);     // auras SPELL_AURA_GHOST, SPELL_AURA_INCREASE_SPEED(why?), SPELL_AURA_INCREASE_SWIM_SPEED(why?)

        pCurrChar->SetMovement(MOVE_WATER_WALK);
    }

    pCurrChar->ContinueTaxiFlight();

    // Load pet if any and player is alive and not in taxi flight
    if(pCurrChar->IsAlive() && pCurrChar->m_taxi.GetTaxiSource()==0)
        pCurrChar->LoadPet();

    // Set FFA PvP for non GM in non-rest mode
    if(sWorld->IsFFAPvPRealm() && !pCurrChar->IsGameMaster() && !pCurrChar->HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_RESTING) )
        pCurrChar->SetFlag(PLAYER_FLAGS,PLAYER_FLAGS_FFA_PVP);

    if(pCurrChar->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP))
        pCurrChar->SetContestedPvP();

    pCurrChar->ClearFarsight();
    pCurrChar->RemoveAurasByType(SPELL_AURA_BIND_SIGHT);

    // Apply at_login requests
    if(pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_SPELLS))
    {
        pCurrChar->resetSpells();
        SendNotification(LANG_RESET_SPELLS);
    }

    if(pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_TALENTS))
    {
        pCurrChar->ResetTalents(true);
        SendNotification(LANG_RESET_TALENTS);
    }

    bool firstLogin = pCurrChar->HasAtLoginFlag(AT_LOGIN_FIRST);
    if (firstLogin)
        pCurrChar->RemoveAtLoginFlag(AT_LOGIN_FIRST);


    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_SET_DESERTER)) {
        pCurrChar->CastSpell(pCurrChar, 26013, true);
        pCurrChar->RemoveAtLoginFlag(AT_LOGIN_SET_DESERTER);
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login & ~'8' WHERE guid = %u", pCurrChar->GetGUIDLow());
        if (pCurrChar->IsDead())
            pCurrChar->ResurrectPlayer(1.f);
    }
    
    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_FLYS)) {
        pCurrChar->m_taxi.ResetTaximask();
        pCurrChar->InitTaxiNodesForLevel();
        pCurrChar->RemoveAtLoginFlag(AT_LOGIN_RESET_FLYS);
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login & ~'16' WHERE guid = %u", pCurrChar->GetGUIDLow());
    }

    if(pCurrChar->HasAtLoginFlag(AT_LOGIN_ALL_REP)) {
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(942),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(935),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(936),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(1011),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(970),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(967),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(989),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(932),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(934),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(1038),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(1077),42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(990),42999);

        // Factions depending on team, like cities and some more stuff
        switch(pCurrChar->GetTeam())
        {
        case TEAM_ALLIANCE:
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(72),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(47),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(69),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(930),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(730),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(978),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(54),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(946),42999);
            break;
        case TEAM_HORDE:
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(76),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(68),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(81),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(911),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(729),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(941),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(530),42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(947),42999);
            break;
        default:
            break;
        }
        pCurrChar->RemoveAtLoginFlag(AT_LOGIN_ALL_REP);
    }
    
    // announce group about member online (must be after add to player list to receive announce to self)
    if(Group *group = pCurrChar->GetGroup())
    {
        //pCurrChar->groupInfo.group->SendInit(this); // useless
        group->CheckLeader(pCurrChar->GetGUID(), false); //check leader login
        group->SendUpdate();
    }

    // show time before shutdown if shutdown planned.
    if(sWorld->IsShuttingDown())
        sWorld->ShutdownMsg(true,pCurrChar,sWorld->GetShutdownReason());

    if(sWorld->getConfig(CONFIG_ALL_TAXI_PATHS))
        pCurrChar->SetTaxiCheater(true);

    if(pCurrChar->IsGameMaster())
        SendNotification(LANG_GM_ON);

    std::string IP_str = GetRemoteAddress();
    //sLog->outChar("Account: %d (IP: %s) Login Character:[%s] (guid:%u)",
   //     GetAccountId(),IP_str.c_str(),pCurrChar->GetName() ,pCurrChar->GetGUIDLow());

    m_playerLoading = false;
    
    #ifdef PLAYERBOT
    if (!_player->GetPlayerbotAI())
    {
        _player->SetPlayerbotMgr(new PlayerbotMgr(_player));
        sRandomPlayerbotMgr.OnPlayerLogin(_player);
    }
    #endif

    //Hook for OnLogin Event
    sScriptMgr->OnPlayerLogin(pCurrChar, firstLogin);

    delete holder;
    
    //avoid bug abuse to enter in heroic instance without needed reputation level
    if (!sMapMgr->CanPlayerEnter(pCurrChar->GetMap()->GetId(), pCurrChar))
    {
        pCurrChar->RepopAtGraveyard();
    }
}

void WorldSession::HandleSetFactionAtWar( WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,4+1);

    TC_LOG_DEBUG("FIXME", "WORLD: Received CMSG_SET_FACTION_ATWAR" );

    uint32 repListID;
    uint8  flag;

    recvData >> repListID;
    recvData >> flag;

    FactionStateList::iterator itr = GetPlayer()->m_factions.find(repListID);
    if (itr == GetPlayer()->m_factions.end())
        return;

    // always invisible or hidden faction can't change war state
    if(itr->second.Flags & (FACTION_FLAG_INVISIBLE_FORCED|FACTION_FLAG_HIDDEN) )
        return;

    GetPlayer()->SetFactionAtWar(&itr->second,flag);
}

//I think this function is never used :/ I dunno, but i guess this opcode not exists
void WorldSession::HandleSetFactionCheat( WorldPacket & /*recvData*/ )
{
    
    
    //CHECK_PACKET_SIZE(recvData,4+4);

    //TC_LOG_DEBUG("FIXME","WORLD SESSION: HandleSetFactionCheat");
    /*
        uint32 FactionID;
        uint32 Standing;

        recvData >> FactionID;
        recvData >> Standing;

        std::list<struct Factions>::iterator itr;

        for(itr = GetPlayer()->factions.begin(); itr != GetPlayer()->factions.end(); ++itr)
        {
            if(itr->ReputationListID == FactionID)
            {
                itr->Standing += Standing;
                itr->Flags = (itr->Flags | 1);
                break;
            }
        }
    */
    GetPlayer()->UpdateReputation();
}

void WorldSession::HandleMeetingStoneInfo( WorldPacket & /*recvData*/ )
{
    
    
    TC_LOG_DEBUG("FIXME", "WORLD: Received CMSG_MEETING_STONE_INFO" );

    WorldPacket data(SMSG_MEETINGSTONE_SETQUEUE, 5);
    data << uint32(0) << uint8(6);
    SendPacket(&data);
}

void WorldSession::HandleTutorialFlag(WorldPacket& recvData)
{
    uint32 data;
    recvData >> data;

    uint8 index = uint8(data / 32);
    if (index >= MAX_ACCOUNT_TUTORIAL_VALUES)
        return;

    uint32 value = (data % 32);

    uint32 flag = GetTutorialInt(index);
    flag |= (1 << value);
    SetTutorialInt(index, flag);
}

void WorldSession::HandleTutorialClear(WorldPacket & /*recvData*/)
{
    

    for (uint8 i = 0; i < MAX_ACCOUNT_TUTORIAL_VALUES; ++i)
        SetTutorialInt(i, 0xFFFFFFFF);
}

void WorldSession::HandleTutorialReset( WorldPacket & /*recvData*/ )
{
    
    
    for (uint8 i = 0; i < MAX_ACCOUNT_TUTORIAL_VALUES; ++i)
        SetTutorialInt(i, 0x00000000);
}

void WorldSession::HandleSetWatchedFactionOpcode(WorldPacket & recvData)
{
    
    
    CHECK_PACKET_SIZE(recvData,4);

    TC_LOG_DEBUG("FIXME","WORLD: Received CMSG_SET_WATCHED_FACTION");
    uint32 fact;
    recvData >> fact;
    GetPlayer()->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, fact);
}

void WorldSession::HandleSetFactionInactiveOpcode(WorldPacket & recvData)
{
    
    
    CHECK_PACKET_SIZE(recvData,4+1);

    TC_LOG_DEBUG("FIXME","WORLD: Received CMSG_SET_FACTION_INACTIVE");
    uint32 replistid;
    uint8 inactive;
    recvData >> replistid >> inactive;

    FactionStateList::iterator itr = _player->m_factions.find(replistid);
    if (itr == _player->m_factions.end())
        return;

    _player->SetFactionInactive(&itr->second, inactive);
}

void WorldSession::HandleShowingHelmOpcode( WorldPacket & /*recvData*/ )
{
    
    
    TC_LOG_DEBUG("FIXME","CMSG_TOGGLE_HELM for %s", _player->GetName().c_str());
    _player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
}

void WorldSession::HandleShowingCloakOpcode( WorldPacket & /*recvData*/ )
{
    
    
    TC_LOG_DEBUG("FIXME","CMSG_TOGGLE_CLOAK for %s", _player->GetName().c_str());
    _player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
}

void WorldSession::HandleCharRenameOpcode(WorldPacket& recvData)
{
    
    
    CHECK_PACKET_SIZE(recvData, 8+1);

    CharacterRenameInfo renameInfo;

    recvData >> renameInfo.Guid
             >> renameInfo.Name;

    // prevent character rename to invalid name
    if (!normalizePlayerName(renameInfo.Name))
    {
        SendCharRename(CHAR_NAME_NO_NAME, renameInfo);
        return;
    }

    ResponseCodes res = ObjectMgr::CheckPlayerName(renameInfo.Name, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        SendCharRename(res, renameInfo);
        return;
    }

    // check name limitations
    if (GetSecurity() == SEC_PLAYER && /*TODORBAC !HasPermission(rbac::RBAC_PERM_SKIP_CHECK_CHARACTER_CREATION_RESERVEDNAME) && */ sObjectMgr->IsReservedName(renameInfo.Name))
    {
        SendCharRename(CHAR_NAME_RESERVED, renameInfo);
        return;
    }

    // Ensure that the character belongs to the current account, that rename at login is enabled
    // and that there is no character with the desired new name
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_FREE_NAME);

    stmt->setUInt32(0, GUID_LOPART(renameInfo.Guid));
    stmt->setUInt32(1, GetAccountId());
    stmt->setUInt16(2, AT_LOGIN_RENAME);
    stmt->setUInt16(3, AT_LOGIN_RENAME);
    stmt->setString(4, renameInfo.Name);

    delete _charRenameCallback.GetParam();
     _charRenameCallback.SetParam(new CharacterRenameInfo(std::move(renameInfo)));
    _charRenameCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::HandleChangePlayerNameOpcodeCallBack(PreparedQueryResult result, CharacterRenameInfo const* renameInfo)
{
    if (!result)
    {
        SendCharRename(CHAR_CREATE_ERROR, *renameInfo);
        return;
    }


    Field* fields = result->Fetch();

    uint32 guidLow      = fields[0].GetUInt32();
    std::string oldname = fields[1].GetString();

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("UPDATE characters set name = '%s', at_login = at_login & ~ %u WHERE guid ='%u'", renameInfo->Name.c_str(), uint32(AT_LOGIN_RENAME), guidLow);
    trans->PAppend("DELETE FROM character_declinedname WHERE guid ='%u'", guidLow);
    CharacterDatabase.CommitTransaction(trans);

    SendCharRename(RESPONSE_SUCCESS, *renameInfo);

    sWorld->UpdateGlobalNameData(guidLow, oldname, renameInfo->Name);
    sWorld->UpdateGlobalPlayerData(guidLow, PLAYER_UPDATE_DATA_NAME, renameInfo->Name);

    LogsDatabaseAccessor::CharacterRename(this, guidLow, oldname, renameInfo->Name, GetRemoteAddress());
}

void WorldSession::HandleSetPlayerDeclinedNames(WorldPacket& recvData)
{
    
    
    uint64 guid;

    CHECK_PACKET_SIZE(recvData, 8);
    recvData >> guid;

    // not accept declined names for unsupported languages
    std::string name;
    if(!sObjectMgr->GetPlayerNameByGUID(guid, name))
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    std::wstring wname;
    if(!Utf8toWStr(name, wname))
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    if(!isCyrillicCharacter(wname[0]))                      // name already stored as only single alphabet using
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    std::string name2;
    DeclinedName declinedname;

    CHECK_PACKET_SIZE(recvData, recvData.rpos() + 1);
    recvData >> name2;

    if(name2 != name)                                       // character have different name
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    for(int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
    {
        CHECK_PACKET_SIZE(recvData, recvData.rpos() + 1);
        recvData >> declinedname.name[i];
        if(!normalizePlayerName(declinedname.name[i]))
        {
            WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
            data << uint32(1);
            data << uint64(guid);
            SendPacket(&data);
            return;
        }
    }

    if(!ObjectMgr::CheckDeclinedNames(GetMainPartOfName(wname, 0), declinedname))
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    for(int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        CharacterDatabase.EscapeString(declinedname.name[i]);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM character_declinedname WHERE guid = '%u'", GUID_LOPART(guid));
    trans->PAppend("INSERT INTO character_declinedname (guid, genitive, dative, accusative, instrumental, prepositional) VALUES ('%u','%s','%s','%s','%s','%s')",
        GUID_LOPART(guid), declinedname.name[0].c_str(), declinedname.name[1].c_str(), declinedname.name[2].c_str(), declinedname.name[3].c_str(), declinedname.name[4].c_str());
    CharacterDatabase.CommitTransaction(trans);

    WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
    data << uint32(0);                                      // OK
    data << uint64(guid);
    SendPacket(&data);
}


void WorldSession::SendCharCreate(ResponseCodes result)
{
    WorldPacket data(SMSG_CHAR_CREATE, 1);
    data << uint8(result);
    SendPacket(&data);
}

void WorldSession::SendCharDelete(ResponseCodes result)
{
    WorldPacket data(SMSG_CHAR_DELETE, 1);
    data << uint8(result);
    SendPacket(&data);
}


void WorldSession::SendCharRename(ResponseCodes result, CharacterRenameInfo const& renameInfo)
{
    WorldPacket data(SMSG_CHAR_RENAME, 1 + 8 + renameInfo.Name.size() + 1);
    data << uint8(result);
    if (result == RESPONSE_SUCCESS)
    {
        data << renameInfo.Guid;
        data << renameInfo.Name;
    }
    SendPacket(&data);
}