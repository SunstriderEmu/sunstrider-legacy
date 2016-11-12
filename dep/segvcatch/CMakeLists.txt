file(GLOB sources_segvcatch *.cpp *.h)

add_library(segvcatch STATIC ${sources_segvcatch})

target_include_directories(segvcatch
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})