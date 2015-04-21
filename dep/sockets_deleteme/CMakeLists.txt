SET(trinitysockets_STAT_SRCS
   Base64.cpp
   Exception.cpp
   Ipv4Address.cpp
   Ipv6Address.cpp
   Lock.cpp
   Mutex.cpp
   Parse.cpp
   ResolvServer.cpp
   ResolvSocket.cpp
   Socket.cpp
   SocketHandler.cpp
   StdoutLog.cpp
   StreamSocket.cpp
   TcpSocket.cpp
   Thread.cpp
   UdpSocket.cpp
   Utility.cpp
   socket_include.cpp
)

INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/dep/include/sockets
${CMAKE_SOURCE_DIR}/src/shared/Logging
${CMAKE_SOURCE_DIR}/src/shared/Networking
${CMAKE_SOURCE_DIR}/src/shared
${CMAKE_SOURCE_DIR}/src/framework
)

add_library(trinitysockets STATIC ${trinitysockets_STAT_SRCS})
