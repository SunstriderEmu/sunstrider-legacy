
#ifndef _WARDEN_DATA_STORAGE_H
#define _WARDEN_DATA_STORAGE_H
#include <map>
#include "BigNumber.h"

struct WardenData
{
    uint16 id;
    uint8 Type;
    BigNumber i;
    uint32 Address;                                         // PROC_CHECK, MEM_CHECK, PAGE_CHECK
    uint8 Length;                                           // PROC_CHECK, MEM_CHECK, PAGE_CHECK
    uint8 action;
    std::string str;                                        // LUA, MPQ, DRIVER
    std::string comment;                                    // Used in logs
};

struct WardenDataResult
{
    BigNumber res;                                          // MEM_CHECK
};

class CWardenDataStorage
{
    public:
        CWardenDataStorage();
        ~CWardenDataStorage();

        uint32 InternalDataID;
        std::vector<uint32> MemCheckIds;

    private:
        std::map<uint32, WardenData*> _data_map;
        std::map<uint32, WardenDataResult*> _result_map;

    public:
        inline uint32 GenerateInternalDataID() { return InternalDataID++; }
        WardenData *GetWardenDataById(uint32 Id);
        WardenDataResult *GetWardenResultById(uint32 Id);
        void Init(bool reload = false);

    protected:
        void LoadWardenDataResult(bool reload = false);
};

extern CWardenDataStorage WardenDataStorage;

#endif
