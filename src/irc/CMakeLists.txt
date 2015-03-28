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
 
IF ( UNIX )
ExternalProject_Add(libircclient
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/dep/libircclient
    CONFIGURE_COMMAND ${CMAKE_SOURCE_DIR}/dep/libircclient/configure --prefix=<INSTALL_DIR> --enable-openssl --enable-shared
    BINARY_DIR ${CMAKE_SOURCE_DIR}/dep/libircclient
    BUILD_COMMAND ${MAKE})
endif ( UNIX )

IF (WIN32)
	
ENDIF()
	
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
target_link_libraries(
    irc
    ${MYSQL_LIBRARY}
	${LIBIRCCLIENT_LIB_WIN32}
    ${Boost_LIBRARIES}
)
