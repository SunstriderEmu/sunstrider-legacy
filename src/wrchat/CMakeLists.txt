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

SET(wrchat_srcs
    IRCMgr.cpp
)

add_library(wrchat STATIC ${wrchat_srcs})
target_link_libraries(
    wrchat
    ${MYSQL_LIBRARY}
)
