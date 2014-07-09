
########### next target ###############

file(GLOB_RECURSE game_STAT_SRCS *.cpp *.h)

IF(UNIX)
add_definitions(-D_TRINITY_SCRIPT_CONFIG='"${CONF_DIR}/worldserver.conf"')
add_definitions(-fopenmp)
ELSEIF(WIN32 AND MSVC)
add_definitions(/openmp)
ENDIF()
add_library(game STATIC ${game_STAT_SRCS})
ADD_DEPENDENCIES(game revision.h)
target_link_libraries(
game
collision
scripts
Detour
Recast
)