project(Sunstrider)
cmake_minimum_required(VERSION 2.8.7)

# Don't escape preprocessor definitions automatically.
cmake_policy(SET CMP0005 OLD)
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 OLD) # Disable 'Ignore COMPILE_DEFINITIONS_<Config> properties'
endif(POLICY CMP0043)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

#install sunstrider compiled libraries to this directory
set(SUNLIBDIR ${CMAKE_INSTALL_PREFIX}/lib)

# Set RPATH-handing (CMake parameters)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${LIBSDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")

include(CheckIncludeFiles)
include(CheckPlatform)
include(PlatformRelatedSettings)

include(FindThreads)
include(FindMySql)
include(FindReadline)
include(ConfigureBoost)
include(FindPCHSupport)

# set default buildoptions and print them
include(cmake/options.cmake)

if(UNIX)
  find_package(ZLIB)
  set(ZLIB ${ZLIB_LIBRARY})
  find_package(BZip2)
endif()

find_package(OpenSSL)

if(OPENSSL_FOUND)
  message(STATUS "Found OpenSSL library (version ${OPENSSL_VERSION}) : ${OPENSSL_LIBRARIES}")
  message(STATUS "Include dir is: ${OPENSSL_INCLUDE_DIR}")
else(OPENSSL_FOUND)
  message(SEND_ERROR "** OpenSSL library not found! Sunstrider cannot be compiled!")
  return()
endif(OPENSSL_FOUND)

find_package(Git) 
FIND_MYSQL()

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

if(UNIX)
  FIND_READLINE()
  if(READLINE_FOUND)
    message(STATUS "Found Readline library: ${READLINE_LIBRARY}")
    message(STATUS "Include dir is: ${READLINE_INCLUDE_DIR}")
  else(READLINE_FOUND)
    message(SEND_ERROR "** Readline library not found!")
    message(SEND_ERROR "** Your distro may provide a binary for Readline e.g. for ubuntu try apt-get install libreadline5-dev")
    return()
  endif(READLINE_FOUND)
endif(UNIX)

if(PROFILING)
  add_definitions(-DPROFILING)
endif()

if(PLAYERBOT)
  add_definitions(-DPLAYERBOT)
endif()

if(TESTS)
	#this variable so that library trying to link testing lib don't complain
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
