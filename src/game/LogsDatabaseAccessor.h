#ifndef _LOGSDATABASEACCESSOR_H
#define _LOGSDATABASEACCESSOR_H

#include "Define.h"
#include "../game/SharedDefines.h"

class Creature;
class Player;
enum MailMessageType : uint32;
class WorldSession;
class Item;
class MailItemsInfo;
class Unit;
enum SanctionType : uint32;

class LogsDatabaseAccessor
{
public:
    static LogsDatabaseAccessor* instance()
    {
        static LogsDatabaseAccessor instance;
        return &instance;
    }

    LogsDatabaseAccessor();

    // Cleanup mon_* table according to config CONFIG_MONITORING_KEEP_DURATION
    static void CleanupOldMonitorLogs();
    // Cleanup most log tables according to their respective CONFIG_LOG_* options
    static void CleanupOldLogs();

    static void Enchantment(Player const* caster, Player const* targetPlayer, uint32 itemGUIDLow, uint32 itemEntry, uint32 enchantID, bool permanent);
    static void BattlegroundStats(uint32 mapId, time_t start, time_t end, Team winner, uint32 scoreAlliance, uint32 scoreHorde);
    static void BossDown(Creature const* victim, std::string const& bossName, std::string const& bossNameFr, uint32 downByGuildId, std::string const& , uint32 guildPercentage, uint32 leaderGuid);
    static void CharacterDelete(WorldSession const* session, uint32 playerGUID, std::string const& charName, uint8 level, std::string const& IP);
    static void CharacterRename(WorldSession const* m_session, uint32 playerGUID, std::string const& oldName, std::string const& newName, std::string const& IP);
    static void GMCommand(WorldSession const* m_session, Unit const* target, std::string const& fullcmd);
    // 'to' can be a player name or a channel name
    static void CharacterChat(ChatMsg type, Language lang, Player const* player, Player const* toPlayer, uint32 logChannelId, std::string const& to, std::string const& msg);
    static void GuildMoneyTransfer(Player const* player, uint32 guildId, int32 money);
    static void GuildBankItemTransfer(Player const* player, bool deposit, uint32 itemGuid, uint32 itemEntry, uint8 itemCount);
    static void CharacterItemDelete(Player const* player, Item const* item);
    static void Sanction(WorldSession const* author, uint32 targetAccount, uint32 targetGUID, SanctionType type, uint32 durationSecs, std::string const& reason);
    static void RemoveSanction(WorldSession const* author, uint32 targetAccount, uint32 targetGUID, std::string const& targetIP, SanctionType type);
    // money can be negative to indicate a COD (Collect on Delivery)
    static void Mail(uint32 mailId, MailMessageType type, uint32 sender_guidlow_or_entry, uint32 receiver_guidlow, std::string const& subject, uint32 itemTextId, MailItemsInfo const* mi, int32 money);

    static void WonAuction(uint32 bidderAccount, uint32 bidderGUID, uint32 sellerAccount, uint32 sellerGUID, uint32 itemGUID, uint32 itemEntry, uint32 itemCount);
    static void CreateAuction(Player const* player, uint32 itemGUID, uint32 itemEntry, uint32 itemCount);

    enum BuyTransactionType
    {
        TRANSACTION_BUY,
        TRANSACTION_SELL,
        TRANSACTION_BUYBACK,
    };

    static void BuyOrSellItemToVendor(BuyTransactionType buyType, Player const* player, Item const* item, Unit const* vendor); //TODO use this with TRANSACTION_BUY

    void CharacterTrade(Player const* p1, Player const* p2, std::vector<Item*> const& p1Items, std::vector<Item*> const& p2Items, uint32 p1Gold, uint32 p2Gold);
private:
    static bool ShouldLog(uint32 configDuration, uint32 configDurationGM, bool gmInvolved);

    uint32 max_trade_id;
};

#define sLogsDatabaseAccessor LogsDatabaseAccessor::instance()

#endif //_LOGSDATABASEACCESSOR_H
