#include "LogsDatabase.h"

void LogsDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_LOGSDATABASE_STATEMENTS);
}
