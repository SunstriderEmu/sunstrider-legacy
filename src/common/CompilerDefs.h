
#ifndef TRINITY_COMPILERDEFS_H
#define TRINITY_COMPILERDEFS_H

#define TRINITY_PLATFORM_WINDOWS 0
#define TRINITY_PLATFORM_UNIX    1
#define TRINITY_PLATFORM_APPLE   2
#define TRINITY_PLATFORM_INTEL   3

// must be first (win 64 also define WIN32)
#if defined( _WIN64 )
#  define TRINITY_PLATFORM TRINITY_PLATFORM_WINDOWS
#elif defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define TRINITY_PLATFORM TRINITY_PLATFORM_WINDOWS
#elif defined( __APPLE_CC__ )
#  define TRINITY_PLATFORM TRINITY_PLATFORM_APPLE
#elif defined( __INTEL_COMPILER )
#  define TRINITY_PLATFORM TRINITY_PLATFORM_INTEL
#else
#  define TRINITY_PLATFORM TRINITY_PLATFORM_UNIX
#endif

#define TRINITY_COMPILER_MICROSOFT 0
#define TRINITY_COMPILER_GNU       1
#define TRINITY_COMPILER_BORLAND   2
#define TRINITY_COMPILER_INTEL     3

#ifdef _MSC_VER
#  define COMPILER TRINITY_COMPILER_MICROSOFT
#elif defined( __BORLANDC__ )
#  define COMPILER TRINITY_COMPILER_BORLAND
#elif defined( __INTEL_COMPILER )
#  define COMPILER TRINITY_COMPILER_INTEL
#elif defined( __GNUC__ )
#  define COMPILER TRINITY_COMPILER_GNU
#else
#  pragma error "FATAL ERROR: Unknown compiler."
#endif

#if COMPILER == TRINITY_COMPILER_MICROSOFT
#  pragma warning( disable : 4267 )                         // conversion from 'size_t' to 'int', possible loss of data
#  pragma warning( disable : 4786 )                         // identifier was truncated to '255' characters in the debug information
#endif
#endif

