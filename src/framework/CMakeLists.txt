file(GLOB_RECURSE sources_Framework *.h *.cpp)

SET(trinityframework_STAT_SRCS
   ${sources_Framework}
)
include_directories(
${CMAKE_CURRENT_SRC_DIR}
${CMAKE_SOURCE_DIR}/src/framework
${CMAKE_SOURCE_DIR}/src/framework/Platform
${CMAKE_SOURCE_DIR}/src/shared
${CMAKE_SOURCE_DIR}/src/shared/Dynamic
${CMAKE_SOURCE_DIR}/src/shared/Dynamic/LinkedReference
)

add_library(trinityframework STATIC ${trinityframework_STAT_SRCS})
