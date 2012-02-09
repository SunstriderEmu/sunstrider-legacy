add_subdirectory(vmap)
add_subdirectory(Auth)
add_subdirectory(Config)
add_subdirectory(Database)

file(GLOB_RECURSE sources_Threading Threading/*.cpp Threading/*.h)

########### next target ###############

SET(shared_STAT_SRCS
   Base.cpp
   Base.h
   ByteBuffer.h
   Common.cpp
   Common.h
   Errors.h
   Log.cpp
   Log.h
   Mthread.cpp
   Mthread.h
   Timer.h
   Util.cpp
   Util.h
   WorldPacket.h
   SystemConfig.h
   ${sources_Threading}
)
add_definitions(-fopenmp)
add_library(shared STATIC ${shared_STAT_SRCS})
target_link_libraries(
shared
${ACE_LIBRARY}
)
