
cmake_minimum_required(VERSION 3.11)

# add this options before PROJECT keyword
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON) 

project(Sunstrider)

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 OLD) # New cmake 1.12 policy, keep the old behavior for now 
endif(POLICY CMP0074)

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW) # Only interpret if() arguments as variables or keywords when unquoted - prevents intepreting if (SOME_STRING_VARIABLE MATCHES "MSVC") as if (SOME_STRING_VARIABLE MATCHES "1")
endif()

# Set RPATH-handing (CMake parameters)
set(CMAKE_SKIP_BUILD_RPATH 0)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 0)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# set macro-directory
list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}/cmake/macros"
  "${CMAKE_SOURCE_DIR}/dep/cotire/CMake")
  
# build in Release-mode by default if not explicitly set
if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

include(CheckIncludeFiles)
include(ConfigureScripts)

# set default buildoptions and print them
include(cmake/options.cmake)

include(ConfigureBaseTargets)
include(CheckPlatform)
include(GroupSources)
include(AutoCollect)
include(PlatformRelatedSettings)

include(FindThreads)
find_package(MySQL)
include(FindPCHSupport)

find_package(Git) 

if(DO_DEBUG)
    add_definitions(-DTRINITY_DEBUG)
endif(DO_DEBUG)

if(USE_GPERFTOOLS)
    add_definitions(-DUSE_GPERFTOOLS)
endif(USE_GPERFTOOLS)

if(PLAYERBOT)
    add_definitions(-DPLAYERBOT)
endif()

if(TESTS)
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

if(DO_DEBUG)
	message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
endif(DO_DEBUG)
