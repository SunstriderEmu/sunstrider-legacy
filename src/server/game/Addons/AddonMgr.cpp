
#include "AddonMgr.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "Log.h"
#include "Timer.h"
#include <openssl/md5.h>
#include "WorldSession.h"

namespace AddonMgr
{

// Anonymous namespace ensures file scope of all the stuff inside it, even
// if you add something more to this namespace somewhere else.
namespace
{
    // List of saved addons (in DB).
    typedef std::list<SavedAddon> SavedAddonsList;

    SavedAddonsList m_knownAddons;

    BannedAddonList m_bannedAddons;
} // namespace

void LoadFromDB()
{
    uint32 oldMSTime = GetMSTime();

    QueryResult result = CharacterDatabase.Query("SELECT name, crc, gamebuild FROM addons");
    if (result)
    {
        uint32 count = 0;

        do
        {
            Field* fields = result->Fetch();

            std::string name = fields[0].GetString();
            uint32 crc = fields[1].GetUInt32();
            uint32 build = fields[2].GetUInt32();

            m_knownAddons.push_back(SavedAddon(name, crc, build));

            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO("server.loading", ">> Loaded %u known addons in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 known addons. DB table `addons` is empty!");

    oldMSTime = GetMSTime();
    result = CharacterDatabase.Query("SELECT id, name, version, UNIX_TIMESTAMP(timestamp) FROM banned_addons");
    if (result)
    {
        uint32 count = 0;
        uint32 dbcMaxBannedAddon = 0;
#ifdef LICH_KING
        dbcMaxBannedAddon = sBannedAddOnsStore.GetNumRows();
#endif

        do
        {
            Field* fields = result->Fetch();

            BannedAddon addon;
            addon.Id = fields[0].GetUInt32() + dbcMaxBannedAddon;
            addon.Timestamp = uint32(fields[3].GetUInt64());

            std::string name = fields[1].GetString();
            std::string version = fields[2].GetString();

            MD5(reinterpret_cast<uint8 const*>(name.c_str()), name.length(), addon.NameMD5);
            MD5(reinterpret_cast<uint8 const*>(version.c_str()), version.length(), addon.VersionMD5);

            m_bannedAddons.push_back(addon);

            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO("server.loading", ">> Loaded %u banned addons in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
}

uint64 GetStandardAddonCRC(uint32 clientBuild)
{
    switch(clientBuild)
    {
    case BUILD_335:
        return STANDARD_ADDON_CRC_12340;
    case BUILD_243:
    default:
        return STANDARD_ADDON_CRC_8606;
    }
}

void SaveAddon(AddonInfo const& addon)
{
    std::string name = addon.Name;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ADDON);

    stmt->setString(0, name);
    stmt->setUInt32(1, addon.CRC);
    stmt->setUInt32(2, addon.build);

    CharacterDatabase.Execute(stmt);

    m_knownAddons.push_back(SavedAddon(addon.Name, addon.CRC, addon.build));
}

SavedAddon const* GetAddonInfo(const std::string& name, ClientBuild build)
{
    for (SavedAddonsList::const_iterator it = m_knownAddons.begin(); it != m_knownAddons.end(); ++it)
    {
        SavedAddon const& addon = (*it);
        if (addon.Name == name && addon.build == uint32(build))
            return &addon;
    }

    return NULL;
}

BannedAddonList const* GetBannedAddons()
{
    return &m_bannedAddons;
}

} // namespace AddonMgr
