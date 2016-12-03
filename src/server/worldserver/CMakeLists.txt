CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

if( WIN32 )
  list(APPEND PRIVATE_SOURCES ${sources_windows})
  if ( MSVC )
    list(APPEND PRIVATE_SOURCES worldserver.rc)
  endif()
endif()

#set(PRIVATE_PCH_HEADER PrecompiledHeaders/worldPCH.h)
#set(PRIVATE_PCH_SOURCE PrecompiledHeaders/worldPCH.cpp)


GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(worldserver
  ${PRIVATE_PCH_SOURCE}
  ${PRIVATE_SOURCES}
)

if( NOT WIN32 )
  set_target_properties(worldserver PROPERTIES
    COMPILE_DEFINITIONS _TRINITY_CORE_CONFIG="${CONF_DIR}/worldserver.conf"
  )
endif()

if( UNIX AND NOT NOJEM AND NOT APPLE )
  set(worldserver_LINK_FLAGS "-pthread -lncurses ${worldserver_LINK_FLAGS}")
endif()

set_target_properties(worldserver PROPERTIES LINK_FLAGS "${worldserver_LINK_FLAGS}")

if (UNIX)
    #this should be changed, see comments is CMakeLists from irc project
    set(UNIX_LIBS gomp ${CMAKE_BINARY_DIR}/libircclient/src/libircclient.so)
endif()

set_target_properties(game scripts PROPERTIES LINK_INTERFACE_MULTIPLICITY 5)

IF(USE_GPERFTOOLS)
	set(gperftools_libs
		profiler
		tcmalloc
	)
ENDIF()

target_link_libraries(worldserver
  PUBLIC
    scripts
    game
    gsoap
    readline
	${UNIX_LIBS}
  PRIVATE
	irc
	segvcatch
)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

target_include_directories(worldserver
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

link_directories(${LIBSDIR}) 

set_target_properties(worldserver
    PROPERTIES
      FOLDER
        "server")

# Add all dynamic projects as dependency to the worldserver
if (WORLDSERVER_DYNAMIC_SCRIPT_MODULES_DEPENDENCIES)
  add_dependencies(worldserver ${WORLDSERVER_DYNAMIC_SCRIPT_MODULES_DEPENDENCIES})
endif()

if( WIN32 )
  if ( MSVC )
    add_custom_command(TARGET worldserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/worldserver.conf.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    )
  elseif ( MINGW )
    add_custom_command(TARGET worldserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/worldserver.conf.dist ${CMAKE_BINARY_DIR}/bin/
    )
  endif()
endif()

if( UNIX )
  install(TARGETS worldserver DESTINATION bin)
  install(FILES worldserver.conf.dist DESTINATION ${CONF_DIR})
elseif( WIN32 )
  install(TARGETS worldserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES worldserver.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()

# Generate precompiled header
#add_cxx_pch(worldserver ${PRIVATE_PCH_HEADER} ${PRIVATE_PCH_SOURCE})
