
#ifndef DBCDatabaseLoader_h__
#define DBCDatabaseLoader_h__

#include "DBCFileLoader.h"
#include <string>
#include <vector>

struct TC_SHARED_API DBCDatabaseLoader
{
    DBCDatabaseLoader(char const* dbTable, char const* dbFormatString, char const* index, char const* dbcFormatString, std::vector<char*>& stringPool);

    char* Load(uint32& records, char**& indexTable);

private:
    char const* _sqlTableName;
    char const* _formatString;
    char const* _indexName;
    char const* _dbcFormat;
    int32 _sqlIndexPos;
    uint32 _recordSize;
    std::vector<char*>& _stringPool;
    char* CloneStringToPool(std::string const& str);

    DBCDatabaseLoader(DBCDatabaseLoader const& right) = delete;
    DBCDatabaseLoader& operator=(DBCDatabaseLoader const& right) = delete;
};

#endif // DBCDatabaseLoader_h__
