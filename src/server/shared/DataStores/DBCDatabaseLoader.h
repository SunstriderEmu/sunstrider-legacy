
#ifndef DBCDatabaseLoader_h__
#define DBCDatabaseLoader_h__

#include "DBCFileLoader.h"
#include <string>
#include <vector>

struct TC_SHARED_API DBCDatabaseLoader
{
    DBCDatabaseLoader(std::string const& storageName, std::string const& dbFormatString, std::string const& primaryKey, char const* dbcFormatString);

    char* Load(uint32& records, char**& indexTable);

private:
    std::string const& _formatString;
    std::string const& _indexName;
    std::string _sqlTableName;
    char const* _dbcFormat;
    int32 _sqlIndexPos;
    uint32 _recordSize;

    DBCDatabaseLoader(DBCDatabaseLoader const& right) = delete;
    DBCDatabaseLoader& operator=(DBCDatabaseLoader const& right) = delete;
};

#endif // DBCDatabaseLoader_h__
