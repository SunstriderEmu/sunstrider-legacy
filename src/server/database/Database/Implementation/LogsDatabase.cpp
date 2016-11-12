#include "LogsDatabase.h"

void LogsDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_LOGSDATABASE_STATEMENTS);

    //PrepareStatement(LOGS_INS_ARENA_MATCH, "INSERT INTO arena_match ", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_BOSS_DOWN, "INSERT INTO boss_down (boss_entry, boss_name, boss_name_fr, guild_id, guild_name, time, guild_percentage, leaderGuid) VALUES (?,?,?,?,?, UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_BG_STATS, "INSERT INTO bg_stats (mapid, start_time, end_time, winner, score_alliance, score_horde) VALUES (?,?,?,?,?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_DELETE, "INSERT INTO char_delete (account,guid,name,time,IP,gm_involved) VALUES (?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_CHAT, "INSERT INTO char_chat (time,type,guid,account,target_guid,channelId,channelName,message,IP,gm_involved) VALUES (UNIX_TIMESTAMP(),?,?,?,?,?,?,?,?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_GUILD_MONEY, "INSERT INTO char_guild_money_deposit (account, guid, guildId, amount, time, IP,gm_involved) VALUES (?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_ITEM_DELETE, "INSERT INTO char_item_delete (account, playerguid, entry, count, time, IP,gm_involved) VALUES (?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_ITEM_GUILD_BANK, "INSERT INTO char_item_guild_bank (account, guid, guildId, direction, item_guid, item_entry, item_count, time, IP, gm_involved) VALUES (?,?,?,?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_MAIL, "INSERT INTO mail (id, type, sender_account, sender_guid_or_entry, receiver_guid, subject, message, money, time, IP, gm_involved) VALUES (?,?,?,?,?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_MAIL_ITEMS, "INSERT INTO mail_items (mail_id, id, item_guid, item_entry, item_count) VALUES (?,?,?,?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_RENAME, "INSERT INTO char_rename (account, guid, old_name, new_name, time, IP, gm_involved) VALUES (?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_SEL_CHAR_TRADE_MAX_ID, "SELECT MAX(id) FROM char_trade", CONNECTION_SYNCH);
    PrepareStatement(LOGS_INS_CHAR_TRADE, "INSERT INTO char_trade (id, player1_account, player2_account, player1_guid, player2_guid, money1, money2, player1_IP, player2_IP, time, gm_involved) VALUES (?,?,?,?,?,?,?,?,?,UNIX_TIMESTAMP(),?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_TRADE_ITEMS, "INSERT INTO char_trade_items (trade_id, p1top2, item_guid, item_entry, item_count) VALUES (?,?,?,?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_GM_COMMAND, "INSERT INTO gm_command (account, guid, gmlevel, time, map, x, y, z, area_name, zone_name, selection_type, selection_guid, selection_name, selection_map, selection_x, selection_y, selection_z, command, IP) VALUES (?,?,?,UNIX_TIMESTAMP(),?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_ENCHANT, "INSERT INTO char_enchant (player_guid, target_player_guid, item_guid, item_entry, enchant_id, permanent, player_IP, target_player_IP, time, gm_involved) VALUES (?,?,?,?,?,?,?,?, UNIX_TIMESTAMP(),?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_SEL_SANCTION_MUTE_ACCOUNT, "SELECT author_account, author_guid, target_account, duration, time, reason, IP FROM gm_sanction WHERE target_account = ? AND type = 5", CONNECTION_SYNCH); //5 is SANCTION_MUTE_ACCOUNT
    PrepareStatement(LOGS_INS_SANCTION, "INSERT INTO gm_sanction (author_account, author_guid, target_account, target_guid, target_IP, type, duration, time, reason, IP) VALUES (?,?,?,?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_SANCTION_REMOVE, "INSERT INTO gm_sanction_remove (author_account, author_guid, target_account, target_guid, target_IP, type, time, IP) VALUES (?,?,?,?,?,?,UNIX_TIMESTAMP(),?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_AUCTION_WON, "INSERT INTO char_auction_won (bidder_account, bidder_guid, seller_account, seller_guid, item_guid, item_entry, item_count, time, gm_involved) VALUES (?,?,?,?,?,?,?,UNIX_TIMESTAMP(),?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_AUCTION_CREATE, "INSERT INTO char_auction_create (seller_account, seller_guid, item_guid, item_entry, item_count, time, IP, gm_involved) VALUES (?,?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_CHAR_ITEM_VENDOR, "INSERT INTO char_item_vendor (transaction_type, account, guid, item_entry, item_count, vendor_entry, time, IP, gm_involved) VALUES (?,?,?,?,?,?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGS_INS_ACCOUNT_IP, "INSERT INTO account_ip (id, time, ip, gm_involved) VALUES (?,UNIX_TIMESTAMP(),?,?)", CONNECTION_ASYNC);
}
