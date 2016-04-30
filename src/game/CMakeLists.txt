file(GLOB_RECURSE game_STAT_SRCS *.cpp *.h)

set(game_STAT_PCH_HDR PrecompiledHeaders/gamePCH.h)
set(game_STAT_PCH_SRC PrecompiledHeaders/gamePCH.cpp)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/cppformat
  ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
  ${CMAKE_SOURCE_DIR}/dep/
  ${CMAKE_SOURCE_DIR}/dep/libircclient/include
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
  ${CMAKE_SOURCE_DIR}/dep/segvcatch
  ${CMAKE_SOURCE_DIR}/dep/zlib
  ${CMAKE_SOURCE_DIR}/src/collision
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_SOURCE_DIR}/src/framework/Platform
  ${CMAKE_SOURCE_DIR}/src/game
  ${CMAKE_SOURCE_DIR}/src/game/Addons
  ${CMAKE_SOURCE_DIR}/src/game/Accounts
  ${CMAKE_SOURCE_DIR}/src/game/DataStores
  ${CMAKE_SOURCE_DIR}/src/game/Movement
  ${CMAKE_SOURCE_DIR}/src/game/Movement/MovementGenerators
  ${CMAKE_SOURCE_DIR}/src/game/Movement/Spline
  ${CMAKE_SOURCE_DIR}/src/game/Movement/Waypoints
  ${CMAKE_SOURCE_DIR}/src/game/Server
  ${CMAKE_SOURCE_DIR}/src/game/Server/Protocol
  ${CMAKE_SOURCE_DIR}/src/game/Warden
  ${CMAKE_SOURCE_DIR}/src/playerbot
  ${CMAKE_SOURCE_DIR}/src/scripts/include
  ${CMAKE_SOURCE_DIR}/src/shared/
  ${CMAKE_SOURCE_DIR}/src/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/shared/Cryptography
  ${CMAKE_SOURCE_DIR}/src/shared/Cryptography/Authentication
  ${CMAKE_SOURCE_DIR}/src/shared/Database
  ${CMAKE_SOURCE_DIR}/src/shared/Database/Implementation
  ${CMAKE_SOURCE_DIR}/src/shared/DataStores
  ${CMAKE_SOURCE_DIR}/src/shared/Debugging
  ${CMAKE_SOURCE_DIR}/src/shared/Dynamic/LinkedReference
  ${CMAKE_SOURCE_DIR}/src/shared/Dynamic
  ${CMAKE_SOURCE_DIR}/src/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/shared/Networking
  ${CMAKE_SOURCE_DIR}/src/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/shared/Utilities
  ${CMAKE_SOURCE_DIR}/src/testing
  ${CMAKE_SOURCE_DIR}/src/irc
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
  ${MYSQL_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
)

add_library(game STATIC 
  ${game_STAT_SRCS}
  ${game_STAT_PCH_SRC}
)

add_dependencies(game revision.h)

target_link_libraries(
game
collision
scripts
Detour
Recast
segvcatch
${TESTING_LIB}
${MYSQL_LIBRARY}
)

# Generate precompiled header
add_cxx_pch(game ${game_STAT_PCH_HDR} ${game_STAT_PCH_SRC})