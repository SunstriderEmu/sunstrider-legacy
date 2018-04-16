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
#include "Config.h"
#include "ScriptMgr.h"
#include "LogsDatabaseAccessor.h"
#include "GitRevision.h"
#include "CharacterCache.h"

#ifdef PLAYERBOT
#include "playerbot.h"
#include "PlayerbotAIConfig.h"
#endif

#ifdef PLAYERBOT

class PlayerbotLoginQueryHolder : public LoginQueryHolder
{
private:
    uint32 masterAccountId;
    PlayerbotHolder* playerbotHolder;

public:
    PlayerbotLoginQueryHolder(PlayerbotHolder* playerbotHolder, uint32 masterAccount, uint32 accountId, ObjectGuid guid)
        : LoginQueryHolder(accountId, ObjectGuid(guid)), masterAccountId(masterAccount), playerbotHolder(playerbotHolder) { }

public:
    uint32 GetMasterAccountId() const { return masterAccountId; }
    PlayerbotHolder* GetPlayerbotHolder() { return playerbotHolder; }
};

Player* PlayerbotHolder::AddPlayerBot(ObjectGuid playerGuid, uint32 masterAccount, bool testingBot)
{
    // has bot already been added?x
    Player* bot = ObjectAccessor::FindPlayer(playerGuid);

    if (bot && bot->IsInWorld())
        return nullptr;

    uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(playerGuid);
    if (accountId == 0)
        return nullptr;

    auto holder = new PlayerbotLoginQueryHolder(this, masterAccount, accountId, playerGuid);
    if(!holder->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        return nullptr;
    }

    QueryResultHolderFuture future = CharacterDatabase.DelayQueryHolder(holder);
    future.get();

    WorldSession* masterSession = masterAccount ? sWorld->FindSession(masterAccount) : nullptr;
    uint32 botAccountId = holder->GetAccountId();
    WorldSession *botSession = new WorldSession(botAccountId, BUILD_243, "rndbot", nullptr, SEC_PLAYER, 2, 0, LOCALE_enUS, 0, false);

    botSession->HandlePlayerLogin(holder); // will delete lqh

    bot = botSession->GetPlayer();
    if (!bot)
        return nullptr;

    PlayerbotMgr *mgr = bot->GetPlayerbotMgr();
    bot->SetPlayerbotMgr(nullptr);
    delete mgr;
    sRandomPlayerbotMgr.OnPlayerLogout(bot);

    bool allowed = false;
    if (botAccountId == masterAccount)
        allowed = true;
    else if (masterSession && sPlayerbotAIConfig.allowGuildBots && bot->GetGuildId() == masterSession->GetPlayer()->GetGuildId())
        allowed = true;
    else if (sPlayerbotAIConfig.IsInRandomAccountList(botAccountId))
        allowed = true;

    if(testingBot)
        OnBotLogin(bot, testingBot);
    else if (allowed)
        OnBotLogin(bot);
    else if (masterSession)
    {
        ChatHandler ch(masterSession);
        ch.PSendSysMessage("You are not allowed to control bot %s...", bot->GetName().c_str());
        LogoutPlayerBot(bot->GetGUID());
    }
    return bot;
}

#endif

bool LoginQueryHolder::Initialize()
{
    SetSize(MAX_PLAYER_LOGIN_QUERY);

    bool res = true;
    ObjectGuid::LowType lowGuid = m_guid.GetCounter();

    // NOTE: all fields in `characters` must be read to prevent lost character data at next save in case wrong DB structure.
    // !!! NOTE: including unused `zone`,`online`
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_FROM, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GROUP_MEMBER);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_GROUP, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_INSTANCE);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_BOUND_INSTANCES, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_AURAS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_AURAS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SPELL);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_SPELLS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_QUESTSTATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_QUESTSTATUS_DAILY);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_DAILY_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_REPUTATION);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_REPUTATION, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_INVENTORY);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_INVENTORY, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_ACTIONS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_ACTIONS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_MAILCOUNT);
    stmt->setUInt32(0, lowGuid);
    stmt->setUInt64(1, uint64(time(nullptr)));
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_MAIL_COUNT, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_MAILDATE);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_MAIL_DATE, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SOCIALLIST);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_SOCIAL_LIST, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_HOMEBIND);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_HOME_BIND, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SPELLCOOLDOWNS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_SPELL_COOLDOWNS, stmt);

    if (sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_DECLINEDNAMES);
        stmt->setUInt32(0, lowGuid);
        res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_DECLINED_NAMES, stmt);
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_MEMBER);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_GUILD, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_ARENAINFO);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_ARENA_INFO, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SKILLS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_SKILLS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_BGDATA);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_BG_DATA, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CORPSE_LOCATION);
    stmt->setUInt64(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_CORPSE_LOCATION, stmt);

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
            ObjectGuid::LowType guidlow = (*result)[0].GetUInt32();
            TC_LOG_INFO("network", "Loading char guid %u from account %u.", guidlow, GetAccountId());
            if (Player::BuildEnumData(result, &data, this))
            {
                // Do not allow banned characters to log in
                if (!(*result)[20].GetUInt32())
                    _legitCharacters.insert(guidlow);

                /*       0        1      2        3      4
                SELECT c.guid, c.name, c.race, c.class, c.gender, c.playerBytes, c.playerBytes2, c.level, c.zone, c.map, c.position_x, c.position_y, c.position_z, "
                "gm.guildid, c.playerFlags, c.at_login, cp.entry, cp.modelid, cp.level, c.equipmentCache, cb.guid "
                */
                if (!sCharacterCache->HasCharacterCacheEntry(guidlow)) // This can happen if characters are inserted into the database manually. Core hasn't loaded name data yet.
                {
                    std::string name = (*result)[1].GetString();
                    uint8 gender = (*result)[4].GetUInt8();
                    uint8 race = (*result)[2].GetUInt8();
                    uint8 playerclass = (*result)[3].GetUInt8();
                    uint8 level = (*result)[7].GetUInt8();
                    sCharacterCache->AddCharacterCacheEntry(guidlow, GetAccountId(), name, gender, race, playerclass, level, 0);
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

    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt).WithPreparedCallback(std::bind(&WorldSession::HandleCharEnum, this, std::placeholders::_1)));
}

void WorldSession::HandleCharCreateOpcode( WorldPacket & recvData )
{
    std::shared_ptr<CharacterCreateInfo> createInfo = std::make_shared<CharacterCreateInfo>();

    recvData >> createInfo->Name
             >> createInfo->Race
             >> createInfo->Class
             >> createInfo->Gender
             >> createInfo->Skin
             >> createInfo->Face
             >> createInfo->HairStyle
             >> createInfo->HairColor
             >> createInfo->FacialHair
             >> createInfo->OutfitId;

    //still got data to extract for packet but lets perform some checks first

    if(GetSecurity() == SEC_PLAYER)
    {
        if(uint32 mask = sWorld->getConfig(CONFIG_CHARACTERS_CREATING_DISABLED))
        {
            bool disabled = false;

            uint32 team = Player::TeamForRace(createInfo->Race);
            switch(team)
            {
                case ALLIANCE: disabled = mask & (1<<0); break;
                case HORDE:    disabled = mask & (1<<1); break;
            }

            if(disabled)
            {
                SendCharCreate(CHAR_CREATE_DISABLED);
                return;
            }
        }
    }

    ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(createInfo->Class);
    ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(createInfo->Race);

    if( !classEntry || !raceEntry )
    {
        SendCharCreate(CHAR_CREATE_FAILED);
        TC_LOG_ERROR("network","Class: %u or Race %u not found in DBC (Wrong DBC files?) or Cheater?", createInfo->Class, createInfo->Race);
        return;
    }

    // prevent character creating Expansion race without Expansion account
    if (raceEntry->addon > Expansion())
    {
        SendCharCreate(CHAR_CREATE_EXPANSION);
        TC_LOG_ERROR("network","Not Expansion 1 account:[%d] but tried to Create character with expansion 1 race (%u)", GetAccountId(), createInfo->Race);
        return;
    }

    // prevent character creating Expansion class without Expansion account
    // TODO: use possible addon field in ChrClassesEntry in next dbc version
    if (Expansion() < 2 && createInfo->Class == CLASS_DEATH_KNIGHT)
    {
        SendCharCreate(CHAR_CREATE_EXPANSION);
        TC_LOG_ERROR("network","Not Expansion 2 account:[%d] but tried to Create character with expansion 2 class (%u)", GetAccountId(), createInfo->Class);
        return;
    }

    // prevent character creating with invalid name
    if(!normalizePlayerName(createInfo->Name))
    {
        SendCharCreate(CHAR_NAME_INVALID_CHARACTER);
        TC_LOG_ERROR("network","Account:[%d] but tried to Create character with empty [name] ",GetAccountId());
        return;
    }

    // check name limitations
    ResponseCodes res = ObjectMgr::CheckPlayerName(createInfo->Name, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        SendCharCreate(res);
        return;
    }

    if(GetSecurity() == SEC_PLAYER && sObjectMgr->IsReservedName(createInfo->Name))
    {
        SendCharCreate(CHAR_NAME_RESERVED);
        return;
    }

    // Reserve the name for the duration of callback chain
    createInfo->NameToken = sCharacterCache->TryCreateCharacterWithName(createInfo->Name);
    if (!createInfo->NameToken)
    {
        SendCharCreate(CHAR_CREATE_NAME_IN_USE);
        return;
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
    stmt->setString(0, createInfo->Name);
    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt)
        .WithChainingPreparedCallback([this](QueryCallback& queryCallback, PreparedQueryResult result)
    {
        if (result)
        {
            SendCharCreate(CHAR_CREATE_NAME_IN_USE);
            return;
        }

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_SUM_REALM_CHARACTERS);
        stmt->setUInt32(0, GetAccountId());
        queryCallback.SetNextQuery(LoginDatabase.AsyncQuery(stmt));
    })
        .WithChainingPreparedCallback([this](QueryCallback& queryCallback, PreparedQueryResult result)
    {
        uint64 acctCharCount = 0;
        if (result)
        {
            Field* fields = result->Fetch();
            acctCharCount = uint64(fields[0].GetDouble());
        }

        if (acctCharCount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_ACCOUNT))
        {
            SendCharCreate(CHAR_CREATE_ACCOUNT_LIMIT);
            return;
        }

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_SUM_CHARS);
        stmt->setUInt32(0, GetAccountId());
        queryCallback.SetNextQuery(CharacterDatabase.AsyncQuery(stmt));
    })
        .WithChainingPreparedCallback([this, createInfo](QueryCallback& queryCallback, PreparedQueryResult result)
    {
        if (result)
        {
            Field* fields = result->Fetch();
            createInfo->CharCount = uint8(fields[0].GetUInt64()); // SQL's COUNT() returns uint64 but it will always be less than uint8.Max

            if (createInfo->CharCount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
            {
                SendCharCreate(CHAR_CREATE_SERVER_LIMIT);
                return;
            }
        }

        //TC // bool allowTwoSideAccounts = !sWorld->IsPvPRealm() || HasPermission(rbac::RBAC_PERM_TWO_SIDE_CHARACTER_CREATION);
        bool allowTwoSideAccounts = !sWorld->IsPvPRealm() || sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS) || GetSecurity() > SEC_PLAYER;
        uint32 skipCinematics = sWorld->getConfig(CONFIG_SKIP_CINEMATICS);

        std::function<void(PreparedQueryResult)> finalizeCharacterCreation = [this, createInfo](PreparedQueryResult result)
        {
            bool haveSameRace = false;
            //TC // bool allowTwoSideAccounts = !sWorld->IsPvPRealm() || HasPermission(rbac::RBAC_PERM_TWO_SIDE_CHARACTER_CREATION);
            bool allowTwoSideAccounts = !sWorld->IsPvPRealm() || sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS) || GetSecurity() > SEC_PLAYER;
            uint32 skipCinematics = sWorld->getConfig(CONFIG_SKIP_CINEMATICS);

            if (result)
            {
                uint32 team = Player::TeamForRace(createInfo->Race);

                Field* field = result->Fetch();
                uint8 accRace = field[1].GetUInt8();

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
                        return;
                    }
                }

                // search same race for cinematic or same class if need
                /// @todo check if cinematic already shown? (already logged in?; cinematic field)
                while (skipCinematics == 1 && !haveSameRace)
                {
                    if (!result->NextRow())
                        break;

                    field = result->Fetch();
                    accRace = field[1].GetUInt8();

                    if (!haveSameRace)
                        haveSameRace = createInfo->Race == accRace;
                }
            }

            Player newChar(this);
            newChar.GetMotionMaster()->Initialize();
            if (!newChar.Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(), createInfo.get()))
            {
                // Player not create (race/class/etc problem?)
                newChar.CleanupsBeforeDelete();

                SendCharCreate(CHAR_CREATE_ERROR);
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
            stmt->setUInt32(1, realm.Id.Realm);
            trans->Append(stmt);

            stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_REALM_CHARACTERS);
            stmt->setUInt32(0, createInfo->CharCount);
            stmt->setUInt32(1, GetAccountId());
            stmt->setUInt32(2, realm.Id.Realm);
            trans->Append(stmt);

            LoginDatabase.CommitTransaction(trans);

            SendCharCreate(CHAR_CREATE_SUCCESS);

            std::string IP_str = GetRemoteAddress();
            TC_LOG_INFO("entities.player.character", "Account: %d (IP: %s) Create Character:[%s] (GUID: %u)", GetAccountId(), IP_str.c_str(), createInfo->Name.c_str(), newChar.GetGUID().GetCounter());
            //sScriptMgr->OnPlayerCreate(&newChar);
            sCharacterCache->AddCharacterCacheEntry(newChar.GetGUID().GetCounter(), GetAccountId(), newChar.GetName(), newChar.GetGender(), newChar.GetRace(), newChar.GetClass(), newChar.GetLevel(), 0);

            newChar.CleanupsBeforeDelete();
        };

        if (allowTwoSideAccounts && !skipCinematics)
        {
            finalizeCharacterCreation(PreparedQueryResult(nullptr));
            return;
        }

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_CREATE_INFO);
        stmt->setUInt32(0, GetAccountId());
        stmt->setUInt32(1, 10 /*(skipCinematics == 1) ? 10 : 1*/);
        queryCallback.WithPreparedCallback(std::move(finalizeCharacterCreation)).SetNextQuery(CharacterDatabase.AsyncQuery(stmt));
    }));
}

void WorldSession::HandleCharDeleteOpcode( WorldPacket & recvData )
{
    ObjectGuid guid;
    recvData >> guid;

    //uint32 initAccountId = GetAccountId();

    // can't delete loaded character
    if (ObjectAccessor::FindPlayer(guid))
    {
        //        sScriptMgr->OnPlayerFailedDelete(guid, initAccountId);
        return;
    }

    uint32 accountId = 0;
    uint8 level = 0;
    std::string name;

    // is guild leader
    if(sObjectMgr->IsGuildLeader(guid))
    {
        //        sScriptMgr->OnPlayerFailedDelete(guid, initAccountId);
        SendCharDelete(CHAR_DELETE_FAILED_GUILD_LEADER);
        return;
    }

    // is arena team captain
    if(sObjectMgr->IsArenaTeamCaptain(guid))
    {
        //        sScriptMgr->OnPlayerFailedDelete(guid, initAccountId);
        SendCharDelete(CHAR_DELETE_FAILED_ARENA_CAPTAIN);
        return;
    }

    QueryResult result = CharacterDatabase.PQuery("SELECT account, name, level FROM characters WHERE guid='%u'", guid.GetCounter());
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
    TC_LOG_DEBUG("entities.player","Account: %d (IP: %s) Delete Character:[%s] (guid:%u)",GetAccountId(),IP_str.c_str(),name.c_str(),guid.GetCounter());

    //    sScriptMgr->OnPlayerDelete(guid, initAccountId); // To prevent race conditioning, but as it also makes sense, we hand the accountId over for successful delete.

    std::string fname = sConfigMgr->GetStringDefault("LogsDir", "");
    if (fname.length() > 0 && fname.at(fname.length()-1) != '/')
        fname.append("/");
    char fpath[64];
    snprintf(fpath, 64, "chardump/%d_%d_%s", GetAccountId(), guid.GetCounter(), name.c_str());
    fname.append(fpath);
    PlayerDumpWriter().WriteDump(fname, guid.GetCounter());

    LogsDatabaseAccessor::CharacterDelete(this, guid.GetCounter(), name, level, GetRemoteAddress());

#ifdef LICH_LING
    sCalendarMgr->RemoveAllPlayerEventsAndInvites(guid);
#endif
    Player::DeleteFromDB(guid, GetAccountId());

    SendCharDelete(CHAR_DELETE_SUCCESS);
}

void WorldSession::HandlePlayerLoginOpcode( WorldPacket & recvData )
{
    if(PlayerLoading() || GetPlayer() != nullptr)
    {
        //TC_LOG_ERROR("network.opcode","Player tryes to login again, AccountId = %d",GetAccountId());
        return;
    }

    m_playerLoading = true;
    ObjectGuid playerGuid;

   // TC_LOG_DEBUG( "network.opcode","WORLD: Recvd Player Logon Message" );

    recvData >> playerGuid;

    auto holder = new LoginQueryHolder(GetAccountId(), playerGuid);
    if(!holder->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        m_playerLoading = false;
        return;
    }

    _charLoginCallback = CharacterDatabase.DelayQueryHolder((SQLQueryHolder*)holder);
}

void WorldSession::_HandlePlayerLogin(Player* pCurrChar, LoginQueryHolder* holder)
{
    // for send server info and strings (config)
    ChatHandler chH = ChatHandler(pCurrChar);

    pCurrChar->GetMotionMaster()->Initialize();

    SetPlayer(pCurrChar);

    pCurrChar->SendDungeonDifficulty(false);

    WorldPacket data(SMSG_LOGIN_VERIFY_WORLD, 20);
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
        if (sWorld->getConfig(CONFIG_ENABLE_SINFO_LOGIN) == 1)
            chH.PSendSysMessage(GitRevision::GetFullVersion());

        //TC_LOG_DEBUG( "network", "WORLD: Sent server info" );
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

    //QueryResult result = CharacterDatabase.PQuery("SELECT guildid,rank FROM guild_member WHERE guid = '%u'",pCurrChar->GetGUID().GetCounter());

    uint32 guildId = 0;
    if (PreparedQueryResult resultGuild = holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_GUILD))
    {
        Field* fields = resultGuild->Fetch();
        guildId = fields[0].GetUInt32();
        pCurrChar->SetInGuild(guildId);
        pCurrChar->SetRank(fields[1].GetUInt8());
    }
    else if (pCurrChar->GetGuildId())                        // clear guild related fields in case wrong data about non existed membership
    {
        pCurrChar->SetInGuild(0);
        pCurrChar->SetRank(0);
    }

    // Default guild for GMs
    if (GetSecurity() > SEC_PLAYER)
    {
        //if character has no guild and is GM, set him in default gm guild
        if (uint32 defaultGuildId = sWorld->getConfig(CONFIG_GM_DEFAULT_GUILD))
        {
            if (guildId == 0)
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
                        TC_LOG_ERROR("entities.player", "Could not add player to default guild Id %u for GM player %s (%u)", defaultGuildId, pCurrChar->GetName().c_str(), pCurrChar->GetGUID().GetCounter());
                    }
                    CharacterDatabase.CommitTransaction(trans);
                }
                else {
                    TC_LOG_ERROR("entities.player", "Could not find default guild Id %u for GM player %s (%u)", defaultGuildId, pCurrChar->GetName().c_str(), pCurrChar->GetGUID().GetCounter());
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
                    currentGuild->DeleteMember(pCurrChar->GetGUID());

                if (Guild* gmGuild = sObjectMgr->GetGuildById(GMForcedGuildId))
                {
                    if (gmGuild->AddMember(pCurrChar->GetGUID(), gmGuild->GetLowestRank()))
                    {
                        pCurrChar->SetInGuild(GMForcedGuildId);
                        pCurrChar->SetRank(gmGuild->GetLowestRank());
                    }
                    else {
                        TC_LOG_ERROR("entities.player", "Could not add player to forced guild Id %u for GM player %s (%u)", GMForcedGuildId, pCurrChar->GetName().c_str(), pCurrChar->GetGUID().GetCounter());
                    }
                }
            }
        }
    }

    if (pCurrChar->GetGuildId() != 0)
    {
        Guild* guild = sObjectMgr->GetGuildById(pCurrChar->GetGuildId());
        if (guild)
        {
            data.Initialize(SMSG_GUILD_EVENT, (2 + guild->GetMOTD().size() + 1));
            data << (uint8)GE_MOTD;
            data << (uint8)1;
            data << guild->GetMOTD();
            SendPacket(&data);
            TC_LOG_DEBUG("network", "WORLD: Sent guild-motd (SMSG_GUILD_EVENT)");

            data.Initialize(SMSG_GUILD_EVENT, (5 + 10));      // we guess size
            data << (uint8)GE_SIGNED_ON;
            data << (uint8)1;
            data << pCurrChar->GetName();
            data << pCurrChar->GetGUID();
            guild->BroadcastPacket(&data);

            TC_LOG_DEBUG("network", "WORLD: Sent guild-signed-on (SMSG_GUILD_EVENT)");

            // Increment online members of the guild
            guild->IncOnlineMemberCount();
        }
        else
        {
            // remove wrong guild data
            TC_LOG_ERROR("network", "Player %s (GUID: %u) marked as member not existed guild (id: %u), removing guild membership for player.", pCurrChar->GetName().c_str(), pCurrChar->GetGUID().GetCounter(), pCurrChar->GetGuildId());
            pCurrChar->SetInGuild(0);
        }
    }

    if (GetClientBuild() == BUILD_335)
    {
        data.Initialize(/*SMSG_LEARNED_DANCE_MOVES*/ 0x455, 4 + 4);
        data << uint32(0);
        data << uint32(0);
        SendPacket(&data);
    }

    if (pCurrChar->HasCorpse())
    {
        int32 corpseReclaimDelay = pCurrChar->CalculateCorpseReclaimDelay();
        if (corpseReclaimDelay >= 0)
            pCurrChar->SendCorpseReclaimDelay(corpseReclaimDelay);
    }

    pCurrChar->SendInitialPacketsBeforeAddToMap();

    //Show cinematic at the first time that player login
    if (!pCurrChar->getCinematic())
    {
        pCurrChar->setCinematic(1);

#ifdef LICH_KING
        //LK has death knights intro handled here
        if (ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(pCurrChar->GetClass()))
        {
            if (cEntry->cinematicSequence)
                pCurrChar->SendCinematicStart(cEntry->cinematicSequence);
            else
#endif
                if (ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(pCurrChar->GetRace()))
                    pCurrChar->SendCinematicStart(rEntry->cinematicSequence);

            // send new char string if not empty
            if (!sWorld->GetNewCharString().empty())
                chH.PSendSysMessage("%s", sWorld->GetNewCharString().c_str());
#ifdef LICH_KING
        }
#endif
    }

    if (!pCurrChar->GetMap()->AddPlayerToMap(pCurrChar))
    {
        AreaTrigger const* at = sObjectMgr->GetGoBackTrigger(pCurrChar->GetMapId());
        if (at)
            pCurrChar->TeleportTo(at->target_mapId, at->target_X, at->target_Y, at->target_Z, pCurrChar->GetOrientation());
        else
            pCurrChar->TeleportTo(pCurrChar->m_homebindMapId, pCurrChar->m_homebindX, pCurrChar->m_homebindY, pCurrChar->m_homebindZ, pCurrChar->GetOrientation());
    }

    ObjectAccessor::AddObject(pCurrChar);
    //TC_LOG_DEBUG("FIXME","Player %s added to Map.",pCurrChar->GetName());
    if (!pCurrChar->GetSocial())
        pCurrChar->m_social = sSocialMgr->GetDefault(pCurrChar->GetGUID().GetCounter());
    else 
        pCurrChar->GetSocial()->SendSocialList();

    pCurrChar->SendInitialPacketsAfterAddToMap();

    CharacterDatabase.PExecute("UPDATE characters SET online = 1 WHERE guid = '%u'", pCurrChar->GetGUID().GetCounter());
    LoginDatabase.PExecute("UPDATE account SET online = 1 WHERE id = '%u'", GetAccountId());
    pCurrChar->SetInGameTime(GetMSTime());

    // friend status
    sSocialMgr->SendFriendStatus(pCurrChar, FRIEND_ONLINE, pCurrChar->GetGUID().GetCounter(), true);

    if (sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB))
        pCurrChar->UpdateArenaTitles();

    // Place character in world (and load zone) before some object loading
    pCurrChar->LoadCorpse(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_CORPSE_LOCATION));

    // setting Ghost+speed if dead
    if (pCurrChar->m_deathState != ALIVE)
    {
        // not blizz like, we must correctly save and load player instead...
        if (pCurrChar->GetRace() == RACE_NIGHTELF)
            pCurrChar->CastSpell(pCurrChar, 20584, true);// auras SPELL_AURA_INCREASE_SPEED(+speed in wisp form), SPELL_AURA_INCREASE_SWIM_SPEED(+swim speed in wisp form), SPELL_AURA_TRANSFORM (to wisp form)
        pCurrChar->CastSpell(pCurrChar, 8326, true);     // auras SPELL_AURA_GHOST, SPELL_AURA_INCREASE_SPEED(why?), SPELL_AURA_INCREASE_SWIM_SPEED(why?)

        pCurrChar->SetMovement(MOVE_WATER_WALK);
    }

    pCurrChar->ContinueTaxiFlight();

    // Load pet if any and player is alive and not in taxi flight
    if (pCurrChar->IsAlive() && pCurrChar->m_taxi.GetTaxiSource() == 0)
        pCurrChar->LoadPet();

    // Set FFA PvP for non GM in non-rest mode
    if (sWorld->IsFFAPvPRealm() && !pCurrChar->IsGameMaster() && !pCurrChar->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
        pCurrChar->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);

    if (pCurrChar->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP))
        pCurrChar->SetContestedPvP();

    pCurrChar->RemoveAurasByType(SPELL_AURA_BIND_SIGHT);

    // Apply at_login requests
    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_SPELLS))
    {
        pCurrChar->resetSpells();
        SendNotification(LANG_RESET_SPELLS);
    }

    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_TALENTS))
    {
        pCurrChar->ResetTalents(true);
        SendNotification(LANG_RESET_TALENTS);
    }

    bool firstLogin = pCurrChar->HasAtLoginFlag(AT_LOGIN_FIRST);
    if (firstLogin)
        pCurrChar->RemoveAtLoginFlag(AT_LOGIN_FIRST);


    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_SET_DESERTER)) 
    {
        pCurrChar->CastSpell(pCurrChar, 26013, true);
        pCurrChar->RemoveAtLoginFlag(AT_LOGIN_SET_DESERTER);
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login & ~'8' WHERE guid = %u", pCurrChar->GetGUID().GetCounter());
        if (pCurrChar->IsDead())
            pCurrChar->ResurrectPlayer(1.f);
    }

    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_FLYS)) 
    {
        pCurrChar->m_taxi.ResetTaximask();
        pCurrChar->InitTaxiNodesForLevel();
        pCurrChar->RemoveAtLoginFlag(AT_LOGIN_RESET_FLYS);
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login & ~'16' WHERE guid = %u", pCurrChar->GetGUID().GetCounter());
    }

    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_ALL_REP)) 
    {
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(942), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(935), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(936), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(1011), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(970), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(967), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(989), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(932), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(934), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(1038), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(1077), 42999);
        pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(990), 42999);

        // Factions depending on team, like cities and some more stuff
        switch (pCurrChar->GetTeam())
        {
        case ALLIANCE:
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(72), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(47), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(69), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(930), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(730), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(978), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(54), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(946), 42999);
            break;
        case HORDE:
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(76), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(68), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(81), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(911), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(729), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(941), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(530), 42999);
            pCurrChar->SetFactionReputation(sFactionStore.LookupEntry(947), 42999);
            break;
        default:
            break;
        }
        pCurrChar->RemoveAtLoginFlag(AT_LOGIN_ALL_REP);
    }

    // announce group about member online (must be after add to player list to receive announce to self)
    if (Group *group = pCurrChar->GetGroup())
    {
        group->CheckLeader(pCurrChar->GetGUID(), false); //check leader login
        group->SendUpdate();
#ifdef LICH_KING
        group->ResetMaxEnchantingLevel();
#endif
    }

    // show time before shutdown if shutdown planned.
    if (sWorld->IsShuttingDown())
        sWorld->ShutdownMsg(true, pCurrChar, sWorld->GetShutdownReason());

    if (sWorld->getConfig(CONFIG_ALL_TAXI_PATHS))
        pCurrChar->SetTaxiCheater(true);

    if (pCurrChar->IsGameMaster())
        SendNotification(LANG_GM_ON);

    std::string IP_str = GetRemoteAddress();
    //sLog->outChar("Account: %d (IP: %s) Login Character:[%s] (guid:%u)",
    //     GetAccountId(),IP_str.c_str(),pCurrChar->GetName() ,pCurrChar->GetGUID().GetCounter());

    m_playerLoading = false;

#ifdef PLAYERBOT
    if (!_player->GetPlayerbotAI())
    {
        _player->SetPlayerbotMgr(new PlayerbotMgr(_player));
        sRandomPlayerbotMgr.OnPlayerLogin(_player);
    }
#endif

    //Hook for OnLogin Event
    //    sScriptMgr->OnPlayerLogin(pCurrChar, firstLogin);

    delete holder;

    //avoid bug abuse to enter in heroic instance without needed reputation level
    if (sMapMgr->PlayerCannotEnter(pCurrChar->GetMap()->GetId(), pCurrChar))
    {
        pCurrChar->RepopAtGraveyard();
    }
}

void WorldSession::HandlePlayerLogin(LoginQueryHolder* holder)
{
    ObjectGuid playerGuid = holder->GetGuid();

    Player* pCurrChar = nullptr;
    pCurrChar = new Player(this);

    // "GetAccountId()==db stored account id" checked in LoadFromDB (prevent login not own character using cheating tools)
    if (!pCurrChar->LoadFromDB(playerGuid.GetCounter(), holder))
    {
        KickPlayer();                                       // disconnect client, player no set to session and it will not deleted or saved at kick
        delete pCurrChar;                                   // delete it manually
        delete holder;                                      // delete all unprocessed queries
        m_playerLoading = false;
        return;
    }

    _HandlePlayerLogin(pCurrChar, holder);
}

void WorldSession::HandleSetFactionAtWar( WorldPacket & recvData )
{


   // TC_LOG_DEBUG("network.opcode", "WORLD: Received CMSG_SET_FACTION_ATWAR" );

    uint32 repListID;
    uint8  flag;

    recvData >> repListID;
    recvData >> flag;

    auto itr = GetPlayer()->m_factions.find(repListID);
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
    //

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
   // TC_LOG_DEBUG("network.opcode", "WORLD: Received CMSG_MEETING_STONE_INFO" );

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


    //TC_LOG_DEBUG("network.opcode","WORLD: Received CMSG_SET_WATCHED_FACTION");
    uint32 fact;
    recvData >> fact;
    GetPlayer()->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, fact);
}

void WorldSession::HandleSetFactionInactiveOpcode(WorldPacket & recvData)
{


   // TC_LOG_DEBUG("network.opcode","WORLD: Received CMSG_SET_FACTION_INACTIVE");
    uint32 replistid;
    uint8 inactive;
    recvData >> replistid >> inactive;

    auto itr = _player->m_factions.find(replistid);
    if (itr == _player->m_factions.end())
        return;

    _player->SetFactionInactive(&itr->second, inactive);
}

void WorldSession::HandleShowingHelmOpcode( WorldPacket & /*recvData*/ )
{
    //TC_LOG_DEBUG("network.opcode","CMSG_TOGGLE_HELM for %s", _player->GetName().c_str());
    _player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
}

void WorldSession::HandleShowingCloakOpcode( WorldPacket & /*recvData*/ )
{
   // TC_LOG_DEBUG("network.opcode","CMSG_TOGGLE_CLOAK for %s", _player->GetName().c_str());
    _player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
}

void WorldSession::HandleCharRenameOpcode(WorldPacket& recvData)
{
    std::shared_ptr<CharacterRenameInfo> renameInfo = std::make_shared<CharacterRenameInfo>();

    recvData >> renameInfo->Guid
             >> renameInfo->Name;

    // prevent character rename to invalid name
    if (!normalizePlayerName(renameInfo->Name))
    {
        SendCharRename(CHAR_NAME_NO_NAME, renameInfo.get());
        return;
    }

    ResponseCodes res = ObjectMgr::CheckPlayerName(renameInfo->Name, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        SendCharRename(res, renameInfo.get());
        return;
    }

    // check name limitations
    if (GetSecurity() == SEC_PLAYER && /*TODORBAC !HasPermission(rbac::RBAC_PERM_SKIP_CHECK_CHARACTER_CREATION_RESERVEDNAME) && */ sObjectMgr->IsReservedName(renameInfo->Name))
    {
        SendCharRename(CHAR_NAME_RESERVED, renameInfo.get());
        return;
    }

    // Ensure that the character belongs to the current account, that rename at login is enabled
    // and that there is no character with the desired new name
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_FREE_NAME);

    stmt->setUInt32(0, renameInfo->Guid.GetCounter());
    stmt->setUInt32(1, GetAccountId());
    stmt->setString(2, renameInfo->Name);

    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt)
        .WithPreparedCallback(std::bind(&WorldSession::HandleCharRenameCallback, this, renameInfo, std::placeholders::_1)));
}

void WorldSession::HandleCharRenameCallback(std::shared_ptr<CharacterRenameInfo> renameInfo, PreparedQueryResult result)
{
    if (!result)
    {
        SendCharRename(CHAR_CREATE_ERROR, renameInfo.get());
        return;
    }

    Field* fields = result->Fetch();

    ObjectGuid::LowType guidLow      = fields[0].GetUInt32();
    std::string oldname = fields[1].GetString();
    uint16 atLoginFlags = fields[2].GetUInt16();

    if (!(atLoginFlags & AT_LOGIN_RENAME))
    {
        SendCharRename(CHAR_CREATE_ERROR, renameInfo.get());
        return;
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("UPDATE characters set name = '%s', at_login = at_login & ~ %u WHERE guid ='%u'", renameInfo->Name.c_str(), uint32(AT_LOGIN_RENAME), guidLow);
    trans->PAppend("DELETE FROM character_declinedname WHERE guid ='%u'", guidLow);
    CharacterDatabase.CommitTransaction(trans);

    //TC_LOG_INFO("entities.player.character", "Account: %d (IP: %s) Character:[%s] (%s) Changed name to: %s", GetAccountId(), GetRemoteAddress().c_str(), oldName.c_str(), renameInfo->Guid.ToString().c_str(), renameInfo->Name.c_str());

    SendCharRename(RESPONSE_SUCCESS, renameInfo.get());

    sCharacterCache->UpdateCharacterData(guidLow, PLAYER_UPDATE_DATA_NAME, renameInfo->Name);

    LogsDatabaseAccessor::CharacterRename(this, guidLow, oldname, renameInfo->Name, GetRemoteAddress());
}

void WorldSession::HandleSetPlayerDeclinedNames(WorldPacket& recvData)
{
    ObjectGuid guid;


    recvData >> guid;

    // not accept declined names for unsupported languages
    std::string name;
    if(!sCharacterCache->GetCharacterNameByGuid(guid, name))
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


    recvData >> name2;

    if(name2 != name)                                       // character have different name
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    for(auto & i : declinedname.name)
    {

        recvData >> i;
        if(!normalizePlayerName(i))
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

    for(auto & i : declinedname.name)
        CharacterDatabase.EscapeString(i);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM character_declinedname WHERE guid = '%u'", guid.GetCounter());
    trans->PAppend("INSERT INTO character_declinedname (guid, genitive, dative, accusative, instrumental, prepositional) VALUES ('%u','%s','%s','%s','%s','%s')",
        guid.GetCounter(), declinedname.name[0].c_str(), declinedname.name[1].c_str(), declinedname.name[2].c_str(), declinedname.name[3].c_str(), declinedname.name[4].c_str());
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


void WorldSession::SendCharRename(ResponseCodes result, CharacterRenameInfo const* renameInfo)
{
    WorldPacket data(SMSG_CHAR_RENAME, 1 + 8 + renameInfo->Name.size() + 1);
    data << uint8(result);
    if (result == RESPONSE_SUCCESS)
    {
        data << renameInfo->Guid;
        data << renameInfo->Name;
    }
    SendPacket(&data);
}
