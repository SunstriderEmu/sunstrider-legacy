PROJECT(Sunstrider)
cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0005 OLD)

include(CheckIncludeFiles)
include(cmake/CheckPlatform.cmake)
include(cmake/FindAce.cmake)
include(cmake/FindMySql.cmake)
include(cmake/FindPostgreSql.cmake)
include(cmake/FindReadline.cmake)
include(cmake/ConfigureBoost.cmake)
include(cmake/FindPCHSupport.cmake)
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

OPTION(DO_MYSQL "With MySQL support" 1)
OPTION(DO_POSTGRESQL "With PostgreSQL support" 0) # not complete yet, may not link properly.
OPTION(DO_SCRIPTS "With trinityscripts" 1)
OPTION(DO_CLI "With CLI" 1)
OPTION(DO_RA "With RA" 0)
OPTION(DO_DEBUG "Debug mode" 0)
OPTION(DO_WARN "Enable all compile warnings" 0)
OPTION(LARGE_CELL "Large cell size" 0)
OPTION(SHORT_SLEEP "Short sleep" 0)
OPTION(USE_EMBEDDED_MYSQL_LIB "Use integrated mysqllib (32 bit only). You can use this if you only have the x64 version and want to compile on 32 bits." 0)
OPTION(TOOLS "Build map/vmap/mmap extraction/assembler tools" 0)



IF (PREFIX)
    SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ENDIF (PREFIX)

if(CONF_DIR)

else(CONF_DIR)
SET(CONF_DIR ${PREFIX}/etc)
endif(CONF_DIR)
SET(LIBSDIR ${CMAKE_INSTALL_PREFIX}/lib)

message("* Will install to: ${CMAKE_INSTALL_PREFIX}")
message("* With config dir at: ${CONF_DIR}")
message("* Libs install dir at: ${LIBSDIR}")

FIND_PACKAGE(OpenSSL)

if(OPENSSL_FOUND)
message(STATUS "Found OpenSSL library (version ${OPENSSL_VERSION}) : ${OPENSSL_LIBRARIES}")
message(STATUS "Include dir is: ${OPENSSL_INCLUDE_DIR}")
else(OPENSSL_FOUND)
message(SEND_ERROR "** OpenSSL library not found! Sunstrider cannot be compiled!")
return()
endif(OPENSSL_FOUND)

FIND_PACKAGE(ZLIB)
set(ZLIB ${ZLIB_LIBRARY})
FIND_PACKAGE(BZip2)

IF(DO_POSTGRESQL)
    MESSAGE("* With PostgreSQL")
    FIND_PostgreSQL()
    ADD_DEFINITIONS(-DDO_POSTGRESQL)
ENDIF(DO_POSTGRESQL)

IF(DO_MYSQL)
    MESSAGE("* With MySQL")
    ADD_DEFINITIONS(-DDO_MYSQL)

    #BAAAAD
    # IF (WIN32)
        # set (MYSQL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dep/include/mysql)
        # set (MYSQL_LIBRARY ${PROJECT_SOURCE_DIR}/dep/lib/win32_release/libmysql.lib)
    # ELSE()
        FIND_MYSQL()
    # ENDIF()
ENDIF(DO_MYSQL)

IF(USE_EMBEDDED_MYSQL_LIB)
	set (MYSQL_LIBRARY ${PROJECT_SOURCE_DIR}/dep/lib/win32_release/libmysql.lib)
	MESSAGE("* USE_EMBEDDED_MYSQL_LIB was set")
ENDIF(USE_EMBEDDED_MYSQL_LIB)

IF (WIN32)
	set (LIBIRCCLIENT_LIB_WIN32 ${PROJECT_SOURCE_DIR}/dep/lib/win32_release/libircclient.lib)
ENDIF()

if(DO_SCRIPTS)
message("* With Trinity Scripts")
#SET(SCRIPT_LIB trinityscript)
SET(SCRIPT_INCLUDE src/scripts/include)
ADD_DEFINITIONS(-DDO_SCRIPTS)
else (DO_SCRIPTS)
message("* Without Trinity Scripts")
SET(SCRIPT_LIB trinityinterface)
SET(SCRIPT_INCLUDE src/bindings/interface)
endif(DO_SCRIPTS)

message("-- Miscellaneus options:")

if(DO_CLI)
message("* With CLI")
add_definitions(-DENABLE_CLI)
else (DO_CLI)
message(* Without CLI)
endif(DO_CLI)

if(DO_RA)
message("* With RA")
add_definitions(-DENABLE_RA)
else(DO_RA)
message("* Without RA")
endif(DO_RA)

if(DO_DEBUG)
message("* Debug mode ON")
    IF (UNIX)
        add_definitions(-g -ggdb)
    ELSEIF(WIN32)
        add_definitions(/Od)
    ENDIF()
    add_definitions(-DTRINITY_DEBUG)
endif(DO_DEBUG)

if(WITH_UNIT_CRASHFIX)
add_definitions(-DWITH_UNIT_CRASHFIX)
endif(WITH_UNIT_CRASHFIX)

if(DO_WARN)
    message("* All warnings mode")
    add_definitions(-Wall -Wfatal-errors -Wextra)
endif(DO_WARN)

if(LARGE_CELL)
message("* Large cell size")
add_definitions(-DLARGE_CELL)
else(LARGE_CELL)
message("* Small cell size")
endif(LARGE_CELL)

IF (UNIX)
    message("* Building with readline")
ENDIF()

#find_package(OpenSSL REQUIRED)
FIND_ACE(ACE)
if(ACE_FOUND)
message(STATUS "Found ACE library: ${ACE_LIBRARY}")
message(STATUS "Include dir is: ${ACE_INCLUDE_DIR}")
else(ACE_FOUND)
message(SEND_ERROR "** ACE library not found! Trinity Core cannot be compiled!")
message(SEND_ERROR "** Please build ACE from http://www.cs.wustl.edu/~schmidt/ACE.html")
#For now remove msg about install from repo, as ubuntu/debian don't have needed ver in repos.
#message(SEND_ERROR "** your distro may provide a binary for ACE e.g. for ubuntu try apt-get install libace-dev")
return()
#SET(BUILD_ACE 1)
#SET(ACE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dep/ACE_wrappers ${CMAKE_BINARY_DIR}/dep/ACE_wrappers")
#SET(ACE_LIBRARY ACE)
#message(STATUS "I will try to build ACE from: ${ACE_INCLUDE_DIR}")
#message(STATUS "And link using: ${ACE_LIBRARY}")
endif(ACE_FOUND)

#somehow line below don't work. so for now change it to if exist
#check_include_files(${ACE_INCLUDE_DIR}/ace/Stack_Trace.h HAVE_ACE_STACK_TRACE_H)
IF(EXISTS ${ACE_INCLUDE_DIR}/ace/Stack_Trace.h)
    SET(HAVE_ACE_STACK_TRACE_H 1)
ELSE(EXISTS ${ACE_INCLUDE_DIR}/ace/Stack_Trace.h)
    message(STATUS "** Your libace is out of date. Please update your libace!")
ENDIF(EXISTS ${ACE_INCLUDE_DIR}/ace/Stack_Trace.h)

IF(UNIX)
  FIND_READLINE()
  IF(READLINE_FOUND)
  message(STATUS "Found Readline library: ${READLINE_LIBRARY}")
  message(STATUS "Include dir is: ${READLINE_INCLUDE_DIR}")
  ELSE(READLINE_FOUND)
  message(SEND_ERROR "** Readline library not found!")
  message(SEND_ERROR "** Your distro may provide a binary for Readline e.g. for ubuntu try apt-get install libreadline5-dev")
  return()
  ENDIF(READLINE_FOUND)
ENDIF(UNIX)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  SET(OSX_LIBS /opt/local/lib/libcrypto.dylib)
  add_definitions(-D__ASSERTMACROS__)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")

IF(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --no-warnings")
#    add_definitions(--no-warnings)  #to make build look nice, no gcc nazi warnings.
#    add_definitions(-std=c++11)     #enable c++11 standard
ELSEIF(WIN32)
    add_definitions(/MP)
    IF(DO_DEBUG)
        add_definitions(/Od /Ob0)
    ENDIF()
ENDIF ()

if(PROFILING)
    message(STATUS "Built-in profiling enabled")
    add_definitions(-DPROFILING)
else()
    message(STATUS "Built-in profiling disabled")
endif()

if (WIN32)
    INCLUDE_DIRECTORIES(src/win)
endif (WIN32)

SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory(dep)
#add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(sql)
add_subdirectory(contrib)
