    
#ifndef TRINITYCORE_COMMON_H
#define TRINITYCORE_COMMON_H

// config.h needs to be included 1st
// TODO this thingy looks like hack ,but its not, need to
// make separate header however, because It makes mess here.
#ifdef HAVE_CONFIG_H
// Remove Some things that we will define
// This is in case including another config.h
// before trinity config.h
#ifdef PACKAGE
#undef PACKAGE
#endif //PACKAGE
#ifdef PACKAGE_BUGREPORT
#undef PACKAGE_BUGREPORT
#endif //PACKAGE_BUGREPORT
#ifdef PACKAGE_NAME
#undef PACKAGE_NAME
#endif //PACKAGE_NAME
#ifdef PACKAGE_STRING
#undef PACKAGE_STRING
#endif //PACKAGE_STRING
#ifdef PACKAGE_TARNAME
#undef PACKAGE_TARNAME
#endif //PACKAGE_TARNAME
#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif //PACKAGE_VERSION
#ifdef VERSION
#undef VERSION
#endif //VERSION
# include "config.h"
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION
#endif //HAVE_CONFIG_H

#include "Define.h"
#include <memory>
#include <string>
#include <utility>

#include "Debugging/Errors.h"

#if COMPILER == TRINITY_COMPILER_MICROSOFT

#pragma warning(disable:4996)

#ifndef __SHOW_STUPID_WARNINGS__

#pragma warning(disable:4244)

#pragma warning(disable:4267)

#pragma warning(disable:4800)

#pragma warning(disable:4018)

#pragma warning(disable:4311)

#pragma warning(disable:4305)

#pragma warning(disable:4005)
#endif                                                      // __SHOW_STUPID_WARNINGS__
#endif                                                      // __GNUC__

#include <unordered_map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>

#if TRINITY_PLATFORM == TRINITY_PLATFORM_WINDOWS
#define STRCASECMP stricmp
#else
#define STRCASECMP strcasecmp
#endif

#include <set>
#include <list>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>
#include <memory>

#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/functional/hash.hpp>

#include "Threading/LockedQueue.h"

#if TRINITY_PLATFORM == TRINITY_PLATFORM_WINDOWS
#  define FD_SETSIZE 4096
// XP winver - needed to compile with standard leak check in MemoryLeaks.h
// uncomment later if needed
//#define _WIN32_WINNT 0x0501
#  include <ws2tcpip.h>
//#undef WIN32_WINNT
#else
#  include <sys/types.h>
#  include <sys/ioctl.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
#  include <netdb.h>
#endif

#if COMPILER == TRINITY_COMPILER_MICROSOFT

#include <float.h>

#define snprintf _snprintf
#define atoll _atoi64
#define vsnprintf _vsnprintf
#define strdup _strdup
#define llabs _abs64

#else

#define stricmp strcasecmp
#define strnicmp strncasecmp

#endif

inline float finiteAlways(float f) { return std::isfinite(f) ? f : 0.0f; }

inline unsigned long atoul(char const* str) { return strtoul(str, nullptr, 10); }
inline unsigned long long atoull(char const* str) { return strtoull(str, nullptr, 10); }

#define STRINGIZE(a) #a

enum TimeConstants
{
    SECOND = 1,
    MINUTE = 60,
    HOUR   = MINUTE*60,
    DAY    = HOUR*24,
    WEEK   = DAY*7,
    MONTH  = DAY*30,
    YEAR   = MONTH*12,
    IN_MILLISECONDS = 1000
};

enum AccountTypes
{
    SEC_PLAYER         = 0,
    SEC_GAMEMASTER1    = 1,
    SEC_GAMEMASTER2    = 2,
    SEC_GAMEMASTER3    = 3,
    SEC_ADMINISTRATOR  = 4,
    SEC_SUPERADMIN     = 5,
    SEC_CONSOLE        = 6, // must be always last in list, accounts must have less security level always also
};
/*
enum GMGroups
{
    GMGROUP_VIDEO   = 1, //can't see invisible units/gobjects
    GMGROUP_ANIM    = 2, //normal gm minus some commands
    GMGROUP_PTRGM   = 3, //only gm if CONFIG_TESTSERVER_ENABLE
    GMGROUP_TESTER  = 4, //can only spectate to track bugs
    GMGROUP_SPY     = 5, //can't be seen by other gm's with lesser ranks
};
*/
enum LocaleConstant : uint8
{
    LOCALE_enUS = 0,
    LOCALE_koKR = 1,
    LOCALE_frFR = 2,
    LOCALE_deDE = 3,
    LOCALE_zhCN = 4,
    LOCALE_zhTW = 5,
    LOCALE_esES = 6,
    LOCALE_esMX = 7,
    LOCALE_ruRU = 8
};

const uint8 TOTAL_LOCALES = 9;
#define MAX_LOCALE 8
#define MAX_ACCOUNT_TUTORIAL_VALUES 8

#define DEFAULT_LOCALE LOCALE_enUS

TC_COMMON_API extern char const* localeNames[TOTAL_LOCALES];

TC_COMMON_API LocaleConstant GetLocaleByName(const std::string& name);

typedef std::vector<std::string> StringVector;

// we always use stdlibc++ std::max/std::min, undefine some not C++ standard defines (Win API and some pother platforms)
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifndef M_PI
#define M_PI            3.14159265358979323846f
#endif

#define MAX_QUERY_LEN 32*1024

//! Optional helper class to wrap optional values within.
template <typename T>
using Optional = boost::optional<T>;

namespace Trinity
{
    using std::make_unique;
}

//! Hash implementation for std::pair to allow using pairs in unordered_set or as key for unordered_map
//! Individual types used in pair must be hashable by boost::hash
namespace std
{
    template<class K, class V>
    struct hash<std::pair<K, V>>
    {
    public:
        size_t operator()(std::pair<K, V> const& key) const
        {
            return boost::hash_value(key);
        }
    };
}

enum ClientBuild : uint32
{
    BUILD_335 = 12340,
    BUILD_243 = 8606,
};

#endif

