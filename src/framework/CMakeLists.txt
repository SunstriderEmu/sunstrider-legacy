file(GLOB_RECURSE sources_Framework *.h *.cpp)

SET(trinityframework_STAT_SRCS
   ${sources_Framework}
)
include_directories(
${CMAKE_CURRENT_SRC_DIR}
)
add_library(trinityframework STATIC ${trinityframework_STAT_SRCS})
