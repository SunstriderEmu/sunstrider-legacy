
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


include_directories(  
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/dep/cppformat
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
    ${CMAKE_SOURCE_DIR}/dep/gsoap
    ${CMAKE_SOURCE_DIR}/dep/
    ${CMAKE_SOURCE_DIR}/dep/libircclient/include
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
    ${CMAKE_SOURCE_DIR}/dep/zlib
    ${CMAKE_SOURCE_DIR}/src/authserver/Realms
    ${CMAKE_SOURCE_DIR}/src/collision
    ${CMAKE_SOURCE_DIR}/src/framework
    ${CMAKE_SOURCE_DIR}/src/framework/Platform
    ${CMAKE_SOURCE_DIR}/src/game
    ${CMAKE_SOURCE_DIR}/src/game/Accounts
    ${CMAKE_SOURCE_DIR}/src/game/DataStores
    ${CMAKE_SOURCE_DIR}/src/game/Movement
    ${CMAKE_SOURCE_DIR}/src/game/Movement/MovementGenerators
    ${CMAKE_SOURCE_DIR}/src/game/Movement/Spline
    ${CMAKE_SOURCE_DIR}/src/game/Movement/Waypoints
    ${CMAKE_SOURCE_DIR}/src/game/Server
    ${CMAKE_SOURCE_DIR}/src/game/Server/Protocol
    ${CMAKE_SOURCE_DIR}/src/scripts/include
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/src/shared/Configuration
    ${CMAKE_SOURCE_DIR}/src/shared/Cryptography
    ${CMAKE_SOURCE_DIR}/src/shared/Cryptography/Authentication
    ${CMAKE_SOURCE_DIR}/src/shared/Database
    ${CMAKE_SOURCE_DIR}/src/shared/DataStores
    ${CMAKE_SOURCE_DIR}/src/shared/Dynamic
    ${CMAKE_SOURCE_DIR}/src/shared/Dynamic/LinkedReference
    ${CMAKE_SOURCE_DIR}/src/shared/Logging
    ${CMAKE_SOURCE_DIR}/src/shared/Networking
    ${CMAKE_SOURCE_DIR}/src/shared/Packets
    ${CMAKE_SOURCE_DIR}/src/shared/Utilities
    ${CMAKE_SOURCE_DIR}/src/shared/Threading
    ${CMAKE_SOURCE_DIR}/src/irc
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandLine
    ${CMAKE_CURRENT_SOURCE_DIR}/RemoteAccess
    ${CMAKE_CURRENT_SOURCE_DIR}/TCSoap
    ${OPENSSL_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
)

add_executable(worldserver ${worldserver_SRCS})

if ( UNIX )
add_definitions(
    -D_WORLD_SERVER_CONFIG='"${CONF_DIR}/worldserver.conf"'
)
endif()

add_dependencies(worldserver revision.h game scripts irc)

if( UNIX )
    set(worldserver_LINK_FLAGS "-pthread ${worldserver_LINK_FLAGS}")
endif()

if ( WIN32 )
	set(worldserver_LINK_FLAGS ${worldserver_LINK_FLAGS} "/LARGEADDRESSAWARE")
endif()

set_target_properties(worldserver PROPERTIES LINK_FLAGS "${worldserver_LINK_FLAGS}")

if (UNIX)
    #this should be changed, see comments is CMakeLists from irc project
    set(UNIX_LIBS gomp malloc ${CMAKE_BINARY_DIR}/libircclient/src/libircclient.so)
endif()

set_target_properties(game scripts PROPERTIES LINK_INTERFACE_MULTIPLICITY 5)
target_link_libraries(
worldserver
Detour
collision
format
g3dlib
game
irc
scripts
shared
trinityframework
${Boost_LIBRARIES}
${MYSQL_LIBRARY}
${OPENSSL_LIBRARIES}
${READLINE_LIBRARY}
${UNIX_LIBS}
${ZLIB_LIBRARIES}
)

if( UNIX )
    install(TARGETS worldserver DESTINATION bin)
    install(FILES worldserver.conf.dist DESTINATION etc)
	set_target_properties(worldserver PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
elseif( WIN32 )
  install(TARGETS worldserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES worldserver.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
