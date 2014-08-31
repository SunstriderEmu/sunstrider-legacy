#ifndef _LOGSDATABASE_H
#define _LOGSDATABASE_H

#include "DatabaseWorkerPool.h"
#include "MySQLConnection.h"

class LogsDatabaseConnection : public MySQLConnection
{
    public:
        //- Constructors for sync and async connections
        LogsDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo) { }
        LogsDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo) { }

        //- Loads database type specific prepared statements
        void DoPrepareStatements() override;
};

typedef DatabaseWorkerPool<LogsDatabaseConnection> LogsDatabaseWorkerPool;

enum LogsDatabaseStatements
{
    /*  Naming standard for defines:
        {DB}_{SEL/INS/UPD/DEL/REP}_{Summary of data changed}
        When updating more than one field, consider looking at the calling function
        name for a suiting suffix.
    */

    MAX_LOGSDATABASE_STATEMENTS
};

#endif
