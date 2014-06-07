########### next target ###############

SET(authserver_SRCS
AuthCodes.h 
AuthSocket.cpp 
AuthSocket.h 
Main.cpp 
RealmList.cpp 
RealmList.h
)

SET(authserver_LINK_FLAGS "")

add_executable(authserver ${authserver_SRCS})
IF (UNIX)
add_definitions(
-D_TRINITY_REALM_CONFIG='"${CONF_DIR}/authserver.conf"'
)

IF (DO_MYSQL)
   SET(authserver_LINK_FLAGS "-pthread ${authserver_LINK_FLAGS}")
ENDIF(DO_MYSQL)
IF (DO_POSTGRESQL)
   SET(authserver_LINK_FLAGS "-pthread ${authserver_LINK_FLAGS}")
ENDIF(DO_POSTGRESQL)

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   SET(authserver_LINK_FLAGS "-framework Carbon ${authserver_LINK_FLAGS}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")

ENDIF( UNIX )

SET_TARGET_PROPERTIES(authserver PROPERTIES LINK_FLAGS "${authserver_LINK_FLAGS}")

IF (UNIX)
    SET(UNIX_LIBS gomp)
ENDIF()
target_link_libraries(
authserver
shared
trinityframework
trinitysockets
trinitydatabase
trinityauth
trinityconfig
ZThread
zlib
${UNIX_LIBS}
${OPENSSL_LIBRARIES}
${MYSQL_LIBRARIES}
${POSTGRESQL_LIBRARIES}
${OSX_LIBS}
)

if( UNIX )
  install(TARGETS authserver DESTINATION bin)
  install(FILES  authserver.conf.dist DESTINATION etc)
elseif( WIN32 )
  install(TARGETS authserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES authserver.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()