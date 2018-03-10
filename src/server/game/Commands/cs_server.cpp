#include "Chat.h"
#include "Language.h"
#include "GlobalEvents.h"
#include "Monitor.h"
#include "GitRevision.h"
#include "GameTime.h"
#include <numeric>
#include "VMapFactory.h"
#include "Realm.h"
#include "DatabaseLoader.h"

#include <boost/filesystem.hpp>
#include <mysql_version.h>
#include <openssl/crypto.h>
#include <openssl/opensslv.h>

/// Triggering corpses expire check in world
bool ChatHandler::HandleServerCorpsesCommand(const char* /*args*/)
{
    SendSysMessage("NYI");
    // sWorld->RemoveOldCorpses();
    return true;
}

bool ChatHandler::HandleServerDebugCommand(char const* /*args*/)
{
    uint16 worldPort = uint16(sWorld->getIntConfig(CONFIG_PORT_WORLD));
    std::string dbPortOutput;

    {
        uint16 dbPort = 0;
        if (QueryResult res = LoginDatabase.PQuery("SELECT port FROM realmlist WHERE id = %u", realm.Id.Realm))
            dbPort = (*res)[0].GetUInt16();

        if (dbPort)
            dbPortOutput = Trinity::StringFormat("Realmlist (Realm Id: %u) configured in port %" PRIu16, realm.Id.Realm, dbPort);
        else
            dbPortOutput = Trinity::StringFormat("Realm Id: %u not found in `realmlist` table. Please check your setup", realm.Id.Realm);
    }

    PSendSysMessage("%s", GitRevision::GetFullVersion());
    PSendSysMessage("Using SSL version: %s (library: %s)", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));
    PSendSysMessage("Using Boost version: %i.%i.%i", BOOST_VERSION / 100000, BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100);
    PSendSysMessage("Using MySQL version: %s", MYSQL_SERVER_VERSION);
    PSendSysMessage("Using CMake version: %s", GitRevision::GetCMakeVersion());

    PSendSysMessage("Compiled on: %s", GitRevision::GetHostOSVersion());

    uint32 updateFlags = sConfigMgr->GetIntDefault("Updates.EnableDatabases", DatabaseLoader::DATABASE_NONE);
    if (!updateFlags)
        SendSysMessage("Automatic database updates are disabled for all databases!");
    else
    {
        static char const* const databaseNames[3 /*TOTAL_DATABASES*/] =
        {
            "Auth",
            "Characters",
            "World"
        };

        std::string availableUpdateDatabases;
        for (uint32 i = 0; i < 3 /* TOTAL_DATABASES*/; ++i)
        {
            if (!(updateFlags & (1 << i)))
                continue;

            availableUpdateDatabases += databaseNames[i];
            if (i != 3 /*TOTAL_DATABASES*/ - 1)
                availableUpdateDatabases += ", ";
        }

        PSendSysMessage("Automatic database updates are enabled for the following databases: %s", availableUpdateDatabases.c_str());
    }

    PSendSysMessage("Worldserver listening connections on port %" PRIu16, worldPort);
    PSendSysMessage("%s", dbPortOutput.c_str());

    //bool vmapIndoorCheck = sWorld->getBoolConfig(CONFIG_VMAP_INDOOR_CHECK);
    bool vmapIndoorCheck = true;
    bool vmapLOSCheck = VMAP::VMapFactory::createOrGetVMapManager()->isLineOfSightCalcEnabled();
    bool vmapHeightCheck = VMAP::VMapFactory::createOrGetVMapManager()->isHeightCalcEnabled();

    //bool mmapEnabled = sWorld->getBoolConfig(CONFIG_ENABLE_MMAPS);
    bool mmapEnabled = true;

    std::string dataDir = sWorld->GetDataPath();
    std::vector<std::string> subDirs;
    subDirs.emplace_back("maps");
    if (vmapIndoorCheck || vmapLOSCheck || vmapHeightCheck)
    {
        PSendSysMessage("VMAPs status: Enabled. LineOfSight: %i, getHeight: %i, indoorCheck: %i", vmapLOSCheck, vmapHeightCheck, vmapIndoorCheck);
        subDirs.emplace_back("vmaps");
    }
    else
        SendSysMessage("VMAPs status: Disabled");

    if (mmapEnabled)
    {
        SendSysMessage("MMAPs status: Enabled");
        subDirs.emplace_back("mmaps");
    }
    else
        SendSysMessage("MMAPs status: Disabled");

    for (std::string const& subDir : subDirs)
    {
        boost::filesystem::path mapPath(dataDir);
        mapPath.append(subDir);

        if (!boost::filesystem::exists(mapPath))
        {
            PSendSysMessage("%s directory doesn't exist!. Using path: %s", subDir.c_str(), mapPath.generic_string().c_str());
            continue;
        }

        auto end = boost::filesystem::directory_iterator();
        std::size_t folderSize = std::accumulate(boost::filesystem::directory_iterator(mapPath), end, std::size_t(0), [](std::size_t val, boost::filesystem::path const& mapFile)
        {
            if (boost::filesystem::is_regular_file(mapFile))
                val += boost::filesystem::file_size(mapFile);
            return val;
        });

        PSendSysMessage("%s directory located in %s. Total size: " SZFMTD " bytes", subDir.c_str(), mapPath.generic_string().c_str(), folderSize);
    }

    LocaleConstant defaultLocale = sWorld->GetDefaultDbcLocale();
    uint32 availableLocalesMask = (1 << defaultLocale);

    for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
    {
        LocaleConstant locale = static_cast<LocaleConstant>(i);
        if (locale == defaultLocale)
            continue;

        if (sWorld->GetAvailableDbcLocale(locale) != defaultLocale)
            availableLocalesMask |= (1 << locale);
    }

    std::string availableLocales;
    for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
    {
        if (!(availableLocalesMask & (1 << i)))
            continue;

        availableLocales += localeNames[i];
        if (i != TOTAL_LOCALES - 1)
            availableLocales += " ";
    }

    PSendSysMessage("Using %s DBC Locale as default. All available DBC locales: %s", localeNames[defaultLocale], availableLocales.c_str());

    PSendSysMessage("Using World DB: %s", sWorld->GetDBVersion());
    return true;
}

/// Exit the realm
bool ChatHandler::HandleServerExitCommand(const char* args)
{
    SendSysMessage(LANG_COMMAND_EXIT);
    World::StopNow(SHUTDOWN_EXIT_CODE);
    return true;
}

bool ChatHandler::HandleServerInfoCommand(const char* /*args*/)
{
    uint32 activeClientsNum = sWorld->GetActiveSessionCount();
    uint32 queuedClientsNum = sWorld->GetQueuedSessionCount();
    uint32 maxActiveClientsNum = sWorld->GetMaxActiveSessionCount();
    std::string str = secsToTimeString(GameTime::GetUptime());
    uint32 currentMapTimeDiff = 0;
    if (GetSession())
        if (Player const* p = GetSession()->GetPlayer())
            if (Map const* m = p->FindMap())
                currentMapTimeDiff = sMonitor->GetLastDiffForMap(*m);

    PSendSysMessage("%s", GitRevision::GetFullVersion());
    PSendSysMessage("Players online: %u (Max: %u, Queued: %u)", activeClientsNum, maxActiveClientsNum, queuedClientsNum);
    PSendSysMessage(LANG_UPTIME, str.c_str());
    PSendSysMessage("Smoothed update time diff: %u.", sMonitor->GetSmoothTimeDiff());
    PSendSysMessage("Instant update time diff: %u.", sWorld->GetUpdateTime());
    PSendSysMessage("Current map update time diff: %u.", currentMapTimeDiff);
    if (sWorld->IsShuttingDown())
        PSendSysMessage("Server restart in %s", secsToTimeString(sWorld->GetShutDownTimeLeft()).c_str());

    return true;
}

/// Display the 'Message of the day' for the realm
bool ChatHandler::HandleServerMotdCommand(const char* /*args*/)
{
    PSendSysMessage(LANG_MOTD_CURRENT, sWorld->GetMotd());
    return true;
}

bool ChatHandler::HandleServerShutDownCancelCommand(const char* args)
{
    sWorld->ShutdownCancel();
    return true;
}

bool ChatHandler::HandleServerShutDownCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (nullptr, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if((time == 0 && (time_str[0]!='0' || time_str[1]!='\0')) || time < 0)
        return false;

    //if (exitcode_str)
    if (reason)
    {
        /*int32 exitcode = atoi (exitcode_str);

        // Handle atoi() errors
        if (exitcode == 0 && (exitcode_str[0] != '0' || exitcode_str[1] != '\0'))
            return false;

        // Exit code should be in range of 0-125, 126-255 is used
        // in many shells for their own return codes and code > 255
        // is not supported in many others
        if (exitcode < 0 || exitcode > 125)
            return false;

        sWorld->ShutdownServ (time, 0, exitcode);*/
        
        sWorld->ShutdownServ(time, 0, SHUTDOWN_EXIT_CODE, reason);
    }
    else
        sWorld->ShutdownServ(time, 0, SHUTDOWN_EXIT_CODE, "");
    return true;
}

bool ChatHandler::HandleServerRestartCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (nullptr, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if((time == 0 && (time_str[0]!='0' || time_str[1]!='\0')) || time < 0)
        return false;

    if (reason)
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE, reason);
    else
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE, "");
        
    return true;
}

bool ChatHandler::HandleServerIdleRestartCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (nullptr, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if((time == 0 && (time_str[0]!='0' || time_str[1]!='\0')) || time < 0)
        return false;

    if (reason)
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE, reason);
    else
        sWorld->ShutdownServ(time,SHUTDOWN_MASK_RESTART|SHUTDOWN_MASK_IDLE, RESTART_EXIT_CODE, "");
        
    return true;
}

bool ChatHandler::HandleServerIdleShutDownCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (nullptr, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if((time == 0 && (time_str[0]!='0' || time_str[1]!='\0')) || time < 0)
        return false;

    if (reason)
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_IDLE, SHUTDOWN_EXIT_CODE, reason);
    else
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_IDLE, SHUTDOWN_EXIT_CODE, "");
        
    return true;
}

/// Define the 'Message of the day' for the realm
bool ChatHandler::HandleServerSetMotdCommand(const char* args)
{
    sWorld->SetMotd(args);
    PSendSysMessage(LANG_MOTD_NEW, args);
    return true;
}

bool ChatHandler::HandleServerSetConfigCommand(const char* args)
{
    ARGS_CHECK

    char* cConfigIndex = strtok((char*)args, " ");
    char* cConfigValue = strtok(nullptr, " ");

    if (!cConfigIndex || !cConfigValue)
        return false;

    uint32 configIndex = (uint32)atoi(cConfigIndex);
    if(configIndex > CONFIG_VALUE_COUNT)
    {
        PSendSysMessage("Wrong index");
        return false;
    }

    uint32 configValue = (uint32)atoi(cConfigValue);
    sWorld->setConfig(configIndex,configValue);

    PSendSysMessage("Config %i set to %i",configIndex,configValue);

    return true;
}
