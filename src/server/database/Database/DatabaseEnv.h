
#ifndef DATABASEENV_H
#define DATABASEENV_H

#include "Common.h"
#include "Debugging/Errors.h"
#include "Log.h"

#include "Field.h"
#include "QueryResult.h"

#include "MySQLThreading.h"
#include "Transaction.h"

#define _LIKE_           "LIKE"
#define _TABLE_SIM_      "`"
#define _CONCAT3_(A, B, C) "CONCAT( " A ", " B ", " C " )"
#define _OFFSET_         "LIMIT %d, 1"

#include "Implementation/LoginDatabase.h"
#include "Implementation/CharacterDatabase.h"
#include "Implementation/WorldDatabase.h"
#include "Implementation/LogsDatabase.h"

TC_DATABASE_API extern WorldDatabaseWorkerPool WorldDatabase;
TC_DATABASE_API extern CharacterDatabaseWorkerPool CharacterDatabase;
TC_DATABASE_API extern LoginDatabaseWorkerPool LoginDatabase;
TC_DATABASE_API extern LogsDatabaseWorkerPool LogsDatabase;

#endif

