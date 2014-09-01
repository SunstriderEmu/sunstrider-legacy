
########### next target ###############

file(GLOB_RECURSE sources_CommandLine CommandLine/*.cpp CommandLine/*.h)
file(GLOB_RECURSE sources_RemoteAccess RemoteAccess/*.cpp RemoteAccess/*.h)
file(GLOB_RECURSE sources_TCSoap TCSoap/*.cpp TCSoap/*.h)
file(GLOB sources_localdir *.cpp *.h)

if (USE_COREPCH)
  set(worldserver_PCH_HDR PrecompiledHeaders/worldPCH.h)
  set(worldserver_PCH_SRC PrecompiledHeaders/worldPCH.cpp)
endif()

set(worldserver_SRCS
  ${worldserver_SRCS}
  ${sources_CommandLine}
  ${sources_RemoteAccess}
  ${sources_TCSoap}
  ${sources_localdir}
)

if( WIN32 )
  set(worldserver_SRCS
    ${worldserver_SRCS}
    ${sources_windows_Debugging}
  )
  if ( MSVC )
    set(worldserver_SRCS
      ${worldserver_SRCS}
      worldserver.rc
    )
  endif()
endif()


INCLUDE_DIRECTORIES(  
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/dep/gsoap
    ${CMAKE_SOURCE_DIR}/dep/include
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
    ${CMAKE_SOURCE_DIR}/src/framework
    ${CMAKE_SOURCE_DIR}/src/framework/Platform
    ${CMAKE_SOURCE_DIR}/src/game
    ${CMAKE_SOURCE_DIR}/src/game/Accounts
    ${CMAKE_SOURCE_DIR}/src/game/DataStores
    ${CMAKE_SOURCE_DIR}/src/game/Server
    ${CMAKE_SOURCE_DIR}/src/game/Server/Protocol
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/src/shared/Configuration
    ${CMAKE_SOURCE_DIR}/src/shared/Cryptography
    ${CMAKE_SOURCE_DIR}/src/shared/Cryptography/Authentication
    ${CMAKE_SOURCE_DIR}/src/shared/Database
    ${CMAKE_SOURCE_DIR}/src/shared/Dynamic
    ${CMAKE_SOURCE_DIR}/src/shared/Dynamic/LinkedReference
    ${CMAKE_SOURCE_DIR}/src/shared/Logging
    ${CMAKE_SOURCE_DIR}/src/shared/Networking
    ${CMAKE_SOURCE_DIR}/src/shared/Packets
    ${CMAKE_SOURCE_DIR}/src/shared/Utilities
    ${CMAKE_SOURCE_DIR}/src/shared/Threading
    ${ACE_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
)

add_executable(worldserver ${worldserver_SRCS})

IF ( UNIX )
add_definitions(
-D_WORLD_SERVER_CONFIG='"${CONF_DIR}/worldserver.conf"'
)
endif()

add_dependencies(worldserver revision.h)

if( UNIX AND NOT NOJEM AND NOT APPLE )
  set(worldserver_LINK_FLAGS "-pthread -lncurses ${worldserver_LINK_FLAGS}")
endif()

SET_TARGET_PROPERTIES(worldserver PROPERTIES LINK_FLAGS "${worldserver_LINK_FLAGS}")

target_link_libraries(
worldserver
Detour
collision
scripts
game
shared
zlib
trinityframework
trinitysockets
trinityauth
trinityconfig
wrchat
ZThread
g3dlite
${READLINE_LIBRARY}
${UNIX_LIBS}
${MYSQL_LIBRARY}
${POSTGRESQL_LIBRARIES}
${OPENSSL_LIBRARIES}
${ACE_LIBRARY}
${ZLIB}
${OSX_LIBS}
)

IF (UNIX)
    SET(UNIX_LIBS gomp malloc ${CMAKE_SOURCE_DIR}/dep/libircclient/src/libircclient.so)
ENDIF()

if( UNIX )
    install(TARGETS worldserver DESTINATION bin)
    install(FILES worldserver.conf.dist DESTINATION etc)
        install(FILES ${CMAKE_SOURCE_DIR}/dep/libircclient/src/libircclient.so DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
        set_target_properties(worldserver PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
elseif( WIN32 )
  install(TARGETS worldserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES worldserver.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()