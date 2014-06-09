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
   Containers.h
   Errors.h
   Log.cpp
   Log.h
   Mthread.cpp
   Mthread.h
   Profiler.cpp
   ProfilerMgr.cpp
   Timer.h
   Util.cpp
   Util.h
   WorldPacket.h
   SystemConfig.h
   ${sources_Threading}
)
IF (WIN32)
    set(shared_STAT_SRCS ${shared_STAT_SRCS} ServiceWin32.cpp)
ENDIF()

IF(UNIX)
add_definitions(-fopenmp)
ELSEIF(WIN32 AND MSVC)
add_definitions(/openmp)
ENDIF()
add_library(shared STATIC ${shared_STAT_SRCS})
target_link_libraries(
shared
${ACE_LIBRARY}
)
