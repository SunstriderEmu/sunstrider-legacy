include(ExternalProject)

include_directories(
  #for revision .h
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
  ${CMAKE_SOURCE_DIR}/src/collision
  ${CMAKE_SOURCE_DIR}/src/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/shared/Utilities
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_SOURCE_DIR}/src/framework/Platform
  ${CMAKE_SOURCE_DIR}/src/scripts/include
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/shared/Database
  ${CMAKE_SOURCE_DIR}/src/shared/DataStores
  ${CMAKE_SOURCE_DIR}/src/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/shared/Dynamic/
  ${CMAKE_SOURCE_DIR}/src/shared/Dynamic/LinkedReference
  ${CMAKE_SOURCE_DIR}/src/game
  ${CMAKE_SOURCE_DIR}/src/game/DataStores
  ${CMAKE_SOURCE_DIR}/src/game/Server
  ${CMAKE_SOURCE_DIR}/src/game/Server/Protocol
  ${CMAKE_SOURCE_DIR}/dep/include
  ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
  ${CMAKE_SOURCE_DIR}/dep/libircclient/include
  ${CMAKE_SOURCE_DIR}/src/game/Movement/Waypoints
  ${MYSQL_INCLUDE_DIR}
  ${ACE_INCLUDE_DIR}
)

#custom libircclient project, probably still a bit dirty (read: if you can make it cleaner do it)
IF ( UNIX )
ExternalProject_Add(libircclient
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/dep/libircclient
    INSTALL_DIR ${CMAKE_INSTALL_RPATH}
    CONFIGURE_COMMAND 
    #set prefix to any folder (we don't use any of these), and libdir to CMAKE_INSTALL_RPATH. This variable is defined in the root CMake file and is where the project lib are installed, and where the linker will look for it (defined in worldserver cmake).
        cp -R ${CMAKE_SOURCE_DIR}/dep/libircclient ${CMAKE_BINARY_DIR} && ${CMAKE_BINARY_DIR}/libircclient/configure --prefix=${CMAKE_BINARY_DIR} 
        --libdir=${CMAKE_INSTALL_RPATH} --enable-openssl --enable-shared
    BINARY_DIR ${CMAKE_BINARY_DIR}/libircclient
    BUILD_COMMAND ${MAKE}) 
    #todo custom clean
endif ( UNIX )


SET(irc_srcs
    IRCMgr.h
    IRCMgr.cpp
    ${CMAKE_SOURCE_DIR}/dep/libircclient/include/libirc_errors.h
    ${CMAKE_SOURCE_DIR}/dep/libircclient/include/libirc_events.h
    ${CMAKE_SOURCE_DIR}/dep/libircclient/include/libirc_options.h
    ${CMAKE_SOURCE_DIR}/dep/libircclient/include/libirc_rfcnumeric.h
    ${CMAKE_SOURCE_DIR}/dep/libircclient/include/libircclient.h
)

add_library(irc STATIC ${irc_srcs})
add_dependencies(irc libircclient)

target_link_libraries(
    irc
    ${MYSQL_LIBRARY}
    ${LIBIRCCLIENT_LIB_WIN32}
    ${Boost_LIBRARIES}
)
