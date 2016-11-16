IF(NOT PLAYERBOT)
	SET(EXCLUDE_PLAYERBOT ${CMAKE_CURRENT_SOURCE_DIR}/playerbot)
ENDIF(NOT PLAYERBOT)

IF(NOT TESTS)
	SET(EXCLUDE_TESTING ${CMAKE_CURRENT_SOURCE_DIR}/testing)
ENDIF(NOT TESTS)

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
  ${EXCLUDE_PLAYERBOT}
  ${EXCLUDE_TESTING})

  
set(PRIVATE_PCH_HEADER PrecompiledHeaders/gamePCH.h)
set(PRIVATE_PCH_SOURCE PrecompiledHeaders/gamePCH.cpp)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DTRINITY_API_EXPORT_GAME)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

  
add_library(game-interface INTERFACE)

target_include_directories(game-interface
  INTERFACE
    ${PUBLIC_INCLUDES})

	
target_link_libraries(game-interface
  INTERFACE
    shared
    Detour
	common
	)

add_library(game
  ${PRIVATE_PCH_SOURCE}
  ${PRIVATE_SOURCES})

target_include_directories(game
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})
	
target_link_libraries(game
  PUBLIC
    game-interface
  PRIVATE
    efsw
	irc
	segvcatch
	catch
)

set_target_properties(game
    PROPERTIES
      FOLDER
        "server")
		
add_dependencies(game revision_data.h)

if( BUILD_SHARED_LIBS )
  if( UNIX )
    install(TARGETS game
      LIBRARY
        DESTINATION lib)
  elseif( WIN32 )
    install(TARGETS game
      RUNTIME
        DESTINATION "${CMAKE_INSTALL_PREFIX}")
  endif()
endif()

# Generate precompiled header
add_cxx_pch(game ${PRIVATE_PCH_HEADER} ${PRIVATE_PCH_SOURCE})

if(PLAYERBOT)
	if ( UNIX )
		add_definitions(
			-D_PLAYERBOT_CONFIG='"${CONF_DIR}/aiplayerbot.conf"'
				)
	 endif()

	if( WIN32 )
	  if ( MSVC )
		add_custom_command(TARGET game
		  POST_BUILD
		  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/aiplayerbot.conf.dist.in ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
		) 
	  elseif ( MINGW )
		
		 add_custom_command(TARGET game
		  POST_BUILD
		  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/aiplayerbot.conf.dist.in ${CMAKE_BINARY_DIR}/bin/
		)
	  endif()
	endif()

	if( UNIX )
	  install(FILES aiplayerbot.conf.dist.in DESTINATION ${CONF_DIR})
	elseif( WIN32 )
	  install(FILES aiplayerbot.conf.dist.in DESTINATION "${CMAKE_INSTALL_PREFIX}")
	endif()
endif(PLAYERBOT)