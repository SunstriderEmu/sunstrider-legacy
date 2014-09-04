
########### next target ###############

file(GLOB_RECURSE game_STAT_SRCS *.cpp *.h)
file(GLOB_RECURSE sources_DataStores Accounts/*.cpp Accounts/*.h)
file(GLOB_RECURSE sources_DataStores DataStores/*.cpp DataStores/*.h)
file(GLOB_RECURSE sources_DataStores Movement/*.cpp Movement/*.h)
file(GLOB_RECURSE sources_DataStores Server/*.cpp Server/*.h)
file(GLOB_RECURSE sources_Warden Warden/*.cpp Warden/*.h)

set(game_STAT_SRCS
  ${game_STAT_SRCS}
  ${sources_DataStores}
  ${sources_Warden}
)
 
INCLUDE_DIRECTORIES(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
  ${CMAKE_SOURCE_DIR}/dep/include
  ${CMAKE_SOURCE_DIR}/dep/libircclient/include
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
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
  ${CMAKE_SOURCE_DIR}/src/scripts/include
  ${CMAKE_SOURCE_DIR}/src/shared/
  ${CMAKE_SOURCE_DIR}/src/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/shared/Cryptography
  ${CMAKE_SOURCE_DIR}/src/shared/Cryptography/Authentication
  ${CMAKE_SOURCE_DIR}/src/shared/Database
  ${CMAKE_SOURCE_DIR}/src/shared/DataStores
  ${CMAKE_SOURCE_DIR}/src/shared/Debugging
  ${CMAKE_SOURCE_DIR}/src/shared/Dynamic/LinkedReference
  ${CMAKE_SOURCE_DIR}/src/shared/Dynamic
  ${CMAKE_SOURCE_DIR}/src/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/shared/Networking
  ${CMAKE_SOURCE_DIR}/src/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/shared/Utilities
  ${CMAKE_SOURCE_DIR}/src/wrchat
  ${MYSQL_INCLUDE_DIR}
  ${ACE_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
)

IF(UNIX)
add_definitions(-D_TRINITY_SCRIPT_CONFIG='"${CONF_DIR}/worldserver.conf"')
add_definitions(-fopenmp)
ELSEIF(WIN32 AND MSVC)
add_definitions(/openmp)
ENDIF()
add_library(game STATIC ${game_STAT_SRCS})

add_dependencies(game revision.h)

target_link_libraries(
game
collision
scripts
Detour
Recast
${MYSQL_LIBRARY}
)