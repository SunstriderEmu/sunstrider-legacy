
########### next target ###############

SET(worldserver_SRCS
CliRunnable.cpp 
CliRunnable.h 
Main.cpp 
Master.cpp 
Master.h 
RASocket.cpp 
RASocket.h 
WorldRunnable.cpp 
WorldRunnable.h
)

SET(worldserver_LINK_FLAGS "")

add_executable(worldserver ${worldserver_SRCS})
IF ( UNIX )
add_definitions(
-D_TRINITY_CORE_CONFIG='"${CONF_DIR}/worldserver.conf"'
)

IF (DO_MYSQL)
    SET(worldserver_LINK_FLAGS "-pthread ${worldserver_LINK_FLAGS}")
ENDIF(DO_MYSQL)
IF (DO_POSTGRESQL)
    SET(worldserver_LINK_FLAGS "-pthread ${worldserver_LINK_FLAGS}")
ENDIF(DO_POSTGRESQL)
ENDIF ( UNIX )

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    SET(worldserver_LINK_FLAGS "-framework Carbon ${worldserver_LINK_FLAGS}")
    SET(SCRIPT_LIB "")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")

SET_TARGET_PROPERTIES(worldserver PROPERTIES LINK_FLAGS "${worldserver_LINK_FLAGS}")

IF (UNIX)
    SET(UNIX_LIBS gomp malloc ${CMAKE_SOURCE_DIR}/dep/libircclient/src/libircclient.so)
ENDIF()

target_link_libraries(
worldserver
scripts
game
shared
zlib
trinityframework
trinitysockets
trinitydatabase
trinityauth
trinityconfig
wrchat
vmaps
ZThread
g3dlite
${READLINE_LIBRARY}
${UNIX_LIBS}
${MYSQL_LIBRARIES}
${POSTGRESQL_LIBRARIES}
${OPENSSL_LIBRARIES}
${ACE_LIBRARY}
${ZLIB}
${OSX_LIBS}
)

IF (CENTOS)
    target_link_libraries(
        worldserver
        termcap
    )
ENDIF (CENTOS)

if( UNIX )
    install(TARGETS worldserver DESTINATION bin)
    install(FILES worldserver.conf.dist DESTINATION etc)
        install(FILES ${CMAKE_SOURCE_DIR}/dep/libircclient/src/libircclient.so DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
        set_target_properties(worldserver PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
elseif( WIN32 )
  install(TARGETS worldserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES worldserver.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
