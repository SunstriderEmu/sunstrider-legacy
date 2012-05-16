########### next target ###############

SET(trinityirc_STAT_SRCS
  ChannelMgr.cpp
  Config.cpp
  In_RFC.cpp
  IRC_ChatHandler.cpp
  IRC.cpp
  Message.h
  UserOperations.cpp
  In_CTCP.cpp
  In_STR.cpp
  IRC_ChatHandler.h
  IRC.h
  Output.cpp
  Socket.cpp
)

add_library(trinityirc STATIC ${trinityirc_STAT_SRCS})
target_link_libraries(
trinityirc
)
