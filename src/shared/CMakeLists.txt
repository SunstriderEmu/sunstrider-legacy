file(GLOB_RECURSE sources_Configuration Configuration/*.cpp Configuration/*.h)
file(GLOB_RECURSE sources_Cryptography Cryptography/*.cpp Cryptography/*.h)
file(GLOB_RECURSE sources_Database Database/*.cpp Database/*.h)
file(GLOB_RECURSE sources_DataStores DataStores/*.cpp DataStores/*.h)
file(GLOB_RECURSE sources_Dynamic Dynamic/*.cpp Dynamic/*.h)
file(GLOB_RECURSE sources_Logging Logging/*.cpp Logging/*.h)
file(GLOB_RECURSE sources_Networking Networking/*.cpp Networking/*.h)
file(GLOB_RECURSE sources_Packets Packets/*.cpp Packets/*.h)
file(GLOB_RECURSE sources_Utilities Utilities/*.cpp Utilities/*.h)
file(GLOB_RECURSE sources_Threading Threading/*.cpp Threading/*.h)

file(GLOB sources_localdir *.cpp *.h)

set(sources_Debugging Debugging/Errors.cpp Debugging/Errors.h)

########### next target ###############

#some of these don't exist yet
set(shared_STAT_SRCS
  ${shared_STAT_SRCS}
  ${sources_Configuration}
  ${sources_Cryptography}
  ${sources_Database}
  ${sources_DataStores}
  ${sources_Debugging}
  ${sources_Dynamic}
  ${sources_Logging}
  ${sources_Networking}
  ${sources_Packets}
  ${sources_Threading}
  ${sources_Utilities}
  ${sources_localdir}
)

IF(UNIX)
add_definitions(-fopenmp)
ELSEIF(WIN32 AND MSVC)
add_definitions(/openmp)
ENDIF()
add_library(shared STATIC 
${shared_STAT_SRCS}
)

add_dependencies(shared revision.h)

target_link_libraries(
shared
)

INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
${CMAKE_SOURCE_DIR}/dep/utf8cpp
${CMAKE_SOURCE_DIR}/dep/SFMT
${CMAKE_SOURCE_DIR}/src/framework
${CMAKE_SOURCE_DIR}/src/framework/Platform
${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
${CMAKE_SOURCE_DIR}/dep/include
${CMAKE_SOURCE_DIR}/dep/zlib
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/Configuration
${CMAKE_CURRENT_SOURCE_DIR}/Cryptography
${CMAKE_CURRENT_SOURCE_DIR}/Database
${CMAKE_CURRENT_SOURCE_DIR}/DataStores
${CMAKE_CURRENT_SOURCE_DIR}/Debugging
${CMAKE_CURRENT_SOURCE_DIR}/Dynamic
${CMAKE_CURRENT_SOURCE_DIR}/Logging
${CMAKE_CURRENT_SOURCE_DIR}/Networking
${CMAKE_CURRENT_SOURCE_DIR}/Packets
${CMAKE_CURRENT_SOURCE_DIR}/Threading
${CMAKE_CURRENT_SOURCE_DIR}/Utilities
${OPENSSL_INCLUDE_DIR}
${MYSQL_INCLUDE_DIR}
#for revision.h
${CMAKE_BINARY_DIR}
)