
########### next target ###############

SET(trinity-core_SRCS
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

SET(trinity-core_LINK_FLAGS "")

add_executable(trinity-core ${trinity-core_SRCS})
IF ( UNIX )
add_definitions(
-D_TRINITY_CORE_CONFIG='"${CONF_DIR}/trinitycore.conf"'
)

IF (DO_MYSQL)
    SET(trinity-core_LINK_FLAGS "-pthread ${trinity-core_LINK_FLAGS}")
ENDIF(DO_MYSQL)
IF (DO_POSTGRESQL)
    SET(trinity-core_LINK_FLAGS "-pthread ${trinity-core_LINK_FLAGS}")
ENDIF(DO_POSTGRESQL)
ENDIF ( UNIX )

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    SET(trinity-core_LINK_FLAGS "-framework Carbon ${trinity-core_LINK_FLAGS}")
    SET(SCRIPT_LIB "")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")

SET_TARGET_PROPERTIES(trinity-core PROPERTIES LINK_FLAGS "${trinity-core_LINK_FLAGS}")

IF (UNIX)
	SET(UNIX_LIBS gomp malloc ${CMAKE_SOURCE_DIR}/dep/libircclient/src/libircclient.so)
ENDIF()

target_link_libraries(
trinity-core
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
        trinity-core
        termcap
    )
ENDIF (CENTOS)

if( UNIX )
	install(TARGETS trinity-core DESTINATION bin)
	install(FILES trinitycore.conf.dist DESTINATION etc)
        install(FILES ${CMAKE_SOURCE_DIR}/dep/libircclient/src/libircclient.so DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
        set_target_properties(trinity-core PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
elseif( WIN32 )
  install(TARGETS trinity-core DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES trinitycore.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
