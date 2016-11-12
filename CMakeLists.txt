project(Sunstrider)
cmake_minimum_required(VERSION 2.8.7)

# Don't escape preprocessor definitions automatically.
cmake_policy(SET CMP0005 OLD)
if(POLICY CMP0043)
    cmake_policy(SET CMP0043 OLD) # Disable 'Ignore COMPILE_DEFINITIONS_<Config> properties'
endif(POLICY CMP0043)

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW) # Only interpret if() arguments as variables or keywords when unquoted - prevents intepreting if (SOME_STRING_VARIABLE MATCHES "MSVC") as if (SOME_STRING_VARIABLE MATCHES "1")
endif()

# add this options before PROJECT keyword
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

#install sunstrider compiled libraries to this directory
set(SUNLIBDIR ${CMAKE_INSTALL_PREFIX}/lib)

# Set RPATH-handing (CMake parameters)
set(CMAKE_SKIP_BUILD_RPATH 0)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 0)
set(CMAKE_INSTALL_RPATH "${LIBSDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")

# build in Release-mode by default if not explicitly set
if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE "Release")
endif()

include(CheckIncludeFiles)
include(ConfigureScripts)

# set default buildoptions and print them
include(cmake/options.cmake)

include(CheckPlatform)
include(GroupSources)
include(AutoCollect)
include(PlatformRelatedSettings)

include(FindThreads)
find_package(MySQL)
include(FindPCHSupport)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

if(UNIX)
    find_package(ZLIB)
    if(ZLIB_FOUND)
        message(STATUS "Found ZLib library (version ${ZLIB_VERSION_STRING}): ${ZLIB_LIBRARY}")
    else(ZLIB_FOUND)
        message(SEND_ERROR "** ZLib library not found! Sunstrider cannot be compiled!")
        return()
    endif(ZLIB_FOUND)

    find_package(BZip2)
    if(BZIP2_FOUND)
        message(STATUS "Found BZip2 libraries (version ${BZIP2_VERSION_STRING}): ${BZIP2_LIBRARIES}")
    else(BZIP2_FOUND)
        message(SEND_ERROR "** BZip2 libraries not found! Sunstrider cannot be compiled!")
        return()
    endif(BZIP2_FOUND)
endif()

find_package(Git) 

#For Windows, use prebuilt libircclient library instead of building it
if (WIN32)
    if(PLATFORM EQUAL 64)
        set (LIBIRCCLIENT_LIB_WIN ${PROJECT_SOURCE_DIR}/dep/lib/x64_release/libircclient.lib)
    elseif(PLATFORM EQUAL 32)
        set (LIBIRCCLIENT_LIB_WIN ${PROJECT_SOURCE_DIR}/dep/lib/win32_release/libircclient.lib)
    else()
        message(SEND_ERROR "Could not find platform when trying to set LIBIRCCLIENT_LIB_WIN")
    endif()
endif()

if(DO_DEBUG)
    add_definitions(-DTRINITY_DEBUG)
endif(DO_DEBUG)

if(PLAYERBOT)
    add_definitions(-DPLAYERBOT)
endif()

if(TESTS)
    #this variable so that libraries trying to link testing lib don't complain
    set(TESTING_LIB testing)
    add_definitions(-DTESTS)
endif()

# Find revision ID and hash of the sourcetree
include(cmake/genrev.cmake)

# print out the results before continuing
include(cmake/showoptions.cmake)

# add dependencies
add_subdirectory(dep)

# add core sources
add_subdirectory(src)

