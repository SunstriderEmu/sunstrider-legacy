#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>

#include "GitRevision.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "Config.h"
#include "Log.h"
#include "BigNumber.h"
#include "OpenSSLCrypto.h"
#include "RASession.h"
#include "AsyncAcceptor.h"
#include "ScriptMgr.h"
#include "BattleGroundMgr.h"
#include "TCSoap.h"
#include "CliRunnable.h"
#include "WorldSocket.h"
#include "WorldSocketMgr.h"
#include "ScriptMgr.h"
#include "ScriptReloadMgr.h"
#include "ScriptLoader.h"
#include "OutdoorPvPMgr.h"
#include "Realm/Realm.h"
#include "World.h"
#include "Configuration/Config.h"
#include "ProcessPriority.h"
#include "Timer.h"
#include "MapManager.h"
#include "IRCMgr.h"
#include "ScriptReloadMgr.h"
#include "AppenderDB.h"
#include <segvcatch.h>
#if PLATFORM == PLATFORM_UNIX
#include <fstream>
#include <execinfo.h>
#endif

using namespace boost::program_options;
namespace fs = boost::filesystem;

#define WORLD_SLEEP_CONST 50

#ifndef _WORLD_SERVER_CONFIG
# define _WORLD_SERVER_CONFIG  "worldserver.conf"
#endif //_WORLD_SERVER_CONFIG

// Format is YYYYMMDDRR where RR is the change in the conf file
// for that day.
#ifndef _TRINITY_CORE_CONFVER
# define _TRINITY_CORE_CONFVER 2014060701
#endif //_TRINITY_CORE_CONFVER

#ifdef WIN32
#include "ServiceWin32.h"
char serviceName[] = "Sunstriderd";
char serviceLongName[] = "Sunstrider service";
char serviceDescription[] = "WoW 2.4.3 Server Emulator service";
/*
 * -1 - not in service mode
 *  0 - stopped
 *  1 - running
 *  2 - paused
 */
int m_ServiceStatus = -1;
#endif

boost::asio::io_service _ioService;
boost::asio::deadline_timer _freezeCheckTimer(_ioService);
uint32 _worldLoopCounter(0);
uint32 _lastChangeMsTime(0);
uint32 _maxCoreStuckTimeInMs(0);

uint32 realmID;                                             ///< Id of the realm

void SignalHandler(const boost::system::error_code& error, int signalNumber);
void FreezeDetectorHandler(const boost::system::error_code& error);
AsyncAcceptor* StartRaSocketAcceptor(boost::asio::io_service& ioService);
bool StartDB();
void StopDB();
void WorldUpdateLoop();
void ClearOnlineAccounts();
void ShutdownThreadPool(std::vector<std::thread>& threadPool);
void ShutdownCLIThread(std::thread* cliThread);
bool LoadRealmInfo();
variables_map GetConsoleArguments(int argc, char** argv, fs::path& configFile, std::string& cfg_service);

//segv handler, print stack to dump file
void handle_segv()
{
#if PLATFORM == PLATFORM_UNIX
    void* arr[20];
    size_t size = backtrace(arr, 20);
    fprintf(stderr, "SEGV or PFE occured :\n");
    //print to stderr
    backtrace_symbols_fd(arr, size, STDERR_FILENO);

    //get backtrace as string array
    char** backtrace = backtrace_symbols(arr, size);
    //print to a dump file in exec folder as well
    std::string outputFileName = "mapcrash_" + std::to_string(time(nullptr));
    std::ofstream dumpFile(outputFileName, std::ios::out);
    if (dumpFile.is_open())
    {
        dumpFile << "Error: signal %d:" << std::endl;
        for (int i = 0; i < size; i++)
            dumpFile << backtrace[i] << std::endl;

        dumpFile.close();
    }
    //delete array allocated by backtrace_symbols
    //delete backtrace;
#endif

    throw std::runtime_error("Segmentation fault or FPE");
}


/// Launch the Sunstrider server
extern int main(int argc, char **argv)
{
#if defined(__has_feature)
#  if __has_feature(address_sanitizer)
       printf("Running with -fsanitize=address flag\n");
#  endif
#endif

    ///- Command line parsing to get the configuration file name
	auto configFile = fs::absolute(_WORLD_SERVER_CONFIG);
    std::string configService;

    auto vm = GetConsoleArguments(argc, argv, configFile, configService);
    // exit if help is enabled
    if (vm.count("help"))
        return 0;

#ifdef _WIN32
	/*
    if (configService.compare("install") == 0)
        return WinServiceInstall() == true ? 0 : 1;
    else if (configService.compare("uninstall") == 0)
        return WinServiceUninstall() == true ? 0 : 1;
    else if (configService.compare("run") == 0)
        WinServiceRun();
		*/
#endif

    std::string configError;
    if (!sConfigMgr->LoadInitial(configFile.generic_string(),
								 std::vector<std::string>(argv, argv + argc),
								 configError))
    {
        printf("Error in config file: %s\n", configError.c_str());
        return 1;
    }

	if (sWorld->getConfig(CONFIG_MAP_CRASH_RECOVERY_ENABLED))
    {
        segvcatch::init_segv(handle_segv);
        segvcatch::init_fpe(handle_segv);
    }

    sLog->RegisterAppender<AppenderDB>();
    // If logs are supposed to be handled async then we need to pass the io_service into the Log singleton
    sLog->Initialize(sConfigMgr->GetBoolDefault("Log.Async.Enable", false) ? &_ioService : nullptr);


    TC_LOG_INFO("server.worldserver", "%s (worldserver-daemon)", GitRevision::GetFullVersion());
    TC_LOG_INFO("server.worldserver", "<Ctrl-C> to stop.\n");
    TC_LOG_INFO("server.worldserver", "  ____                          _            _       _               ");
    TC_LOG_INFO("server.worldserver", " / ___|   _   _   _ __    ___  | |_   _ __  (_)   __| |   ___   _ __ ");
    TC_LOG_INFO("server.worldserver", " \\___ \\  | | | | | '_ \\  / __| | __| | '__| | |  / _` |  / _ \\ | '__|");
    TC_LOG_INFO("server.worldserver", "  ___) | | |_| | | | | | \\__ \\ | |_  | |    | | | (_| | |  __/ | |   ");
    TC_LOG_INFO("server.worldserver", " |____/   \\__,_| |_| |_| |___/  \\__| |_|    |_|  \\__,_|  \\___| |_|   ");
    TC_LOG_INFO("server.worldserver", " ");
    TC_LOG_INFO("server.worldserver", "Using configuration file %s.", sConfigMgr->GetFilename().c_str());
    TC_LOG_INFO("server.worldserver", "Using SSL version: %s (library: %s)", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));
    TC_LOG_INFO("server.worldserver", "Using Boost version: %i.%i.%i", BOOST_VERSION / 100000, BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100);

    OpenSSLCrypto::threadsSetup();

    BigNumber seed;
    seed.SetRand(16 * 8);

    /// worldserver PID file creation
    std::string pidFile = sConfigMgr->GetStringDefault("PidFile", "");
    if (!pidFile.empty())
    {
        if (uint32 pid = CreatePIDFile(pidFile))
            TC_LOG_INFO("server.worldserver", "Daemon PID: %u\n", pid);
        else
        {
            TC_LOG_ERROR("server.worldserver", "Cannot create PID file %s.\n", pidFile.c_str());
            return 1;
        }
    }

    // Set signal handlers (this must be done before starting io_service threads, because otherwise they would unblock and exit)
    boost::asio::signal_set signals(_ioService, SIGINT, SIGTERM);
#if PLATFORM == PLATFORM_WINDOWS
    signals.add(SIGBREAK);
#endif
    signals.async_wait(SignalHandler);

    // Start the Boost based thread pool
    int numThreads = sConfigMgr->GetIntDefault("ThreadPool", 1);
    std::vector<std::thread> threadPool;

    if (numThreads < 1)
        numThreads = 1;

    for (int i = 0; i < numThreads; ++i)
        threadPool.push_back(std::thread(boost::bind(&boost::asio::io_service::run, &_ioService)));

    //Set process priority according to configuration settings
    SetProcessPriority("server.worldserver");

    // Start the databases
    if (!StartDB())
    {
        ShutdownThreadPool(threadPool);
        return 1;
    }

    // Set server offline (not connectable)
    LoginDatabase.DirectPExecute("UPDATE realmlist SET flag = (flag & ~%u) | %u WHERE id = '%d'", REALM_FLAG_OFFLINE, REALM_FLAG_VERSION_MISMATCH, realmID);

	LoadRealmInfo();

    // Initialize the World
	sScriptMgr->SetScriptLoader(AddScripts);
    sWorld->SetInitialWorldSettings();
    
    // Launch CliRunnable thread
    std::thread* cliThread = nullptr;
#ifdef _WIN32
    if (sConfigMgr->GetBoolDefault("Console.Enable", true) && (m_ServiceStatus == -1)) // need disable console in service mode
#else
    if (sConfigMgr->GetBoolDefault("Console.Enable", true))
#endif
    {
        cliThread = new std::thread(CliThread);
    }

    // Start the Remote Access port (acceptor) if enabled
    AsyncAcceptor* raAcceptor = nullptr;
    if (sConfigMgr->GetBoolDefault("Ra.Enable", false))
        raAcceptor = StartRaSocketAcceptor(_ioService);

    // Start soap serving thread if enabled
    std::thread* soapThread = nullptr;
    if (sConfigMgr->GetBoolDefault("SOAP.Enabled", false))
    {
        soapThread = new std::thread(TCSoapThread, sConfigMgr->GetStringDefault("SOAP.IP", "127.0.0.1"), uint16(sConfigMgr->GetIntDefault("SOAP.Port", 7878)));
    }

    ///- Start up the IRC client
    if (sConfigMgr->GetBoolDefault("IRC.Enabled", false))
    {
        TC_LOG_INFO("server.worldserver", "IRC bridge is enabled.");
        sIRCMgr->startSessions();
    }

    // Launch the worldserver listener socket
    uint16 worldPort = uint16(sWorld->getIntConfig(CONFIG_PORT_WORLD));
    std::string worldListener = sConfigMgr->GetStringDefault("BindIP", "0.0.0.0");

	int networkThreads = sConfigMgr->GetIntDefault("Network.Threads", 1);

	if (networkThreads <= 0)
	{
		TC_LOG_ERROR("server.worldserver", "Network.Threads must be greater than 0");
		return false;
	}

    sWorldSocketMgr.StartNetwork(_ioService, worldListener, worldPort, networkThreads);
    TC_LOG_INFO("server.worldserver", "Start listening on %s:%u", worldListener.c_str(), worldPort);

    //    sScriptMgr->OnNetworkStart();

    // Set server online (allow connecting now)
    LoginDatabase.DirectPExecute("UPDATE realmlist SET flag = flag & ~%u, population = 0 WHERE id = '%u'", REALM_FLAG_VERSION_MISMATCH, realmID);

    // Start the freeze check callback cycle in 5 seconds (cycle itself is 1 sec)
    if (int coreStuckTime = sConfigMgr->GetIntDefault("MaxCoreStuckTime", 0))
    {
        _maxCoreStuckTimeInMs = coreStuckTime * 1000;
        _freezeCheckTimer.expires_from_now(boost::posix_time::seconds(5));
        _freezeCheckTimer.async_wait(FreezeDetectorHandler);
        TC_LOG_INFO("server.worldserver", "Starting up anti-freeze thread (%u seconds max stuck time)...", coreStuckTime);
    }

    TC_LOG_INFO("server.worldserver", "%s (worldserver-daemon) ready...", GitRevision::GetFullVersion());

  //  sScriptMgr->OnStartup();

    WorldUpdateLoop();

    // Shutdown starts here
    ShutdownThreadPool(threadPool);

    sLog->SetSynchronous();

    //  sScriptMgr->OnShutdown();

    sWorld->KickAll();                                     // save and kick all players
    sWorld->UpdateSessions(1);                             // real players unload required UpdateSessions call

    // unload battleground templates before different singletons destroyed
    sBattlegroundMgr->DeleteAllBattlegrounds();

    sWorldSocketMgr.StopNetwork();

    //sInstanceSaveMgr->Unload();
    sMapMgr->UnloadAll();                     // unload all grids (including locked in memory)
    sObjectAccessor->UnloadAll();
    sOutdoorPvPMgr->Die();
    sScriptMgr->Unload();
    sScriptReloadMgr->Unload();

    // set server offline
    LoginDatabase.DirectPExecute("UPDATE realmlist SET flag = flag | %u WHERE id = '%d'", REALM_FLAG_OFFLINE, realmID);

    // Clean up threads if any
    if (soapThread != nullptr)
    {
        soapThread->join();
        delete soapThread;
    }

    if (raAcceptor != nullptr)
        delete raAcceptor;

    ///- Clean database before leaving
    ClearOnlineAccounts();

    StopDB();

    TC_LOG_INFO("server.worldserver", "Halting process...");

    ShutdownCLIThread(cliThread);

    OpenSSLCrypto::threadsCleanup();

    // 0 - normal shutdown
    // 1 - shutdown at error
    // 2 - restart command used, this code can be used by restarter for restart Trinityd
    
    return World::GetExitCode();
}

void ShutdownCLIThread(std::thread* cliThread)
{
    if (cliThread != nullptr)
    {
#ifdef _WIN32
        // First try to cancel any I/O in the CLI thread
        if (!CancelSynchronousIo(cliThread->native_handle()))
        {
            // if CancelSynchronousIo() fails, print the error and try with old way
            DWORD errorCode = GetLastError();
            LPSTR errorBuffer;

            DWORD formatReturnCode = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
                                                   nullptr, errorCode, 0, (LPTSTR)&errorBuffer, 0, nullptr);
            if (!formatReturnCode)
                errorBuffer = "Unknown error";

            TC_LOG_DEBUG("server.worldserver", "Error cancelling I/O of CliThread, error code %u, detail: %s",
                errorCode, errorBuffer);
            LocalFree(errorBuffer);

            // send keyboard input to safely unblock the CLI thread
            INPUT_RECORD b[4];
            HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
            b[0].EventType = KEY_EVENT;
            b[0].Event.KeyEvent.bKeyDown = TRUE;
            b[0].Event.KeyEvent.uChar.AsciiChar = 'X';
            b[0].Event.KeyEvent.wVirtualKeyCode = 'X';
            b[0].Event.KeyEvent.wRepeatCount = 1;

            b[1].EventType = KEY_EVENT;
            b[1].Event.KeyEvent.bKeyDown = FALSE;
            b[1].Event.KeyEvent.uChar.AsciiChar = 'X';
            b[1].Event.KeyEvent.wVirtualKeyCode = 'X';
            b[1].Event.KeyEvent.wRepeatCount = 1;

            b[2].EventType = KEY_EVENT;
            b[2].Event.KeyEvent.bKeyDown = TRUE;
            b[2].Event.KeyEvent.dwControlKeyState = 0;
            b[2].Event.KeyEvent.uChar.AsciiChar = '\r';
            b[2].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
            b[2].Event.KeyEvent.wRepeatCount = 1;
            b[2].Event.KeyEvent.wVirtualScanCode = 0x1c;

            b[3].EventType = KEY_EVENT;
            b[3].Event.KeyEvent.bKeyDown = FALSE;
            b[3].Event.KeyEvent.dwControlKeyState = 0;
            b[3].Event.KeyEvent.uChar.AsciiChar = '\r';
            b[3].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
            b[3].Event.KeyEvent.wVirtualScanCode = 0x1c;
            b[3].Event.KeyEvent.wRepeatCount = 1;
            DWORD numb;
            WriteConsoleInput(hStdIn, b, 4, &numb);
        }
#endif
        cliThread->join();
        delete cliThread;
    }
}

/// Initialize connection to the databases
bool StartDB()
{
    MySQL::Library_Init();
	
    // Load databases
    DatabaseLoader loader("server.worldserver", DatabaseLoader::DATABASE_NONE);
    loader
        .AddDatabase(LoginDatabase, "Login")
        .AddDatabase(CharacterDatabase, "Character")
        .AddDatabase(WorldDatabase, "World");

	loader.AddDatabase(LogsDatabase, "Logs"); //Strange reference bug when I append this to the last command, so I kept this out
	auto a = nullptr;
	
    if (!loader.Load())
        return false;

    ///- Get the realm Id from the configuration file
    realm.Id.Realm = sConfigMgr->GetIntDefault("RealmID", 0);
    if (!realm.Id.Realm)
    {
        TC_LOG_ERROR("server.worldserver", "Realm ID not defined in configuration file");
        return false;
    }

    TC_LOG_INFO("server.worldserver", "Realm running as realm ID %d", realm.Id.Realm);

    ///- Clean the database before starting
    ClearOnlineAccounts();

    ///- Insert version info into DB
    WorldDatabase.PExecute("UPDATE version SET core_version = '%s', core_revision = '%s'", GitRevision::GetFullVersion(), GitRevision::GetHash());        // One-time query

    sWorld->LoadDBVersion();

    TC_LOG_INFO("server.worldserver", "Using World DB: %s", sWorld->GetDBVersion());
    return true;
}

void StopDB()
{
    CharacterDatabase.Close();
    WorldDatabase.Close();
    LoginDatabase.Close();
    LogsDatabase.Close();

    MySQL::Library_End();
}

/// Clear 'online' status for all accounts with characters in this realm
void ClearOnlineAccounts()
{
    // Reset online status for all accounts with characters on the current realm
    LoginDatabase.DirectPExecute("UPDATE account SET online = 0 WHERE online > 0 AND id IN (SELECT acctid FROM realmcharacters WHERE realmid = %d)", realmID);

    // Reset online status for all characters
    CharacterDatabase.DirectExecute("UPDATE characters SET online = 0 WHERE online <> 0");

    // Battleground instance ids reset at server restart
  //  CharacterDatabase.DirectExecute("UPDATE character_battleground_data SET instanceId = 0");
}


variables_map GetConsoleArguments(int argc, char** argv,  fs::path& configFile, std::string& configService)
{
    // Silences warning about configService not be used if the OS is not Windows
    (void)configService;

    options_description all("Allowed options");
	all.add_options()
		("help,h", "print usage message")
		("version,v", "print version build info")
		("config,c", value<fs::path>(&configFile)->default_value(fs::absolute(_WORLD_SERVER_CONFIG)),
			"use <arg> as configuration file");
#ifdef _WIN32
    options_description win("Windows platform specific options");
    win.add_options()
        ("service,s", value<std::string>(&configService)->default_value(""), "Windows service options: [install | uninstall]")
        ;

    all.add(win);
#endif
    variables_map vm;
    try
    {
        store(command_line_parser(argc, argv).options(all).allow_unregistered().run(), vm);
        notify(vm);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    if (vm.count("help")) {
        std::cout << all << "\n";
    }

    return vm;
}

void ShutdownThreadPool(std::vector<std::thread>& threadPool)
{
    //    sScriptMgr->OnNetworkStop();

    _ioService.stop();

    for (auto& thread : threadPool)
    {
        thread.join();
    }
}

void FreezeDetectorHandler(const boost::system::error_code& error)
{
    if (!error)
    {
        uint32 curtime = GetMSTime();

        uint32 worldLoopCounter = World::m_worldLoopCounter;
        if (_worldLoopCounter != worldLoopCounter)
        {
            _lastChangeMsTime = curtime;
            _worldLoopCounter = worldLoopCounter;
        }
        // possible freeze
        else if (GetMSTimeDiff(_lastChangeMsTime, curtime) > _maxCoreStuckTimeInMs)
        {
            TC_LOG_ERROR("server.worldserver", "World Thread hangs, kicking out server!");
            ABORT();
        }

        _freezeCheckTimer.expires_from_now(boost::posix_time::seconds(1));
        _freezeCheckTimer.async_wait(FreezeDetectorHandler);
    }
}

void WorldUpdateLoop()
{
    uint32 realCurrTime = 0;
    uint32 realPrevTime = GetMSTime();

    uint32 prevSleepTime = 0;                               // used for balanced full tick time length near WORLD_SLEEP_CONST

    ///- While we have not World::m_stopEvent, update the world
    while (!World::IsStopped())
    {
        ++World::m_worldLoopCounter;
        realCurrTime = GetMSTime();

        uint32 diff = GetMSTimeDiff(realPrevTime, realCurrTime);

        sWorld->Update(diff);
        realPrevTime = realCurrTime;

        // diff (D0) include time of previous sleep (d0) + tick time (t0)
        // we want that next d1 + t1 == WORLD_SLEEP_CONST
        // we can't know next t1 and then can use (t0 + d1) == WORLD_SLEEP_CONST requirement
        // d1 = WORLD_SLEEP_CONST - t0 = WORLD_SLEEP_CONST - (D0 - d0) = WORLD_SLEEP_CONST + d0 - D0
        if (diff <= WORLD_SLEEP_CONST + prevSleepTime)
        {
            prevSleepTime = WORLD_SLEEP_CONST + prevSleepTime - diff;

            std::this_thread::sleep_for(std::chrono::milliseconds(prevSleepTime));
        }
        else
            prevSleepTime = 0;

#ifdef _WIN32
        if (m_ServiceStatus == 0)
            World::StopNow(SHUTDOWN_EXIT_CODE);

        while (m_ServiceStatus == 2)
            Sleep(1000);
#endif
    }
}

void SignalHandler(const boost::system::error_code& error, int /*signalNumber*/)
{
    if (!error)
        World::StopNow(SHUTDOWN_EXIT_CODE);
}

AsyncAcceptor* StartRaSocketAcceptor(boost::asio::io_service& ioService)
{
    uint16 raPort = uint16(sConfigMgr->GetIntDefault("Ra.Port", 3443));
    std::string raListener = sConfigMgr->GetStringDefault("Ra.IP", "0.0.0.0");
    TC_LOG_INFO("server.worldserver", "Starting RA listener on address '%s' and port %u", raListener.c_str(), raPort);

    auto  acceptor = new AsyncAcceptor(ioService, raListener, raPort);
    acceptor->AsyncAccept<RASession>();
    return acceptor;
}

bool LoadRealmInfo()
{
	boost::asio::ip::tcp::resolver resolver(_ioService);
	boost::asio::ip::tcp::resolver::iterator end;

	QueryResult result = LoginDatabase.PQuery("SELECT id, name, address, localAddress, localSubnetMask, port, icon, flag, timezone, allowedSecurityLevel, population, gamebuild FROM realmlist WHERE id = %u", realm.Id.Realm);
	if (!result)
		return false;

	Field* fields = result->Fetch();
	realm.Name = fields[1].GetString();
	boost::asio::ip::tcp::resolver::query externalAddressQuery(ip::tcp::v4(), fields[2].GetString(), "");

	boost::system::error_code ec;
	boost::asio::ip::tcp::resolver::iterator endPoint = resolver.resolve(externalAddressQuery, ec);
	if (endPoint == end || ec)
	{
		TC_LOG_ERROR("server.worldserver", "Could not resolve address %s", fields[2].GetString().c_str());
		return false;
	}

	realm.ExternalAddress = (*endPoint).endpoint().address();

	boost::asio::ip::tcp::resolver::query localAddressQuery(ip::tcp::v4(), fields[3].GetString(), "");
	endPoint = resolver.resolve(localAddressQuery, ec);
	if (endPoint == end || ec)
	{
		TC_LOG_ERROR("server.worldserver", "Could not resolve address %s", fields[3].GetString().c_str());
		return false;
	}

	realm.LocalAddress = (*endPoint).endpoint().address();

	boost::asio::ip::tcp::resolver::query localSubmaskQuery(ip::tcp::v4(), fields[4].GetString(), "");
	endPoint = resolver.resolve(localSubmaskQuery, ec);
	if (endPoint == end || ec)
	{
		TC_LOG_ERROR("server.worldserver", "Could not resolve address %s", fields[4].GetString().c_str());
		return false;
	}

	realm.LocalSubnetMask = (*endPoint).endpoint().address();

	realm.Port = fields[5].GetUInt16();
	realm.Type = fields[6].GetUInt8();
	realm.Flags = RealmFlags(fields[7].GetUInt8());
	realm.Timezone = fields[8].GetUInt8();
	realm.AllowedSecurityLevel = AccountTypes(fields[9].GetUInt8());
	realm.PopulationLevel = fields[10].GetFloat();
	realm.Build = fields[11].GetUInt32();
	return true;
}
/// @}
