/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/// \addtogroup realmd Realm Daemon
/// @{
/// \file

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "RealmList.h"

#include "Config/ConfigEnv.h"
#include "Log.h"
#include "sockets/ListenSocket.h"
#include "AuthSocket.h"
#include "SystemConfig.h"
#include "revision.h"
#include "Util.h"
#include <omp.h>

// Format is YYYYMMDDRR where RR is the change in the conf file
// for that day.
#ifndef _REALMDCONFVERSION
# define _REALMDCONFVERSION 2007062001
#endif

#ifndef _TRINITY_REALM_CONFIG
# define _TRINITY_REALM_CONFIG  "trinityrealm.conf"
#endif //_TRINITY_REALM_CONFIG

bool StartDB(std::string &dbstring);
void UnhookSignals();
void HookSignals();

bool stopEvent = false;                                     ///< Setting it to true stops the server
RealmList m_realmList;                                      ///< Holds the list of realms for this server

DatabaseType LoginDatabase;                                 ///< Accessor to the realm server database

/// Print out the usage string for this program on the console.
void usage(const char *prog)
{
    sLog.outString("Usage: \n %s [<options>]\n"
        "    --version                print version and exit\n\r"
        "    -c config_file           use config_file as configuration file\n\r"
        ,prog);
}

/// Launch the realm server
extern int main(int argc, char **argv)
{
    ///- Command line parsing to get the configuration file name
    char const* cfg_file = _TRINITY_REALM_CONFIG;
    int c = 1;
    while (c < argc) {
        if (strcmp(argv[c],"-c") == 0) {
            if (++c >= argc) {
                sLog.outError("Runtime-Error: -c option requires an input argument");
                usage(argv[0]);
                return 1;
            }
            else
                cfg_file = argv[c];
        }

        if (strcmp(argv[c],"--version") == 0) {
            printf("%s\n", _FULLVERSION);
            return 0;
        }

        ++c;
    }

    if (!sConfig.SetSource(cfg_file)) {
        sLog.outError("Could not find configuration file %s.", cfg_file);
        return 1;
    }

    sLog.Initialize();
    sLog.outString("Using configuration file %s.", cfg_file);

    ///- Check the version of the configuration file
    uint32 confVersion = sConfig.GetIntDefault("ConfVersion", 0);
    if (confVersion < _REALMDCONFVERSION) {
        sLog.outError("*****************************************************************************");
        sLog.outError(" WARNING: Your trinityrealm.conf version indicates your conf file is out of date!");
        sLog.outError("          Please check for updates, as your current default values may cause");
        sLog.outError("          strange behavior.");
        sLog.outError("*****************************************************************************");
        clock_t pause = 3000 + clock();

        while (pause > clock()) {}
    }

    sLog.outString("%s (realm-daemon)", _FULLVERSION);
    sLog.outString("<Ctrl-C> to stop.\n");

    /// realmd PID file creation
    std::string pidfile = sConfig.GetStringDefault("PidFile", "");
    if (!pidfile.empty()) {
        uint32 pid = CreatePIDFile(pidfile);
        if( !pid ) {
            sLog.outError("Cannot create PID file %s.\n", pidfile.c_str());
            return 1;
        }

        sLog.outString("Daemon PID: %u\n", pid);
    }

    ///- Initialize the database connection
    std::string dbstring;
    if (!StartDB(dbstring))
        return 1;

    ///- Get the list of realms for the server
    m_realmList.Initialize(sConfig.GetIntDefault("RealmsStateUpdateDelay", 20));
    if (m_realmList.size() == 0) {
        sLog.outError("No valid realms specified.");
        return 1;
    }

    ///- Launch the listening network socket
    port_t rmport = sConfig.GetIntDefault("RealmServerPort", DEFAULT_REALMSERVER_PORT);
    std::string bind_ip = sConfig.GetStringDefault("BindIP", "0.0.0.0");

    SocketHandler h;
    ListenSocket<AuthSocket> authListenSocket(h);
    if (authListenSocket.Bind(bind_ip.c_str(),rmport)) {
        sLog.outError("Trinity realm can not bind to %s:%d",bind_ip.c_str(), rmport);
        return 1;
    }

    h.Add(&authListenSocket);

    ///- Catch termination signals
    HookSignals();

    // maximum counter for next ping
    uint32 numLoops = (sConfig.GetIntDefault("MaxPingTime", 30) * (MINUTE * 1000000 / 100000));
    uint32 loopCounter = 0;

    ///- Wait for termination signal
    omp_set_num_threads(4);

    //while (!stopEvent) {
#pragma omp parallel for shared(loopCounter)
    for ( ; !stopEvent; ) {
        h.Select(0, 100000);

        if ((++loopCounter) == numLoops) {
            loopCounter = 0;
            sLog.outDetail("Ping MySQL to keep connection alive");
            delete LoginDatabase.Query("SELECT 1 FROM realmlist LIMIT 1");
        }
    }

    ///- Wait for the delay thread to exit
    LoginDatabase.Close();

    ///- Remove signal handling before leaving
    UnhookSignals();

    sLog.outString("Halting process...");
    return 0;
}

/// Handle termination signals
/** Put the global variable stopEvent to 'true' if a termination signal is caught **/
void OnSignal(int s)
{
    switch (s)
    {
    case SIGINT:
    case SIGTERM:
        stopEvent = true;
        break;
    }

    signal(s, OnSignal);
}

/// Initialize connection to the database
bool StartDB(std::string &dbstring)
{
    if (!sConfig.GetString("LoginDatabaseInfo", &dbstring)) {
        sLog.outError("Database not specified");
        return false;
    }

    sLog.outString("Database: %s", dbstring.c_str() );
    uint8 num_threads = sConfig.GetIntDefault("LoginDatabase.WorkerThreads", 1);
    if (num_threads < 1 || num_threads > 32) {
        sLog.outError("Improper value specified for LoginDatabase.WorkerThreads, defaulting to 1.");
        num_threads = 1;
    }
    
    if (!LoginDatabase.Open(dbstring.c_str(), num_threads)) {
        sLog.outError("Cannot connect to database");
        return false;
    }

    return true;
}

/// Define hook 'OnSignal' for all termination signals
void HookSignals()
{
    signal(SIGINT, OnSignal);
    signal(SIGTERM, OnSignal);
}

/// Unhook the signals before leaving
void UnhookSignals()
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
}

/// @}

