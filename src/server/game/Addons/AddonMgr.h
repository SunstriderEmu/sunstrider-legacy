
#ifndef _ADDONMGR_H
#define _ADDONMGR_H

#include "Define.h"
#include <string>
#include <list>

enum ClientBuild : uint32;

struct AddonInfo
{
    AddonInfo(std::string  name, uint8 enabled, uint32 crc, uint8 state, bool crcOrPubKey, uint32 build)
        : Name(std::move(name)), CRC(crc), State(state), UsePublicKeyOrCRC(crcOrPubKey), Enabled(enabled), build(build)
        { }

    std::string Name;
    uint8 Enabled; //LK only
    uint32 CRC;
    uint32 build;
    uint8 State;
    bool UsePublicKeyOrCRC;
};

struct SavedAddon
{
    SavedAddon(std::string  name, uint32 crc, uint32 build) :
        Name(std::move(name)),
        CRC(crc),
        build(build)
    { }

    std::string Name;
    uint32 CRC;
    uint32 build;
};

struct BannedAddon
{
    uint32 Id;
    uint8 NameMD5[16];
    uint8 VersionMD5[16];
    uint32 Timestamp;
};

//LK
#define STANDARD_ADDON_CRC_12340 0x4C1C776DLL
//BC
#define STANDARD_ADDON_CRC_8606 0x4C1C776DLL

namespace AddonMgr
{
    void LoadFromDB();
    void SaveAddon(AddonInfo const& addon);
    SavedAddon const* GetAddonInfo(const std::string& name, ClientBuild build);

    typedef std::list<BannedAddon> BannedAddonList;
    BannedAddonList const* GetBannedAddons();

    uint64 GetStandardAddonCRC(uint32 clientBuild);
}

#endif