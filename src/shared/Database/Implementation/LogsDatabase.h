#ifndef _LOGSDATABASE_H
#define _LOGSDATABASE_H

#include "DatabaseWorkerPool.h"
#include "MySQLConnection.h"

enum LogsDatabaseStatements
{
    /*  Naming standard for defines:
    {DB}_{SEL/INS/UPD/DEL/REP}_{Summary of data changed}
    When updating more than one field, consider looking at the calling function
    name for a suiting suffix.
    */
     
    // LOGS_INS_ARENA_MATCH, //TODO
    LOGS_INS_BOSS_DOWN,
    LOGS_INS_BG_STATS,
    LOGS_INS_CHAR_DELETE,
    LOGS_INS_CHAR_CHAT,
    LOGS_INS_CHAR_GUILD_MONEY,
    LOGS_INS_CHAR_ITEM_DELETE,
    LOGS_INS_CHAR_ITEM_GUILD_BANK,
    LOGS_INS_MAIL,
    LOGS_INS_MAIL_ITEMS,
    LOGS_INS_CHAR_RENAME,
    LOGS_SEL_CHAR_TRADE_MAX_ID,
    LOGS_INS_CHAR_TRADE,
    LOGS_INS_CHAR_TRADE_ITEMS,
    LOGS_INS_GM_COMMAND,
    LOGS_SEL_SANCTION_MUTE_ACCOUNT,
    LOGS_INS_SANCTION,
    LOGS_INS_SANCTION_REMOVE,
    LOGS_INS_CHAR_ENCHANT,
    LOGS_INS_CHAR_AUCTION_WON,
    LOGS_INS_CHAR_AUCTION_CREATE,
    LOGS_INS_CHAR_ITEM_VENDOR, //TODO: buy
    LOGS_INS_ACCOUNT_IP,

    MAX_LOGSDATABASE_STATEMENTS
};

class LogsDatabaseConnection : public MySQLConnection
{
    public:
        typedef LogsDatabaseStatements Statements;

        //- Constructors for sync and async connections
        LogsDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo) { }
        LogsDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo) { }

        //- Loads database type specific prepared statements
        void DoPrepareStatements() override;
};

typedef DatabaseWorkerPool<LogsDatabaseConnection> LogsDatabaseWorkerPool;

#endif
