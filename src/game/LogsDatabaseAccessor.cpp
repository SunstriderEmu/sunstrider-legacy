#include "LogsDatabaseAccessor.h"
#include "Player.h"
#include "WorldSession.h"
#include "DBCStores.h"
#include "World.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"

#define NO_SESSION_STRING "no session"

LogsDatabaseAccessor::LogsDatabaseAccessor() : max_trade_id(0)
{
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_SEL_CHAR_TRADE_MAX_ID);
    if (PreparedQueryResult result = LogsDatabase.Query(stmt))
    {
        max_trade_id = result->Fetch()[0].GetUInt64();
    }
    else {
        TC_LOG_FATAL("misc", "LogsDatabaseAccessor could get max trade id. Starting with trade id 0 again.");
    }
}

bool LogsDatabaseAccessor::ShouldLog(uint32 configDuration, uint32 configDurationGM, bool gmInvolved)
{
    uint32 duration = sWorld->getConfig(configDuration);
    uint32 gmDuration = configDurationGM ? sWorld->getConfig(configDurationGM) : 0;

    return (duration != 0 || (gmInvolved && (gmDuration != 0)));
}

void LogsDatabaseAccessor::Enchantment(Player const* caster, Player const* targetPlayer, uint32 itemGUIDLow, uint32 itemEntry, uint32 enchantID, bool permanent)
{
    bool gmInvolved = caster->GetSession()->GetSecurity() > SEC_PLAYER;

    if (!ShouldLog(CONFIG_LOG_CHAR_ITEM_ENCHANT, CONFIG_GM_LOG_CHAR_ITEM_ENCHANT, gmInvolved))
        return;

    // PrepareStatement(LOGS_INS_CHAR_ENCHANT, "INSERT INTO char_enchant (player_guid, target_player_guid, item_guid, item_entry, enchant_id, permanent, player_IP, target_player_IP, time, gm_involved) VALUES (?,?,?,?,?,?,?,?, UNIX_TIMESTAMP(),?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_ENCHANT);
    stmt->setUInt32(0, caster->GetGUIDLow());
    stmt->setUInt32(1, targetPlayer->GetGUIDLow());
    stmt->setUInt32(2, itemGUIDLow);
    stmt->setUInt32(3, itemEntry);
    stmt->setUInt32(4, enchantID);
    stmt->setBool(5, permanent);
    stmt->setString(6, caster->GetSession()->GetRemoteAddress());
    stmt->setString(7, targetPlayer->GetSession()->GetRemoteAddress());
    stmt->setBool(8, gmInvolved);
    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::BattlegroundStats(uint32 mapId, time_t start, time_t end, Team winner, uint32 scoreAlliance, uint32 scoreHorde)
{
    if (!ShouldLog(CONFIG_LOG_BG_STATS, 0, false))
        return;

    //PrepareStatement(LOGS_INS_BG_STATS, "INSERT INTO bg_stats (mapid, start_time, end_time, winner, score_alliance, score_horde) VALUES (?,?,UNIX_TIMESTAMP(),?,?,?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_BG_STATS);

    stmt->setUInt32(0, mapId);
    stmt->setUInt64(1, start);
    stmt->setUInt64(2, end);
    std::string winnerDB = "none";
    switch (winner)
    {
    case TEAM_HORDE:     winnerDB = "horde";    break;
    case TEAM_ALLIANCE:  winnerDB = "alliance"; break;
    case TEAM_NONE:      winnerDB = "none";     break;
    default:
        TC_LOG_ERROR("misc", "LogsDatabaseAccessor::LogBattlegroundStats Invalid team %u given. Setting winner to 'none'", winner);
        break;
    }
    stmt->setString(3, winnerDB);
    stmt->setUInt32(4, scoreAlliance);
    stmt->setUInt32(5, scoreHorde);

    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::BossDown(Creature const* victim, std::string const& bossName, std::string const& bossNameFr, uint32 downByGuildId, std::string const& guildName, uint32 guildPercentage, uint32 leaderGuid)
{
    if (!ShouldLog(CONFIG_LOG_BOSS_DOWNS, 0, false))
        return;

    //"INSERT INTO (boss_entry, boss_name, boss_name_fr, guild_id, guild_name, time, guild_percentage, leaderGuid) VALUES ( ? , ? , ? , ? , ? , UNIX_TIMESTAMP(), ? , ? )", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_BOSS_DOWN);

    stmt->setUInt32(0, victim->GetEntry());
    stmt->setString(1, bossName);
    stmt->setString(2, bossNameFr);
    stmt->setUInt32(3, downByGuildId);
    stmt->setString(4, guildName);
    stmt->setUInt32(5, guildPercentage);
    stmt->setUInt32(6, leaderGuid);

    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::CharacterDelete(WorldSession const* session, uint32 playerGUID, std::string const& charName, std::string const& IP)
{
    bool gmInvolved = session->GetSecurity() > SEC_PLAYER;

    if (!ShouldLog(CONFIG_LOG_CHAR_DELETE, CONFIG_GM_LOG_CHAR_DELETE, gmInvolved))
        return;

    //PrepareStatement(LOGS_INS_CHAR_DELETE, "INSERT INTO char_delete (account,guid,name,time,IP,gm_involved) VALUES (?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_DELETE);

    stmt->setUInt32(0, session->GetAccountId());
    stmt->setUInt32(1, playerGUID);
    stmt->setString(2, charName);
    stmt->setString(3, IP);
    stmt->setBool(4, gmInvolved);

    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::CharacterRename(WorldSession const* session, uint32 playerGUID, std::string const& oldName, std::string const& newName, std::string const& IP)
{
    bool gmInvolved = session->GetSecurity() > SEC_PLAYER;

    if (!ShouldLog(CONFIG_LOG_CHAR_RENAME, CONFIG_GM_LOG_CHAR_RENAME, gmInvolved))
        return;

    //PrepareStatement(LOGS_INS_CHAR_RENAME, "INSERT INTO char_rename (account, guid, old_name, new_name, time, IP, gm_involved) VALUES (?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_RENAME);
    stmt->setUInt32(0, session->GetAccountId());
    stmt->setUInt32(1, playerGUID);
    stmt->setString(2, oldName);
    stmt->setString(3, newName);
    stmt->setString(4, IP);
    stmt->setBool(5, gmInvolved);
    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::GMCommand(WorldSession const* m_session, Unit const* target, std::string const& fullcmd)
{
    if (!ShouldLog(LOGS_INS_GM_COMMAND, 0, false))
        return;

    Player* player = m_session ? m_session->GetPlayer() : nullptr;

    uint64 targetGUID = 0;
    if (target)
    {
        if (target->GetTypeId() == TYPEID_UNIT)
            targetGUID = MAKE_PAIR64(target->ToCreature()->GetDBTableGUIDLow(), HIGHGUID_UNIT);
        else if (target->GetTypeId() == TYPEID_GAMEOBJECT)
            targetGUID = MAKE_PAIR64(target->ToGameObject()->GetDBTableGUIDLow(), HIGHGUID_GAMEOBJECT);
        else
            targetGUID = target->GetGUID();
    }

    uint32 areaId = player ? player->GetAreaId() : 0;
    std::string areaName = "Unknown";
    std::string zoneName = "Unknown";
    if (player && m_session)
    {
        if (AreaTableEntry const* area = GetAreaEntryByAreaID(areaId))
        {
            int locale = m_session->GetSessionDbcLocale();
            areaName = area->area_name[locale];
            if (area->parentArea)
            {
                if (AreaTableEntry const* zone = GetAreaEntryByAreaID(area->parentArea))
                    zoneName = zone->area_name[locale];
            }
            else
                zoneName = areaName; //no parent area = the area is the zone (can an area have parents on two levels ? If so this is incorrect)
        }
    }

    /*PrepareStatement(LOGS_INS_GM_COMMAND, "INSERT INTO gm_command (account, guid, gmlevel, time, map, x, y, z, area_name, zone_name, selection_type, selection_guid,
    selection_name, selection_map, selection_x, selection_y, selection_z, command, IP) VALUES (?,?,?,UNIX_TIMESTAMP(),?,?,?,??,?,?,?,?,?,?,?,?,?,?)", CONNECTION_ASYNC); */
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_GM_COMMAND);
    stmt->setUInt32(0, m_session ? m_session->GetAccountId() : 0);
    stmt->setUInt32(1, player ? player->GetGUIDLow() : 0);
    stmt->setUInt8(2, m_session ? m_session->GetSecurity() : 0);
    stmt->setUInt32(3, player ? player->GetMapId() : 0);
    stmt->setFloat(4, player ? player->GetPositionX() : 0);
    stmt->setFloat(5, player ? player->GetPositionY() : 0);
    stmt->setFloat(6, player ? player->GetPositionZ() : 0);
    stmt->setString(7, areaName);
    stmt->setString(8, zoneName);
    stmt->setString(9, GetLogNameForGuid(targetGUID));
    stmt->setUInt32(10, GUID_LOPART(targetGUID));
    stmt->setString(11, (player && player->GetSelectedUnit()) ? player->GetSelectedUnit()->GetName() : "");
    stmt->setUInt32(12, (player && player->GetSelectedUnit()) ? player->GetSelectedUnit()->GetMapId() : 0);
    stmt->setFloat(13, (player && player->GetSelectedUnit()) ? player->GetSelectedUnit()->GetPositionX() : 0);
    stmt->setFloat(14, (player && player->GetSelectedUnit()) ? player->GetSelectedUnit()->GetPositionY() : 0);
    stmt->setFloat(15, (player && player->GetSelectedUnit()) ? player->GetSelectedUnit()->GetPositionZ() : 0);
    stmt->setString(16, fullcmd);
    stmt->setString(17, m_session ? m_session->GetRemoteAddress() : NO_SESSION_STRING);
    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::CharacterChat(ChatMsg type, Player const* player, Player const* toPlayer, uint32 logChannelId, std::string const& to, std::string const& msg)
{
    if (type == CHAT_MSG_ADDON)
        return;

    WorldSession const* session = player->GetSession();
    bool gmInvolved = (session->GetSecurity() > SEC_PLAYER) || (toPlayer && toPlayer->GetSession()->GetSecurity() > SEC_PLAYER);

    if (!ShouldLog(CONFIG_LOG_CHAR_CHAT, CONFIG_GM_LOG_CHAR_CHAT, gmInvolved))
        return;

    // PrepareStatement(LOGS_INS_CHAR_CHAT, "INSERT INTO char_chat (time,type,guid,target_guid,channelId,channelName,message,IP,gm_involved) VALUES (UNIX_TIMESTAMP(),?,?,?,?,?,?,?,?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_CHAT);

    stmt->setUInt8(0, uint8(type));
    stmt->setUInt32(1, session->GetAccountId());
    stmt->setUInt32(2, player->GetGUIDLow());
    stmt->setUInt32(3, toPlayer ? toPlayer->GetGUIDLow() : 0);
    stmt->setUInt32(4, logChannelId);
    stmt->setString(5, to);
    stmt->setString(6, msg);
    stmt->setString(7, session->GetRemoteAddress());
    stmt->setBool(8, gmInvolved);

    LogsDatabase.Execute(stmt);
}


//positive = deposit / negative = withdrawal
void LogsDatabaseAccessor::GuildMoneyTransfer(Player const* player, uint32 guildId, int32 money)
{
    bool gmInvolved = player->GetSession()->GetSecurity() > SEC_PLAYER;

    if (!ShouldLog(CONFIG_LOG_CHAR_GUILD_MONEY, CONFIG_GM_LOG_CHAR_GUILD_MONEY, gmInvolved))
        return;

    //PrepareStatement(LOGS_INS_CHAR_GUILD_MONEY, "INSERT INTO char_guild_money_deposit (account, guid, guildId, amount, time) VALUES (?,?,?,?,UNIX_TIMESTAMP())", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_GUILD_MONEY);

    stmt->setUInt32(0, player->GetSession()->GetAccountId());
    stmt->setUInt32(1, player->GetGUIDLow());
    stmt->setUInt32(2, guildId);
    stmt->setInt32(3, money);
    stmt->setString(4, player->GetSession()->GetRemoteAddress());
    stmt->setBool(5, gmInvolved);

    LogsDatabase.Execute(stmt);
}


void LogsDatabaseAccessor::GuildBankItemTransfer(Player const* player, bool deposit, uint32 itemGuid, uint32 itemEntry, uint8 itemCount)
{
    bool gmInvolved = player->GetSession()->GetSecurity() > SEC_PLAYER;

    if (!ShouldLog(CONFIG_LOG_CHAR_ITEM_GUILD_BANK, CONFIG_GM_LOG_CHAR_ITEM_GUILD_BANK, gmInvolved))
        return;

    //PrepareStatement(LOGS_INS_CHAR_ITEM_GUILD_BANK, "INSERT INTO char_item_guild_bank (account, guid, guildId, direction, item_guid, item_entry, item_count, time, IP, gm_involved) VALUES (?,?,?,?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_ITEM_GUILD_BANK);

    stmt->setUInt32(0, player->GetSession()->GetAccountId());
    stmt->setUInt32(1, player->GetGUIDLow());
    stmt->setUInt32(2, player->GetGuildId());
    stmt->setString(3, deposit ? "chartoguild" : "guildtochar");
    stmt->setUInt32(4, itemGuid);
    stmt->setUInt32(5, itemEntry);
    stmt->setUInt16(6, itemCount);
    stmt->setString(7, player->GetSession()->GetRemoteAddress());
    stmt->setBool(8, gmInvolved);

    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::CharacterItemDelete(Player const* player, Item const* item)
{
    bool gmInvolved = player->GetSession()->GetSecurity() > SEC_PLAYER;

    if (!ShouldLog(CONFIG_LOG_CHAR_ITEM_DELETE, CONFIG_GM_LOG_CHAR_ITEM_DELETE, gmInvolved))
        return;

    //PrepareStatement(LOGS_INS_CHAR_ITEM_DELETE, "INSERT INTO char_item_delete (account, playerguid, entry, count, time, IP, gm_involved) VALUES (?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_ITEM_DELETE);

    stmt->setUInt32(0, player->GetSession()->GetAccountId());
    stmt->setUInt32(1, player->GetGUIDLow());
    stmt->setUInt32(2, item->GetEntry());
    stmt->setUInt32(3, item->GetCount());
    stmt->setString(4, player->GetSession()->GetRemoteAddress());
    stmt->setBool(5, gmInvolved);

    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::Sanction(WorldSession const* authorSession, uint32 targetAccount, uint32 targetGUID, SanctionType type, uint32 durationSecs, std::string const& reason)
{
    if (!ShouldLog(CONFIG_LOG_SANCTIONS, 0, false))
        return;

    std::string targetIP = NO_SESSION_STRING;
    if (targetAccount)
    {
        if (WorldSession const* targetSession = sWorld->FindSession(targetAccount))
            targetIP = targetSession->GetRemoteAddress();
    }

    //PrepareStatement(LOGS_INS_SANCTIONS, "INSERT INTO gm_sanction (author_account, author_guid, target_account, target_guid, target_IP, type, duration, time, reason, IP) VALUES (?,?,?,?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_SANCTION);

    stmt->setUInt32(0, authorSession ? authorSession->GetAccountId() : 0);
    stmt->setUInt32(1, authorSession ? authorSession->GetPlayer()->GetGUIDLow() : 0);
    stmt->setUInt32(2, targetAccount);
    stmt->setUInt32(3, targetGUID);
    stmt->setString(4, targetIP);
    stmt->setUInt8(5, type);
    stmt->setUInt32(6, durationSecs);
    stmt->setString(7, reason);
    stmt->setString(8, authorSession ? authorSession->GetRemoteAddress() : NO_SESSION_STRING);

    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::RemoveSanction(WorldSession const* authorSession, uint32 targetAccount, uint32 targetGUID, std::string const& targetIP, SanctionType type)
{
    if (!ShouldLog(CONFIG_LOG_SANCTIONS, 0, false))
        return;

    //PrepareStatement(LOGS_INS_SANCTION_REMOVE, "INSERT INTO gm_sanction_remove  (author_account, author_guid, target_account, target_guid, target_IP, type, time, IP) VALUES (?,?,?,?,?,?,UNIX_TIMESTAMP(),?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_SANCTION_REMOVE);

    stmt->setUInt32(0, authorSession ? authorSession->GetAccountId() : 0);
    stmt->setUInt32(1, authorSession ? authorSession->GetPlayer()->GetGUIDLow() : 0);
    stmt->setUInt32(2, targetAccount);
    stmt->setUInt32(3, targetGUID);
    stmt->setString(4, targetIP);
    stmt->setUInt8(5, uint8(type));
    stmt->setString(6, authorSession ? authorSession->GetRemoteAddress() : NO_SESSION_STRING);

    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::Mail(uint32 mailId, MailMessageType type, uint32 sender_guidlow_or_entry, uint32 receiver_guidlow, std::string const& subject, uint32 itemTextId, MailItemsInfo const* mi, int32 money)
{
    //## Prepare some infos
    bool gmInvolved = false;
    std::string IP = NO_SESSION_STRING;
    Player const* sender = nullptr;
    if ((type == MAIL_NORMAL))
    {
        if ((sender = sObjectAccessor->FindPlayer(sender_guidlow_or_entry)))
        {
            IP = sender->GetSession()->GetRemoteAddress();
            if(sender->GetSession()->GetSecurity() > SEC_PLAYER)
                gmInvolved = true;
        }
    }
    else {
        gmInvolved = true;
    }

    if (!ShouldLog(CONFIG_LOG_CHAR_MAIL, CONFIG_GM_LOG_CHAR_MAIL, gmInvolved))
        return;

    std::string body = sObjectMgr->GetItemText(itemTextId);

    //## Insert into database

    SQLTransaction trans = LogsDatabase.BeginTransaction();

    //PrepareStatement(LOGS_INS_CHAR_MAIL, "INSERT INTO mail (id, type, sender_account, sender_guid_or_entry, receiver_guid, subject, message, money, time, IP, gm_involved) VALUES (?,?,?,?,?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC)
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_MAIL);
    stmt->setUInt32(0, mailId);
    stmt->setUInt8(1, uint8(type));
    stmt->setUInt32(2, sender ? sender->GetSession()->GetAccountId() : 0);
    stmt->setUInt32(3, sender_guidlow_or_entry);
    stmt->setUInt32(4, receiver_guidlow);
    stmt->setString(5, subject);
    stmt->setString(6, body);
    stmt->setInt32(7, money);
    stmt->setString(8, IP);
    stmt->setBool(9, gmInvolved);
    trans->Append(stmt);

    if (mi)
    {
        for (auto itr : *mi)
        {
            //PrepareStatement(LOGS_INS_CHAR_MAIL_ITEMS, "INSERT INTO mail_items (mail_id, item_guid, item_entry, item_count) VALUES (?,?,?,?)", CONNECTION_ASYNC);
            stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_MAIL_ITEMS);
            stmt->setUInt32(0, mailId);
            stmt->setUInt32(1, itr.second.item_guidlow);
            stmt->setUInt32(2, itr.second.item_template);
            stmt->setUInt16(3, itr.second.item->GetCount());
            trans->Append(stmt);
        }
    }

    LogsDatabase.CommitTransaction(trans);
}

void LogsDatabaseAccessor::CharacterTrade(Player const* p1, Player const* p2, std::vector<Item*> const& p1Items, std::vector<Item*> const& p2Items, uint32 p1Gold, uint32 p2Gold)
{
    uint32 tradeId = ++max_trade_id;

    bool gmInvolved = (p1->GetSession()->GetSecurity() > SEC_PLAYER) || (p2->GetSession()->GetSecurity() > SEC_PLAYER);

    if (!ShouldLog(CONFIG_LOG_CHAR_ITEM_TRADE, CONFIG_GM_LOG_CHAR_ITEM_TRADE, gmInvolved))
        return;

    SQLTransaction trans = LogsDatabase.BeginTransaction();

    //PrepareStatement(LOGS_INS_CHAR_TRADE, "INSERT INTO char_trade (id, player1_account, player2_account, player1_guid, player2_guid, money1, money2, player1_IP, player2_IP, time, gm_involved) VALUES (?,?,?,?,?,?,?,UNIX_TIMESTAMP(),?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_TRADE);
    stmt->setUInt32(0, max_trade_id);
    stmt->setUInt32(1, p1->GetSession()->GetAccountId());
    stmt->setUInt32(2, p2->GetSession()->GetAccountId());
    stmt->setUInt32(3, p1->GetGUIDLow());
    stmt->setUInt32(4, p2->GetGUIDLow());
    stmt->setUInt32(5, p1Gold);
    stmt->setUInt32(6, p2Gold);
    stmt->setString(7, p1->GetSession()->GetRemoteAddress());
    stmt->setString(8, p2->GetSession()->GetRemoteAddress());
    stmt->setBool(9, gmInvolved);
    trans->Append(stmt);

    auto logItemTrade = [&](Item* item, bool p1top2)
    { 
        if (!item)
            return;

        // PrepareStatement(LOGS_INS_CHAR_TRADE_ITEMS, "INSERT INTO char_trade_items (trade_id, p1top2, item_guid, item_entry, item_count) VALUES (?,?,?,?,?)", CONNECTION_ASYNC);
        stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_TRADE_ITEMS);

        stmt->setUInt32(0, tradeId);
        stmt->setBool(1, p1top2);
        stmt->setUInt32(2, item->GetGUIDLow());
        stmt->setUInt32(3, item->GetEntry());
        stmt->setUInt8(4, item->GetCount());

        trans->Append(stmt);
    };

    for (auto itr : p1Items)
        logItemTrade(itr, true);

    for (auto itr : p2Items)
        logItemTrade(itr, false);

    LogsDatabase.CommitTransaction(trans);
}

void LogsDatabaseAccessor::CleanupOldMonitorLogs()
{
    //0 means keep forever
    if (uint32 keepDays = sWorld->getIntConfig(CONFIG_MONITORING_KEEP_DURATION))
    {
        TC_LOG_DEBUG("sql.sql", "Cleaning old logs from monitoring system. ( > 1 month old)");

        time_t now = time(NULL);
        time_t limit = now - (keepDays * DAY);

        SQLTransaction trans = LogsDatabase.BeginTransaction();
        trans->PAppend("DELETE FROM mon_players WHERE time < %u", limit);
        trans->PAppend("DELETE FROM mon_timediff WHERE time < %u", limit);
        trans->PAppend("DELETE FROM mon_maps WHERE time < %u", limit);
        trans->PAppend("DELETE FROM mon_races WHERE time < %u", limit);
        trans->PAppend("DELETE FROM mon_classes WHERE time < %u", limit);
        LogsDatabase.CommitTransaction(trans);
    }
}

void LogsDatabaseAccessor::WonAuction(uint32 bidderAccount, uint32 bidderGUID, uint32 sellerAccount, uint32 sellerGUID, uint32 itemGUID, uint32 itemEntry, uint32 itemCount)
{
    uint32 sellerSecurity = sAccountMgr->GetSecurity(sellerAccount);
    uint32 bidderSecurity = sAccountMgr->GetSecurity(bidderAccount);
    bool gmInvolved = bidderSecurity > SEC_PLAYER || sellerSecurity > SEC_PLAYER;

    if (!ShouldLog(CONFIG_LOG_CHAR_ITEM_AUCTION, CONFIG_GM_LOG_CHAR_ITEM_AUCTION, gmInvolved))
        return;

    //PrepareStatement(LOGS_INS_CHAR_AUCTION_WON, "INSERT INTO char_auction_won (bidder_account, bidder_guid, seller_account, seller_guid, item_guid, item_entry, item_count, time, gm_involved) VALUES (?,?,?,?,?,?,?,UNIX_TIMESTAMP(),?", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_AUCTION_WON);

    stmt->setUInt32(0, bidderAccount);
    stmt->setUInt32(1, bidderGUID);
    stmt->setUInt32(2, sellerAccount);
    stmt->setUInt32(3, sellerGUID);
    stmt->setUInt32(4, itemGUID);
    stmt->setUInt32(5, itemEntry);
    stmt->setUInt16(6, itemCount);
    stmt->setBool(7, gmInvolved);

    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::CreateAuction(Player const* player, uint32 itemGUID, uint32 itemEntry, uint32 itemCount)
{
    WorldSession const* session = player->GetSession();
    uint32 accountId = session->GetAccountId();
    uint32 sellerSecurity = sAccountMgr->GetSecurity(accountId);
    bool gmInvolved = sellerSecurity > SEC_PLAYER;

    if (!ShouldLog(CONFIG_LOG_CHAR_ITEM_AUCTION, CONFIG_GM_LOG_CHAR_ITEM_AUCTION, gmInvolved))
        return;

    //PrepareStatement(LOGS_INS_CHAR_AUCTION_CREATE, "INSERT INTO char_auction_create (seller_account, seller_guid, item_guid, item_entry, item_count, time, IP, gm_involved) VALUES (?,?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_AUCTION_CREATE);
    
    stmt->setUInt32(0, accountId);
    stmt->setUInt32(1, player->GetGUIDLow());
    stmt->setUInt32(2, itemGUID);
    stmt->setUInt32(3, itemEntry);
    stmt->setUInt16(4, itemCount);
    stmt->setString(5, session->GetRemoteAddress());
    stmt->setBool(6, gmInvolved);

    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::BuyOrSellItemToVendor(BuyTransactionType type, Player const* player, Item const* item, Unit const* vendor)
{
    bool gmInvolved = player->GetSession()->GetSecurity() > SEC_PLAYER;

    if (!ShouldLog(CONFIG_LOG_CHAR_ITEM_VENDOR, CONFIG_GM_LOG_CHAR_ITEM_VENDOR, gmInvolved))
        return;

    std::string transaction_type;
    switch (type)
    {
    case TRANSACTION_BUY:      transaction_type = "buy";     break;
    case TRANSACTION_SELL:     transaction_type = "sell";    break;
    case TRANSACTION_BUYBACK:  transaction_type = "buyback"; break;
    default:
        //wrong type given, should never happen
        ASSERT(false);
        return;
    }

    //PrepareStatement(LOGS_INS_CHAR_ITEM_VENDOR, "INSERT INTO char_item_vendor (transaction_type, account, guid, item_entry, item_count, vendor_entry, time, IP, gm_involved) VALUES (?,?,?,?,?,?,UNIT_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PreparedStatement* stmt = LogsDatabase.GetPreparedStatement(LOGS_INS_CHAR_ITEM_VENDOR);

    stmt->setString(0, transaction_type);
    stmt->setUInt32(1, player->GetSession()->GetAccountId());
    stmt->setUInt32(2, player->GetGUIDLow());
    stmt->setUInt32(3, item->GetEntry());
    stmt->setUInt32(4, item->GetCount());
    stmt->setUInt32(5, vendor->GetEntry());
    stmt->setString(6, player->GetSession()->GetRemoteAddress());
    stmt->setBool(7, gmInvolved);

    LogsDatabase.Execute(stmt);
}

void LogsDatabaseAccessor::CleanupOldLogs()
{
    //0 means keep forever
    TC_LOG_DEBUG("sql.sql", "Cleaning old logs");

    time_t now = time(NULL);

    SQLTransaction trans = LogsDatabase.BeginTransaction();

    auto deleteOldLogs = [&](const char* table, uint32 configIndex, uint32 configGMIndex)
    {
        uint32 keepDuration;
        time_t limit;
        keepDuration = sWorld->getConfig(configIndex);
        if (keepDuration != -1)
        {
            limit = now - (keepDuration * DAY);
            trans->PAppend("DELETE FROM %s WHERE time < %u AND gm_involved = 0", table, limit);
        }
        keepDuration = sWorld->getConfig(configGMIndex);
        if (keepDuration != -1)
        {
            limit = now - (keepDuration * DAY);
            trans->PAppend("DELETE FROM %s WHERE time < %u AND gm_involved = 1", table, limit);
        }
    };

    auto deleteOldLogsCombined = [&](const char* table1, const char* table2, const char* joinColumn1, const char* joinColumn2, uint32 configIndex, uint32 configGMIndex)
    {
        uint32 keepDuration;
        time_t limit;
        keepDuration = sWorld->getConfig(configIndex);
        if (keepDuration != -1)
        {
            limit = now - (keepDuration * DAY);
            trans->PAppend("DELETE a, b FROM %s a LEFT JOIN %s b ON a.%s = b.%s WHERE a.time < %u and a.gm_involved = 0", table1, table2, joinColumn1, joinColumn2, limit);
        }
        keepDuration = sWorld->getConfig(configGMIndex);
        if (keepDuration != -1)
        {
            limit = now - (keepDuration * DAY);
            trans->PAppend("DELETE a, b FROM %s a LEFT JOIN %s b ON a.%s = b.%s WHERE a.time < %u and a.gm_involved = 1", table1, table2, joinColumn1, joinColumn2, limit);
        }
    };

    deleteOldLogs("char_chat", CONFIG_LOG_CHAR_CHAT, CONFIG_GM_LOG_CHAR_CHAT);
    deleteOldLogs("char_enchant", CONFIG_LOG_CHAR_ITEM_ENCHANT, CONFIG_GM_LOG_CHAR_ITEM_ENCHANT);
    deleteOldLogs("char_guild_money_deposit", CONFIG_LOG_CHAR_GUILD_MONEY, CONFIG_GM_LOG_CHAR_GUILD_MONEY);
    deleteOldLogs("char_item_delete", CONFIG_LOG_CHAR_ITEM_DELETE, CONFIG_GM_LOG_CHAR_ITEM_DELETE);
    deleteOldLogs("char_item_guild_bank", CONFIG_LOG_CHAR_ITEM_GUILD_BANK, CONFIG_GM_LOG_CHAR_ITEM_GUILD_BANK);
    deleteOldLogs("char_rename", CONFIG_LOG_CHAR_RENAME, CONFIG_GM_LOG_CHAR_RENAME);

    deleteOldLogsCombined("char_trade", "char_trade_items", "id", "trade_id", CONFIG_LOG_CHAR_ITEM_TRADE, CONFIG_GM_LOG_CHAR_ITEM_TRADE);
    deleteOldLogsCombined("mail", "mail_items", "id", "mail_id", CONFIG_LOG_CHAR_MAIL, CONFIG_GM_LOG_CHAR_MAIL);

    LogsDatabase.CommitTransaction(trans);
}
